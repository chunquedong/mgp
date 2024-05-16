#ifndef VertexAttributeObject_H_
#define VertexAttributeObject_H_

#include "base/Ref.h"
#include "scene/VertexFormat.h"

namespace mgp
{

class Mesh;
class ShaderProgram;
class VertexAttributeObject;

typedef uint64_t BufferHandle;

/**
 * Defines a binding between the vertex layout of a Mesh and the vertex
 * input attributes of a vertex shader (Effect).
 *
 * In a perfect world, this class would always be a binding directly between
 * a unique VertexFormat and an Effect, where the VertexFormat is simply the
 * definition of the layout of any anonymous vertex buffer. However, the OpenGL
 * mechanism for setting up these bindings is Vertex Array Objects (VAOs).
 * OpenGL requires a separate VAO per vertex buffer object (VBO), rather than per
 * vertex layout definition. Therefore, although we would like to define this
 * binding between a VertexFormat and Effect, we are specifying the binding
 * between a Mesh and Effect to satisfy the OpenGL requirement of one VAO per VBO.
 *
 * Note that this class still does provide a binding between a VertexFormat
 * and an Effect, however this binding is actually a client-side binding and 
 * should only be used when writing custom code that use client-side vertex
 * arrays, since it is slower than the server-side VAOs used by OpenGL
 * (when creating a VertexAttributeObject between a Mesh and Effect).
 */
class VertexAttributeBinding : public Refable {
    friend class VertexAttributeObject;
    friend class Mesh;
public:
    /**
     * Creates a new VertexAttributeObject between the given Mesh and Effect.
     *
     * If a VertexAttributeObject matching the specified Mesh and Effect already
     * exists, it will be returned. Otherwise, a new VertexAttributeObject will
     * be returned. If OpenGL VAOs are enabled, the a new VAO will be created and
     * stored in the returned VertexAttributeObject, otherwise a client-side
     * array of vertex attribute bindings will be stored.
     *
     * @param mesh The mesh.
     * @param vertexPointer Pointer to beginning of client-side vertex array.
     * @param effect The effect.
     *
     * @return A VertexAttributeObject for the requested parameters.
     * @script{create}
     */
    static UPtr<VertexAttributeBinding> create(BufferHandle mesh, const VertexFormat& vertexFormat, 
        void* vertexPointer, BufferHandle indexBufferObject);

    VertexAttributeObject *getVao(ShaderProgram* effect);

    void setVertexPointer(void* vertexPointer);

    void update();
private:
    VertexAttributeBinding();
    ~VertexAttributeBinding();

    friend class VertexAttributeObject;
    friend class GLRenderer;

    BufferHandle _vertexBufferObject;
    BufferHandle _instanceBufferObject;
    BufferHandle _indexBufferObject;
    VertexFormat vertexFormat;
    void* vertexPointer;

    std::vector<VertexAttributeObject*> _vaoList;
};

class VertexAttributeObject : public Refable
{
public:

    /**
     * Binds this vertex array object.
     */
    void bind();

    /**
     * Unbinds this vertex array object.
     */
    void unbind();

public:

    class VertexAttribute
    {
    public:
        bool enabled;
        int size;
        unsigned int type;
        bool normalized;
        unsigned int stride;
        void* pointer;
        unsigned int location;
    };

    /**
     * Constructor.
     */
    VertexAttributeObject(VertexAttributeBinding* parent, ShaderProgram* effect);

    /**
     * Destructor.
     */
    ~VertexAttributeObject();

    BufferHandle getVbo() { return _vertexAttributeBinding->_vertexBufferObject; }

    BufferHandle getInstancedVbo() { return _vertexAttributeBinding->_instanceBufferObject; }

    BufferHandle getEbo() { return _vertexAttributeBinding->_indexBufferObject; }

private:
    void init();
    
    friend class GLRenderer;
    friend class VertexAttributeBinding;

    uint64_t _handle;
    std::vector<VertexAttribute> _attributes;
    ShaderProgram* _effect;
    VertexAttributeBinding* _vertexAttributeBinding;
    bool _isDirty;
};

}

#endif
