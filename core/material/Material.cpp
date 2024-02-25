#include "base/Base.h"
#include "Material.h"
#include "base/FileSystem.h"
#include "ShaderProgram.h"
#include "base/Properties.h"
#include "scene/Node.h"
#include "MaterialParameter.h"
#include "platform/Toolkit.h"
#include "scene/Scene.h"

namespace mgp
{
extern void loadRenderState(Material* renderState, Properties* properties);

Material::Material() :
    _shaderProgram(NULL)
{
}

Material::~Material()
{
    // Destroy all the material parameters
    for (size_t i = 0, count = _parameters.size(); i < count; ++i)
    {
        SAFE_RELEASE(_parameters[i]);
    }

    SAFE_RELEASE(_shaderProgram);
    //SAFE_RELEASE(_vertexAttributeBinding);
}

UPtr<Material> Material::create(const char* url)
{
    return create(url, (PassCallback)NULL, NULL);
}

std::vector<Material*> Material::createAll(const char* url)
{
    std::vector<Material*> res;

    UPtr<Properties> properties = Properties::create(url);
    if (properties.get() == NULL)
    {
        GP_WARN("Failed to create material from file: %s", url);
        return res;
    }

    for (Properties *p = properties->getNextNamespace(); p != NULL; p = properties->getNextNamespace()) {
        Material* material = create(p, NULL, NULL).take();
        res.push_back(material);
    }
    //SAFE_DELETE(properties);
    return res;
}

UPtr<Material> Material::create(const char* url, PassCallback callback, void* cookie)
{
    // Load the material properties from file.
    UPtr<Properties> properties = Properties::create(url);
    if (properties.get() == NULL)
    {
        GP_WARN("Failed to create material from file: %s", url);
        return UPtr<Material>(NULL);
    }

    UPtr<Material> material = create((strlen(properties->getNamespace()) > 0) ? properties.get() : properties->getNextNamespace(), callback, cookie);
    //SAFE_DELETE(properties);

    return material;
}

UPtr<Material> Material::create(Properties* materialProperties)
{
    return create(materialProperties, (PassCallback)NULL, NULL);
}

bool Material::initialize(Drawable* drawable, std::vector<Light*>* lights, int lightMask)
{
    std::string dynamicDefines;
    if (lights && vertexShaderPath.size() > 0) {
        int DIRECTIONAL_LIGHT_COUNT = 0;
        int POINT_LIGHT_COUNT = 0;
        int SPOT_LIGHT_COUNT = 0;
        for (int i = 0; i < lights->size(); ++i) {
            Light* light = (*lights)[i];
            if (light->getLightMask() & lightMask) {
                switch (light->getLightType()) {
                case Light::DIRECTIONAL:
                    ++DIRECTIONAL_LIGHT_COUNT;
                    break;
                case Light::POINT:
                    ++POINT_LIGHT_COUNT;
                    break;
                case Light::SPOT:
                    ++SPOT_LIGHT_COUNT;
                    break;
                }
            }
        }
        char buf[256];
        if (DIRECTIONAL_LIGHT_COUNT || POINT_LIGHT_COUNT || SPOT_LIGHT_COUNT) {
            snprintf(buf, 256, "DIRECTIONAL_LIGHT_COUNT %d;POINT_LIGHT_COUNT %d;SPOT_LIGHT_COUNT %d", DIRECTIONAL_LIGHT_COUNT, POINT_LIGHT_COUNT, SPOT_LIGHT_COUNT);
            dynamicDefines = buf;
        }
    }

    Model* model = dynamic_cast<Model*>(drawable);
    if (model)
    {
        MeshSkin* skin = model->getSkin();
        if (skin) {
            char buf[256];
            if (skin->getJointCount()) {
                snprintf(buf, 256, "SKINNING; SKINNING_JOINT_COUNT %d", skin->getJointCount());
                if (dynamicDefines.size() > 0) {
                    dynamicDefines += ";";
                }
                dynamicDefines += buf;
            }
        }
    }

    if (drawable && drawable->getNode() && drawable->getNode()->getWeights().size() > 0) {
        if (dynamicDefines.size() > 0) {
            dynamicDefines += ";";
        }
        dynamicDefines += "MORPH_TARGET_COUNT " + std::to_string(drawable->getNode()->getWeights().size());
    }

    if (_dynamicDefines != dynamicDefines) {
        _dynamicDefines = dynamicDefines;
        if (_shaderProgram) {
            SAFE_RELEASE(_shaderProgram);
        }
    }

    if (_shaderProgram) return true;

    std::string defines = shaderDefines;
    if (_dynamicDefines.size() > 0) {
        if (defines.size() > 0) {
            defines += ";";
        }
        defines += _dynamicDefines;
    }

    // Attempt to create/load the effect.
    _shaderProgram = ShaderProgram::createFromFile(vertexShaderPath.c_str(), fragmentShaderPath.c_str(), defines.c_str());
    if (_shaderProgram == NULL)
    {
        GP_WARN("Failed to create effect for pass. vertexShader = %s, fragmentShader = %s, defines = %s", 
            vertexShaderPath.c_str(), fragmentShaderPath.c_str(), defines.c_str());
        return false;
    }

    return true;
}

void Material::bindLights(Camera* camera, std::vector<Light*>* lights, int lightMask) {
    if (lights == NULL) return;

    int DIRECTIONAL_LIGHT_COUNT = 0;
    int POINT_LIGHT_COUNT = 0;
    int SPOT_LIGHT_COUNT = 0;

    char buf[256];
    for (int i = 0; i < lights->size(); ++i) {
        Light* light = (*lights)[i];
        if ((light->getLightMask() & lightMask) == 0) {
            continue;
        }
        switch (light->getLightType()) {
        case Light::DIRECTIONAL: {
            snprintf(buf, 256, "u_directionalLightColor[%d]", DIRECTIONAL_LIGHT_COUNT);
            getParameter(buf)->setVector3(light->getColor());

            snprintf(buf, 256, "u_directionalLightDirection[%d]", DIRECTIONAL_LIGHT_COUNT);
            Vector3 v = light->getNode()->getForwardVector();
            camera->getViewMatrix().transformVector(&v);
            getParameter(buf)->setVector3(v);

            ++DIRECTIONAL_LIGHT_COUNT;
        }
            break;
        case Light::POINT: {
            snprintf(buf, 256, "u_pointLightColor[%d]", POINT_LIGHT_COUNT);
            getParameter(buf)->setVector3(light->getColor());

            snprintf(buf, 256, "u_pointLightPosition[%d]", POINT_LIGHT_COUNT);
            Vector3 p = light->getNode()->getTranslation();
            camera->getViewMatrix().transformPoint(&p);
            getParameter(buf)->setVector3(p);

            snprintf(buf, 256, "u_pointLightRangeInverse[%d]", POINT_LIGHT_COUNT);
            getParameter(buf)->setFloat(light->getRangeInverse());

            ++POINT_LIGHT_COUNT;
        }
            break;
        case Light::SPOT: {
            snprintf(buf, 256, "u_spotLightColor[%d]", SPOT_LIGHT_COUNT);
            getParameter(buf)->setVector3(light->getColor());

            snprintf(buf, 256, "u_spotLightInnerAngleCos[%d]", SPOT_LIGHT_COUNT);
            getParameter(buf)->setFloat(light->getInnerAngleCos());

            snprintf(buf, 256, "u_spotLightOuterAngleCos[%d]", SPOT_LIGHT_COUNT);
            getParameter(buf)->setFloat(light->getOuterAngleCos());

            snprintf(buf, 256, "u_spotLightRangeInverse[%d]", SPOT_LIGHT_COUNT);
            getParameter(buf)->setFloat(light->getRangeInverse());

            snprintf(buf, 256, "u_spotLightDirection[%d]", SPOT_LIGHT_COUNT);
            Vector3 sv = light->getNode()->getForwardVector();
            camera->getViewMatrix().transformVector(&sv);
            getParameter(buf)->setVector3(sv);

            snprintf(buf, 256, "u_spotLightPosition[%d]", SPOT_LIGHT_COUNT);
            Vector3 sp = light->getNode()->getTranslation();
            camera->getViewMatrix().transformPoint(&sp);
            getParameter(buf)->setVector3(sp);

            ++SPOT_LIGHT_COUNT;
        }
            break;
        }
    }
}

void Material::bindCamera(Camera* camera, Rectangle &viewport, Node *node) {
    
    GP_ASSERT(camera);
    GP_ASSERT(node);

    Uniform *uniform = _shaderProgram->getUniform("u_worldViewProjectionMatrix");
    if (uniform) {
        Matrix worldViewProj;
        Matrix::multiply(camera->getViewProjectionMatrix(), node->getWorldMatrix(), &worldViewProj);
        MaterialParameter* param = getParameter("u_worldViewProjectionMatrix");
        param->setMatrix(worldViewProj);
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_worldMatrix");
    if (uniform) {
        MaterialParameter* param = getParameter("u_worldMatrix");
        param->setMatrix(node->getWorldMatrix());
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_viewMatrix");
    if (uniform) {
        MaterialParameter* param = getParameter("u_worldMatrix");
        param->setMatrix(camera->getViewMatrix());
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_projectionMatrix");
    if (uniform) {
        MaterialParameter* param = getParameter("u_projectionMatrix");
        param->setMatrix(camera->getProjectionMatrix());
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_inverseProjectionMatrix");
    if (uniform) {
        MaterialParameter* param = getParameter("u_inverseProjectionMatrix");
        Matrix m = camera->getProjectionMatrix();
        m.invert();
        param->setMatrix(m);
        param->_temporary = true;
    }
    

    uniform = _shaderProgram->getUniform("u_worldViewMatrix");
    if (uniform) {
        Matrix worldViewProj;
        Matrix::multiply(camera->getViewMatrix(), node->getWorldMatrix(), &worldViewProj);
        MaterialParameter* param = getParameter("u_worldViewMatrix");
        param->setMatrix(worldViewProj);
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_viewProjectionMatrix");
    if (uniform) {
        MaterialParameter* param = getParameter("u_viewProjectionMatrix");
        param->setMatrix(camera->getViewProjectionMatrix());
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_inverseTransposeWorldMatrix");
    if (uniform) {
        Matrix invTransWorld;
        invTransWorld = node->getWorldMatrix();
        invTransWorld.invert();
        invTransWorld.transpose();
        MaterialParameter* param = getParameter("u_inverseTransposeWorldMatrix");
        param->setMatrix(invTransWorld);
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_inverseTransposeWorldViewMatrix");
    if (uniform) {
        Matrix invTransWorld;
        Matrix::multiply(camera->getViewMatrix(), node->getWorldMatrix(), &invTransWorld);
        invTransWorld.invert();
        invTransWorld.transpose();
        MaterialParameter* param = getParameter("u_inverseTransposeWorldViewMatrix");
        param->setMatrix(invTransWorld);
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_normalMatrix");
    if (uniform) {
        Matrix invTransWorld;
        Matrix::multiply(camera->getViewMatrix(), node->getWorldMatrix(), &invTransWorld);
        invTransWorld.invert();
        invTransWorld.transpose();
        MaterialParameter* param = getParameter("u_normalMatrix");
        param->setMatrix(invTransWorld);
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_cameraPosition");
    if (uniform) {
        MaterialParameter* param = getParameter("u_cameraPosition");
        param->setVector3(camera->getNode()->getTranslationWorld());
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_nearPlane");
    if (uniform) {
        MaterialParameter* param = getParameter("u_nearPlane");
        param->setFloat(camera->getNearPlane());
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_farPlane");
    if (uniform) {
        MaterialParameter* param = getParameter("u_farPlane");
        param->setFloat(camera->getFarPlane());
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_matrixPalette");
    if (uniform) {
        Model* model = dynamic_cast<Model*>(node->getDrawable());
        if (model)
        {
            MeshSkin* skin = model->getSkin();
            if (skin) {
                MaterialParameter* param = getParameter("u_matrixPalette");
                param->setVector4Array(skin->getMatrixPalette(), skin->getMatrixPaletteSize());
                param->_temporary = true;
            }
        }
    }

    uniform = _shaderProgram->getUniform("u_morphWeights");
    if (uniform) {
        if (node)
        {
            int n = node->getWeights().size();
            std::vector<float> weights(n);
            for (int i = 0; i < n; ++i) {
                weights[i] = node->getWeights()[i];
            }
            MaterialParameter* param = getParameter("u_morphWeights");
            param->setFloatArray(weights.data(), n, true);
            param->_temporary = true;
        }
    }

    uniform = _shaderProgram->getUniform("u_ambientColor");
    if (uniform) {
        Scene* scene = node->getScene();
        MaterialParameter* param = getParameter("u_ambientColor");
        param->setVector3(scene->getAmbientColor());
        param->_temporary = true;
    }

    uniform = _shaderProgram->getUniform("u_viewport");
    if (uniform) {
        MaterialParameter* param = getParameter("u_viewport");
        Vector2 vp(viewport.width, viewport.height);
        param->setVector2(vp);
        param->_temporary = true;
    }


    uniform = _shaderProgram->getUniform("u_time");
    if (uniform) {
        MaterialParameter* param = getParameter("u_time");
        double milliTime = Toolkit::cur()->getGameTime();
        param->setFloat(milliTime / (double)1000);
        param->_temporary = true;
    }
}

UPtr<Material> Material::create(Properties* materialProperties, PassCallback callback, void* cookie)
{
    // Check if the Properties is valid and has a valid namespace.
    if (!materialProperties || !(strcmp(materialProperties->getNamespace(), "material") == 0))
    {
        GP_ERROR("Properties object must be non-null and have namespace equal to 'material'.");
        return UPtr<Material>(NULL);
    }

    // Create new material from the file passed in.
    Material* material = new Material();

    material->setName(materialProperties->getId());

    // Load uniform value parameters for this material.
    loadRenderState(material, materialProperties);

    // Fetch shader info required to create the effect of this technique.
    const char* vertexShaderPath = materialProperties->getString("vertexShader");
    GP_ASSERT(vertexShaderPath);
    const char* fragmentShaderPath = materialProperties->getString("fragmentShader");
    GP_ASSERT(fragmentShaderPath);
    const char* passDefines = materialProperties->getString("defines");

    // If a pass callback was specified, call it and add the result to our list of defines
    std::string allDefines = passDefines ? passDefines : "";
    if (callback)
    {
        std::string customDefines = callback(material, cookie);
        if (customDefines.length() > 0)
        {
            if (allDefines.length() > 0)
                allDefines += ';';
            allDefines += customDefines;
        }
    }

    material->vertexShaderPath = vertexShaderPath;
    material->fragmentShaderPath = fragmentShaderPath;
    material->shaderDefines = allDefines;

    return UPtr<Material>(material);
}

UPtr<Material> Material::create(ShaderProgram* effect)
{
    GP_ASSERT(effect);

    // Create a new material with a single technique and pass for the given effect.
    Material* material = new Material();
    material->_shaderProgram = effect;
    effect->addRef();

    return UPtr<Material>(material);
}

UPtr<Material> Material::create(const char* vshPath, const char* fshPath, const char* defines)
{
    GP_ASSERT(vshPath);
    GP_ASSERT(fshPath);

    // Create a new material with a single technique and pass for the given effect
    Material* material = new Material();

    material->vertexShaderPath = vshPath;
    material->fragmentShaderPath = fshPath;
    material->shaderDefines = defines == NULL ? "" : defines;

    return UPtr<Material>(material);
}

const std::string& Material::getShaderDefines() {
    return shaderDefines;
}
void Material::setShaderDefines(const std::string& defiens) {
    if (defiens != shaderDefines) {
        if (_shaderProgram) {
            SAFE_RELEASE(_shaderProgram);
        }
        shaderDefines = defiens;
    }
}

void Material::getShaderId(std::string& uniqueId) {
    uniqueId = vertexShaderPath;
    uniqueId += ';';
    uniqueId += fragmentShaderPath;
    uniqueId += ';';
    if (shaderDefines.size() > 0)
    {
        uniqueId += shaderDefines;
    }
}

//void Material::setNodeBinding(Node* node)
//{
//    _node = node;
//
//    //_paramBinding.setNodeBinding(node);
//
//    if (_nextPass) _nextPass->setNodeBinding(node);
//}

//void Material::setParameterAutoBinding(const char* name, MaterialParamBinding::AutoBinding autoBinding) {
//    _paramBinding.setParameterAutoBinding(name, autoBinding);
//}
//
//
//void Material::setParameterAutoBinding(const char* name, const char* autoBinding) {
//    _paramBinding.setParameterAutoBinding(name, autoBinding);
//}

UPtr<Material> Material::clone(NodeCloneContext &context) const
{
    Material* material = new Material();

    for (std::vector<MaterialParameter*>::const_iterator it = _parameters.begin(); it != _parameters.end(); ++it)
    {
        const MaterialParameter* param = *it;
        GP_ASSERT(param);

        // If this parameter is a method binding auto binding, don't clone it - it will get setup automatically
        // via the cloned auto bindings instead.
        if (param->_methodBinding && param->_methodBinding->_autoBinding)
            continue;

        MaterialParameter* paramCopy = new MaterialParameter(param->getName());
        param->cloneInto(paramCopy);

        material->_parameters.push_back(paramCopy);
    }

    // Clone our state block
    _state.cloneInto(material->getStateBlock());

    //_paramBinding.cloneInto(&material->_paramBinding, context);

    if (_shaderProgram) {
        _shaderProgram->addRef();
        material->_shaderProgram = _shaderProgram;
    }

    material->name = this->name;
    material->vertexShaderPath = this->vertexShaderPath;
    material->fragmentShaderPath = this->fragmentShaderPath;
    material->shaderDefines = this->shaderDefines;

    if (_nextPass.get()) material->_nextPass = _nextPass->clone(context);
    return UPtr<Material>(material);
}


ShaderProgram* Material::getEffect() const {
    return _shaderProgram;
}

//void Material::setVertexAttributeBinding(VertexAttributeBinding* binding)
//{
//    SAFE_RELEASE(_vertexAttributeBinding);
//
//    if (binding)
//    {
//        _vertexAttributeBinding = binding;
//        binding->addRef();
//    }
//
//    if (_nextPass) _nextPass->setVertexAttributeBinding(binding);
//}
//
//VertexAttributeBinding* Material::getVertexAttributeBinding() const
//{
//    return _vertexAttributeBinding;
//}

void Material::bind() {
    if (!_shaderProgram) {
        initialize(NULL, NULL, 0);
    }
    GP_ASSERT(_shaderProgram);
    // Bind our effect.
    _shaderProgram->bind();

    std::map<std::string, int> allParams;
    // Bind our render state
    for (size_t i = 0, count = _parameters.size(); i < count; ++i)
    {
        GP_ASSERT(_parameters[i]);
        _parameters[i]->bind(this->_shaderProgram);
        allParams[_parameters[i]->getName()] = 1;
    }
    _state.bind();

    for (int i = 0; i < _shaderProgram->getUniformCount(); ++i) {
        auto uniform = _shaderProgram->getUniform(i);
        std::string name = uniform->getName();
        if (allParams.find(name) == allParams.end() && allParams.find(name+"[0]") == allParams.end()) {
            GP_ERROR("Uniform not set: %s", name.c_str());
        }
    }
}

void Material::setParams(std::vector<Light*>* lights,
        Camera* camera,
        Rectangle* viewport, Drawable* drawable)
{
    int lightMask = 0;
    if (drawable) {
        lightMask = drawable->getLightMask();
    }

    if (!initialize(drawable, lights, lightMask)) {
        return;
    }
    
    if (camera) bindLights(camera, lights, lightMask);

    if (camera && drawable && drawable->getNode()) {
        bindCamera(camera, *viewport, drawable->getNode());
    }
    else {
        //printf("DEBUG");
    }
}

void Material::unbind()
{
    // If we have a vertex attribute binding, unbind it
    /*if (_vertexAttributeBinding)
    {
        _vertexAttributeBinding->unbind();
    }*/
}

Material* Material::getNextPass() {
    return _nextPass.get();
}

void Material::setNextPass(UPtr<Material> next) {
    _nextPass = std::move(next);
}

Serializable* Material::createObject() {
    return new Material();
}

/**
 * @see Serializable::getClassName
 */
std::string Material::getClassName() {
    return "mgp::Material";
}

/**
 * @see Serializable::onSerialize
 */
void Material::onSerialize(Serializer* serializer) {
    //serializer->writeString("name", getName().c_str(), "");
    serializer->writeString("vertexShaderPath", vertexShaderPath.c_str(), "");
    serializer->writeString("fragmentShaderPath", fragmentShaderPath.c_str(), "");
    serializer->writeString("shaderDefines", shaderDefines.c_str(), "");

    /*
    std::vector<std::string> params;
    for (int i=0; i<getParameterCount(); ++i) {
        MaterialParameter *p = getParameterByIndex(i);
        params.push_back(p->getName());
    }
    */

    int count = 0;
    for (int i = 0; i < getParameterCount(); ++i) {
        MaterialParameter* p = getParameterByIndex(i);
        if (p->_temporary) continue;
        ++count;
    }

    serializer->writeList("parameters", count);
    for (int i=0; i<getParameterCount(); ++i) {
        MaterialParameter *p = getParameterByIndex(i);
        if (p->_temporary) continue;
        serializer->writeObject(NULL, p);
    }
    serializer->finishColloction();
}

/**
 * @see Serializable::onDeserialize
 */
void Material::onDeserialize(Serializer* serializer) {
    serializer->readString("vertexShaderPath", vertexShaderPath, "");
    serializer->readString("fragmentShaderPath", fragmentShaderPath, "");
    serializer->readString("shaderDefines", shaderDefines, "");


    int size = serializer->readList("parameters");
    for (int i = 0; i < size; ++i) {
        MaterialParameter* p = dynamic_cast<MaterialParameter*>(serializer->readObject(NULL));
        _parameters.push_back(p);
    }
    serializer->finishColloction();
}

void Material::setStateBlock(StateBlock* state)
{
    _state = *state;
}

StateBlock* Material::getStateBlock() const
{
    return (StateBlock*)(&_state);
}

MaterialParameter* Material::getParameter(const char* name, bool add) const
{
    GP_ASSERT(name);

    // Search for an existing parameter with this name.
    MaterialParameter* param;
    for (size_t i = 0, count = _parameters.size(); i < count; ++i)
    {
        param = _parameters[i];
        GP_ASSERT(param);
        if (strcmp(param->getName(), name) == 0)
        {
            return param;
        }
    }

    if (!add) return NULL;

    // Create a new parameter and store it in our list.
    param = new MaterialParameter(name);
    _parameters.push_back(param);

    return param;
}

unsigned int Material::getParameterCount() const
{
    return _parameters.size();
}

MaterialParameter* Material::getParameterByIndex(unsigned int index)
{
    return _parameters[index];
}

void Material::addParameter(MaterialParameter* param)
{
    _parameters.push_back(param);
    param->addRef();
}

void Material::removeParameter(const char* name)
{
    for (size_t i = 0, count = _parameters.size(); i < count; ++i)
    {
        MaterialParameter* p = _parameters[i];
        if (p->_name == name)
        {
            _parameters.erase(_parameters.begin() + i);
            SAFE_RELEASE(p);
            break;
        }
    }
}
}
