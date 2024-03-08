#ifndef MATERIAL_H_
#define MATERIAL_H_

#include "../scene/Drawable.h"
#include "base/Properties.h"
#include "base/Serializable.h"
#include "StateBlock.h"

namespace mgp
{

class NodeCloneContext;
class RenderInfo;
class MaterialParameter;
class ShaderProgram;

/**
 * Defines a material for an object to be rendered.
 *
 * This class encapsulates a set of rendering techniques that can be used to render an
 * object. This class facilitates loading of techniques using specified shaders or
 * material files (.material). When multiple techniques are loaded using a material file,
 * the current technique for an object can be set at runtime.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Materials
 */
class Material : public Refable, public Serializable
{
    friend class MaterialParamBinding;
    friend class Node;
    friend class Model;

public:

    /**
     * Pass creation callback function definition.
     */
    typedef std::string(*PassCallback)(Material*, void*);

    /**
     * Creates a material using the data from the Properties object defined at the specified URL, 
     * where the URL is of the format "<file-path>.<extension>#<namespace-id>/<namespace-id>/.../<namespace-id>"
     * (and "#<namespace-id>/<namespace-id>/.../<namespace-id>" is optional). 
     * 
     * @param url The URL pointing to the Properties object defining the material.
     * 
     * @return A new Material or NULL if there was an error.
     * @script{create}
     */
    static UPtr<Material> create(const char* url);
    static std::vector<Material*> createAll(const char* url);

    /**
     * Creates a material from a Properties file.
     *
     * This overloaded method allows you to pass a function pointer to be called back for each
     * pass that is loaded for the material. The passed in callback receives a pointer to each
     * Pass being created and returns a string of optional defines to append to the shaders
     * being compiled for each Pass. The function is called during Pass creation, prior to
     * compiling the shaders for that Pass. MaterialParameters can be safely modified in this
     * callback even though the shader is not yet compiled.
     *
     * @param url The URL pointing to the Properties object defining the material.
     * @param callback Function pointer to be called during Pass creation.
     * @param cookie Optional custom parameter to be passed to the callback function.
     *
     * @return A new Material or NULL if there was an error.
     * @script{ignore}
     */
    static UPtr<Material> create(const char* url, PassCallback callback, void* cookie = NULL);

    /**
     * Creates a material from the specified properties object.
     * 
     * @param materialProperties The properties object defining the 
     *      material (must have namespace equal to 'material').
     * @return A new Material.
     * @script{create}
     */
    static UPtr<Material> create(Properties* materialProperties);

    /**
     * Creates a material from the specified effect.
     *
     * The returned material has a single technique and a single pass for the
     * given effect.
     *
     * @param effect ShaderProgram for the new material.
     * 
     * @return A new Material.
     * @script{create}
     */
    static UPtr<Material> create(ShaderProgram* effect);

    /**
     * Creates a material using the specified vertex and fragment shader.
     *
     * The returned material has a single technique and a single pass for the
     * given effect.
     *
     * @param vshPath Path to the vertex shader file.
     * @param fshPath Path to the fragment shader file.
     * @param defines New-line delimited list of preprocessor defines.
     * 
     * @return A new Material.
     * @script{create}
     */
    static UPtr<Material> create(const char* vshPath, const char* fshPath, const char* defines = NULL);

    /**
     * @see MaterialParamBinding::setNodeBinding
     */
    //void setNodeBinding(Node* node);

    void setName(const std::string &name) {
        this->name = name;
    }
    const std::string &getName() {
        return name;
    }

    /**
     * Returns the effect for this Pass.
     */
    ShaderProgram* getEffect() const;

    /**
     * Sets a vertex attribute binding for this pass.
     *
     * When a mesh binding is set, the VertexAttributeBinding will be automatically
     * bound when the bind() method is called for the pass.
     *
     * @param binding The VertexAttributeBinding to set (or NULL to remove an existing binding).
     */
    //void setVertexAttributeBinding(VertexAttributeBinding* binding);

    /**
     * Sets a vertex attribute binding for this pass.
     *
     * @return The vertex attribute binding for this pass.
     */
    //VertexAttributeBinding* getVertexAttributeBinding() const;

    /**
     * Binds the render state for this pass.
     *
     * This method should be called before executing any drawing code that should
     * use this pass. When drawing code is complete, the unbind() method should be called.
     */
    void bind();

    /**
    * Set the buildin parameter
    */
    void setParams(std::vector<Light*>* lights,
        Camera* camera,
        Rectangle* viewport, Drawable* drawable, int instanced);

    /**
     * Unbinds the render state for this pass.
     *
     * This method should always be called when rendering for a pass is complete, to
     * restore the render state to the state it was in previous to calling bind().
     */
    void unbind();

    Material* getNextPass();

    void setNextPass(UPtr<Material> next);

    /**
     * @see Activator::createObject
     */
    static Serializable* createObject();

    /**
     * @see Serializable::getClassName
     */
    std::string getClassName();

    /**
     * @see Serializable::onSerialize
     */
    void onSerialize(Serializer* serializer);

    /**
     * @see Serializable::onDeserialize
     */
    void onDeserialize(Serializer* serializer);


    /**
     * Sets the fixed-function render state of this object to the state contained
     * in the specified StateBlock.
     *
     * The passed in StateBlock is stored in this MaterialParamBinding object with an
     * increased reference count and released when either a different StateBlock
     * is assigned, or when this MaterialParamBinding object is destroyed.
     *
     * @param state The state block to set.
     */
    void setStateBlock(StateBlock* state);

    /**
     * Gets the fixed-function StateBlock for this MaterialParamBinding object.
     *
     * The returned StateBlock is referenced by this MaterialParamBinding and therefore
     * should not be released by the user. To release a StateBlock for a
     * MaterialParamBinding, the setState(StateBlock*) method should be called, passing
     * NULL. This removes the StateBlock and resets the fixed-function render
     * state to the default state.
     *
     * It is legal to pass the returned StateBlock to another MaterialParamBinding object.
     * In this case, the StateBlock will be referenced by both MaterialParamBinding objects
     * and any changes to the StateBlock will be reflected in all objects
     * that reference it.
     *
     * @return The StateBlock for this MaterialParamBinding.
     */
    StateBlock* getStateBlock() const;

    /**
     * Gets a MaterialParameter for the specified name.
     *
     * The returned MaterialParameter can be used to set values for the specified
     * parameter name.
     *
     * Note that this method causes a new MaterialParameter to be created if one
     * does not already exist for the given parameter name.
     *
     * @param name Material parameter (uniform) name.
     *
     * @return A MaterialParameter for the specified name.
     */
    MaterialParameter* getParameter(const char* name, bool add = true) const;

    /**
     * Gets the number of material parameters.
     *
     * @return The number of material parameters.
     */
    unsigned int getParameterCount() const;

    /**
     * Gets a MaterialParameter for the specified index.
     *
     * @return A MaterialParameter for the specified index.
     */
    MaterialParameter* getParameterByIndex(unsigned int index);

    /**
     * Adds a MaterialParameter to the render state.
     *
     * @param param The parameters to to added.
     */
    void addParameter(MaterialParameter* param);

    /**
     * Removes(clears) the MaterialParameter with the given name.
     *
     * If a material parameter exists for the given name, it is destroyed and
     * removed from this MaterialParamBinding.
     *
     * @param name Material parameter (uniform) name.
     */
    void removeParameter(const char* name);

    /**
     * Sets a material parameter auto-binding.
     *
     * @param name The name of the material parameter to store an auto-binding for.
     * @param autoBinding A valid AutoBinding value.
     */
    //void setParameterAutoBinding(const char* name, AutoBinding autoBinding);

    /**
     * Sets a material parameter auto-binding.
     *
     * This method parses the passed in autoBinding string and attempts to convert it
     * to an AutoBinding enumeration value, which is then stored in this render state.
     *
     * @param name The name of the material parameter to store an auto-binding for.
     * @param autoBinding A string matching one of the built-in AutoBinding enum constants.
     */
    //void setParameterAutoBinding(const char* name, const char* autoBinding);

    /**
     * Clones this material.
     *
     * @param context The clone context.
     *
     * @return The newly created material.
     * @script{create}
     */
    UPtr<Material> clone(NodeCloneContext& context) const;


    /**
     * Creates a new material with optional pass callback function.
     */
    static UPtr<Material> create(Properties* materialProperties, PassCallback callback, void* cookie);


    const std::string& getShaderDefines();
    void setShaderDefines(const std::string &defiens);

    void getShaderId(std::string& id);
private:

    /**
     * Constructor.
     */
    Material();

    /**
     * Constructor.
     */
    //Material(const Material& m);
    
    /**
     * Destructor.
     */
    ~Material();

    bool initialize(Drawable* drawable, std::vector<Light*>* lights, int lightMask, int instanced);
    void bindCamera(Camera* camera, Rectangle& viewport, Node* node, Drawable* drawable);
    void bindLights(Camera* camera, std::vector<Light*>* lights, int lightMask);

    std::string name;
    ShaderProgram* _shaderProgram;
    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    std::string shaderDefines;
    std::string _dynamicDefines;
    //VertexAttributeBinding* _vertexAttributeBinding;
    UniquePtr<Material, true> _nextPass;

    //MaterialParamBinding _paramBinding;
    //Node* _node;

    /**
     * The StateBlock of fixed-function render states that can be applied to the MaterialParamBinding.
     */
    StateBlock _state;

    /**
     * Collection of MaterialParameter's to be applied to the mgp::ShaderProgram.
     */
    mutable std::vector<MaterialParameter*> _parameters;
};

}

#endif
