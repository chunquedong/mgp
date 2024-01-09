#ifndef EFFECT_H_
#define EFFECT_H_

#include "base/Ref.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "math/Matrix.h"
#include "Texture.h"

#include <unordered_map>

namespace mgp
{
/** Vertex attribute. */
typedef unsigned int VertexAttributeLoc;
typedef uint64_t ProgramHandle;
class Uniform;

/**
 * Defines an effect which can be applied during rendering.
 *
 * An effect essentially wraps an OpenGL program object, which includes the
 * vertex and fragment shader.
 *
 * In the future, this class may be extended to support additional logic that
 * typical effect systems support, such as GPU render state management,
 * techniques and passes.
 */
class ShaderProgram: public Refable
{
public:

    /**
     * Creates an effect using the specified vertex and fragment shader.
     *
     * @param vshPath The path to the vertex shader file.
     * @param fshPath The path to the fragment shader file.
     * @param defines A new-line delimited list of preprocessor defines. May be NULL.
     * 
     * @return The created effect.
     */
    static ShaderProgram* createFromFile(const char* vshPath, const char* fshPath, const char* defines = NULL);

    /**
     * Creates an effect from the given vertex and fragment shader source code.
     *
     * @param vshSource The vertex shader source code.
     * @param fshSource The fragment shader source code.
     * @param defines A new-line delimited list of preprocessor defines. May be NULL.
     * 
     * @return The created effect.
     */
    static ShaderProgram* createFromSource(const char* vshSource, const char* fshSource, const char* defines = NULL);

    /**
     * Returns the unique string identifier for the effect, which is a concatenation of
     * the shader paths it was loaded from.
     */
    const char* getId() const;

    /**
     * Returns the vertex attribute handle for the vertex attribute with the specified name.
     *
     * @param name The name of the vertex attribute to return.
     * 
     * @return The vertex attribute, or -1 if no such vertex attribute exists.
     */
    VertexAttributeLoc getVertexAttribute(const char* name) const;

    /**
     * Returns the uniform handle for the uniform with the specified name.
     *
     * @param name The name of the uniform to return.
     * 
     * @return The uniform, or NULL if no such uniform exists.
     */
    Uniform* getUniform(const std::string& name) const;

    /**
     * Returns the specified active uniform.
     * 
     * @param index The index of the uniform to return.
     * 
     * @return The uniform, or NULL if index is invalid.
     */
    Uniform* getUniform(unsigned int index) const;

    /**
     * Returns the number of active uniforms in this effect.
     * 
     * @return The number of active uniforms.
     */
    unsigned int getUniformCount() const;

    /**
     * Binds this effect to make it the currently active effect for the rendering system.
     */
    void bind();

    /**
     * Returns the currently bound effect for the rendering system.
     *
     * @return The currently bound effect, or NULL if no effect is currently bound.
     */
    //static ShaderProgram* getCurrentEffect();

public:

    /**
     * Hidden constructor (use createEffect instead).
     */
    ShaderProgram();
private:
    /**
     * Hidden destructor (use destroyEffect instead).
     */
    ~ShaderProgram();

    /**
     * Hidden copy assignment operator.
     */
    ShaderProgram& operator=(const ShaderProgram&);

    static ShaderProgram* createFromSource(const char* id, const char* vshPath, const char* vshSource, const char* fshPath, const char* fshSource, const char* defines = NULL);

private:
    friend class VertexAttributeBinding;
    friend class GLRenderer;
    ProgramHandle _program;
    std::string _id;
    std::map<std::string, VertexAttributeLoc> _vertexAttributes;
    mutable std::unordered_map<std::string, Uniform*> _uniforms;
    static Uniform _emptyUniform;
};

/**
 * Represents a uniform variable within an effect.
 */
class Uniform
{
    friend class ShaderProgram;

public:

    /**
     * Returns the name of this uniform.
     * 
     * @return The name of the uniform.
     */
    const char* getName() const;

    /**
     * Returns the OpenGL uniform type.
     * 
     * @return The OpenGL uniform type.
     */
    const unsigned int getType() const;

    bool isSampler2d() const;

    /**
     * Returns the effect for this uniform.
     *
     * @return The uniform's effect.
     */
    ShaderProgram* getEffect() const;

public:

    /**
     * Constructor.
     */
    Uniform();

    /**
     * Copy constructor.
     */
    Uniform(const Uniform& copy);

    /**
     * Destructor.
     */
    ~Uniform();

    /**
     * Hidden copy assignment operator.
     */
    Uniform& operator=(const Uniform&);

private:
    friend class GLRenderer;
    friend class MaterialParameter;

    std::string _name;
    int _location;
    unsigned int _type;

    //texture unit offset
    unsigned int _index;
    
    //array size; 1 if not array
    int _size;
    ShaderProgram* _effect;
};

}

#endif
