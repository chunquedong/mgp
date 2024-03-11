#ifndef RENDERDRIVER_H_
#define RENDERDRIVER_H_

#include "base/Base.h"
#include "scene/Mesh.h"
#include "scene/MeshBatch.h"
#include "material/StateBlock.h"
#include "material/Texture.h"
#include "material/ShaderProgram.h"
#include "material/MaterialParameter.h"
#include "material/VertexAttributeBinding.h"

namespace mgp
{
class FrameBuffer;
class RenderInfo;
class Drawable;

/** Vertex buffer handle. */
typedef uint64_t VertexBufferHandle;
/** Index buffer handle. */
typedef uint64_t IndexBufferHandle;

class DrawCall {
public:
    VertexBufferHandle _vertexBuffer = 0;
    unsigned int _vertexCount = 0;
    Mesh::PrimitiveType _primitiveType = Mesh::TRIANGLES;

    IndexBufferHandle _indexBuffer = 0;
    unsigned int _indexCount = 0;
    unsigned int _indexBufferOffset = 0;
    Mesh::IndexFormat _indexFormat = Mesh::INDEX16;
    void *_indices = 0;
    
    VertexAttributeBinding *_vertexAttributeArray = 0;
    Material* _material = 0;
    Drawable* _drawable = 0;
    void* _mesh = 0;
    bool _wireframe = false;

    uint64_t _instanceVbo = 0;
    int _instanceCount = 0;

    Drawable::RenderLayer _renderLayer = Drawable::Qpaque;
    double _distanceToCamera = 0;
};

class Renderer {
public:
    static Renderer* cur();
    void finalize();

public:
    virtual ~Renderer() {}
    /**
     * Flags used when clearing the active frame buffer targets.
     */
    enum ClearFlags
    {
        CLEAR_COLOR = 0x00004000,
        CLEAR_DEPTH = 0x00000100,
        CLEAR_STENCIL = 0x00000400,
        CLEAR_COLOR_DEPTH = CLEAR_COLOR | CLEAR_DEPTH,
        CLEAR_COLOR_STENCIL = CLEAR_COLOR | CLEAR_STENCIL,
        CLEAR_DEPTH_STENCIL = CLEAR_DEPTH | CLEAR_STENCIL,
        CLEAR_COLOR_DEPTH_STENCIL = CLEAR_COLOR | CLEAR_DEPTH | CLEAR_STENCIL
    };

    virtual void init() = 0;

    virtual void clear(ClearFlags flags, const Vector4 &color = Vector4::zero(), float clearDepth = 1.0, int clearStencil = 0.0) = 0;
    virtual void setViewport(int x, int y, int w, int h) = 0;

    virtual void updateState(StateBlock *state, int force) = 0;

public:
    /**
    * @type 0:vertex buffer, 1:index buffer
    */
    virtual uint64_t createBuffer(int type) = 0;

    /**
    * @type 0:vertex buffer, 1:index buffer
    * @usage 0: static, 1: dynamic;
    */
    virtual void setBufferData(uint64_t buffer, int type, size_t startOffset, const char* data, size_t len, int usage) = 0;
    virtual void deleteBuffer(uint64_t buffer) = 0;

    virtual void draw(DrawCall* drawCall) = 0;
public:
    virtual void updateTexture(Texture* texture) = 0;
    virtual void deleteTexture(Texture* texture) = 0;
    virtual void bindTextureSampler(Texture* texture) = 0;


    virtual UPtr<FrameBuffer> createFrameBuffer(const char* id, unsigned int width, unsigned int height, Image::Format format = Image::RGBA) = 0;
    virtual FrameBuffer* getCurrentFrameBuffer() = 0;


	virtual void bindVertexAttributeObj(VertexAttributeObject* vertextAttribute) = 0;
	virtual void unbindVertexAttributeObj(VertexAttributeObject* vertextAttribute) = 0;
	virtual void deleteVertexAttributeObj(VertexAttributeObject* vertextAttribute) = 0;

public:
    struct ProgramSrc {
        const char* id;
        const char* defines;
        const char* vshSource;
        const char* fshSource;
        const char* version = NULL;
    };

    virtual ShaderProgram* createProgram(ProgramSrc* src) = 0;
    virtual void deleteProgram(ShaderProgram* effect) = 0;
    virtual void bindProgram(ShaderProgram* effect) = 0;
    virtual bool bindUniform(MaterialParameter *value, Uniform *uniform, ShaderProgram* effect) = 0;

    virtual int drawCallCount() = 0;

};

}
#endif
