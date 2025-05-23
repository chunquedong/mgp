#include "base/Base.h"
#include "Material.h"
#include "base/FileSystem.h"
#include "ShaderProgram.h"
#include "scene/Node.h"
#include "MaterialParameter.h"
#include "platform/Toolkit.h"
#include "scene/Scene.h"
#include "base/SerializerJson.h"

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
    for (auto it = _parameters.begin(); it != _parameters.end(); ++it)
    {
        MaterialParameter* param = it->second;
        SAFE_RELEASE(param);
    }
    _parameters.clear();
    SAFE_RELEASE(_shaderProgram);
    //SAFE_RELEASE(_vertexAttributeBinding);
}

UPtr<Material> Material::create(const std::string& name, const char* defines)
{
    std::string v = "res/shaders/" + name + ".vert";
    std::string f = "res/shaders/" + name + ".frag";
    return create(v.c_str(), f.c_str(), defines);
}

bool Material::initialize(Drawable* drawable, std::vector<Light*>* lights, int lightMask, int instanced)
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

    if (instanced) {
        if (dynamicDefines.size() > 0) {
            dynamicDefines += ";";
        }
        dynamicDefines += "INSTANCED";
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
            getParameter(buf, true, true)->setVector3(light->getColor());

            snprintf(buf, 256, "u_directionalLightDirection[%d]", DIRECTIONAL_LIGHT_COUNT);
            Vector3 v = light->getNode()->getForwardVector();
            camera->getViewMatrix().transformVector(&v);
            getParameter(buf, true, true)->setVector3(v);

            ++DIRECTIONAL_LIGHT_COUNT;
        }
            break;
        case Light::POINT: {
            snprintf(buf, 256, "u_pointLightColor[%d]", POINT_LIGHT_COUNT);
            getParameter(buf, true, true)->setVector3(light->getColor());

            snprintf(buf, 256, "u_pointLightPosition[%d]", POINT_LIGHT_COUNT);
            Vector3 p = light->getNode()->getTranslation();
            camera->getViewMatrix().transformPoint(&p);
            getParameter(buf, true, true)->setVector3(p);

            snprintf(buf, 256, "u_pointLightRangeInverse[%d]", POINT_LIGHT_COUNT);
            getParameter(buf, true, true)->setFloat(light->getRangeInverse());

            ++POINT_LIGHT_COUNT;
        }
            break;
        case Light::SPOT: {
            snprintf(buf, 256, "u_spotLightColor[%d]", SPOT_LIGHT_COUNT);
            getParameter(buf, true, true)->setVector3(light->getColor());

            snprintf(buf, 256, "u_spotLightInnerAngleCos[%d]", SPOT_LIGHT_COUNT);
            getParameter(buf, true, true)->setFloat(light->getInnerAngleCos());

            snprintf(buf, 256, "u_spotLightOuterAngleCos[%d]", SPOT_LIGHT_COUNT);
            getParameter(buf, true, true)->setFloat(light->getOuterAngleCos());

            snprintf(buf, 256, "u_spotLightRangeInverse[%d]", SPOT_LIGHT_COUNT);
            getParameter(buf, true, true)->setFloat(light->getRangeInverse());

            snprintf(buf, 256, "u_spotLightDirection[%d]", SPOT_LIGHT_COUNT);
            Vector3 sv = light->getNode()->getForwardVector();
            camera->getViewMatrix().transformVector(&sv);
            getParameter(buf, true, true)->setVector3(sv);

            snprintf(buf, 256, "u_spotLightPosition[%d]", SPOT_LIGHT_COUNT);
            Vector3 sp = light->getNode()->getTranslation();
            camera->getViewMatrix().transformPoint(&sp);
            getParameter(buf, true, true)->setVector3(sp);

            ++SPOT_LIGHT_COUNT;
        }
            break;
        }
    }
}

void Material::bindNode(Camera* camera, Node *node, Drawable* drawable, Rectangle& viewport) {
    
    GP_ASSERT(camera);

    for (auto uniformItr = _shaderProgram->getUniforms().begin(); uniformItr != _shaderProgram->getUniforms().end(); ++uniformItr) {
        const std::string& name = uniformItr->first;
        Uniform* uniform = uniformItr->second;

        if (node) {
            bool ok = true;
            if (name == "u_worldViewProjectionMatrix") {
                Matrix worldViewProj;
                Matrix::multiply(camera->getViewProjectionMatrix(), node->getWorldMatrix(), &worldViewProj);
                MaterialParameter* param = getParameter("u_worldViewProjectionMatrix");
                param->setMatrix(worldViewProj);
                param->_temporary = true;

                Uniform* uniform2 = _shaderProgram->getUniform("u_inverseWorldViewProjectionMatrix");
                if (uniform2) {
                    worldViewProj.invert();
                    MaterialParameter* param = getParameter("u_inverseWorldViewProjectionMatrix");
                    param->setMatrix(worldViewProj);
                    param->_temporary = true;
                }
            }

            else if (name == "u_worldMatrix") {
                MaterialParameter* param = getParameter("u_worldMatrix");
                param->setMatrix(node->getWorldMatrix());
                param->_temporary = true;
            }

            else if (name == "u_worldViewMatrix") {
                Matrix worldViewProj;
                Matrix::multiply(camera->getViewMatrix(), node->getWorldMatrix(), &worldViewProj);
                MaterialParameter* param = getParameter("u_worldViewMatrix");
                param->setMatrix(worldViewProj);
                param->_temporary = true;
            }

            else if (name == "u_inverseTransposeWorldMatrix") {
                Matrix invTransWorld;
                invTransWorld = node->getWorldMatrix();
                invTransWorld.invert();
                invTransWorld.transpose();
                MaterialParameter* param = getParameter("u_inverseTransposeWorldMatrix");
                param->setMatrix(invTransWorld);
                param->_temporary = true;
            }

            else if (name == "u_inverseTransposeWorldViewMatrix") {
                Matrix invTransWorld;
                Matrix::multiply(camera->getViewMatrix(), node->getWorldMatrix(), &invTransWorld);
                invTransWorld.invert();
                invTransWorld.transpose();
                MaterialParameter* param = getParameter("u_inverseTransposeWorldViewMatrix");
                param->setMatrix(invTransWorld);
                param->_temporary = true;
            }

            else if (name == "u_normalMatrix") {
                Matrix invTransWorld;
                Matrix::multiply(camera->getViewMatrix(), node->getWorldMatrix(), &invTransWorld);
                invTransWorld.invert();
                invTransWorld.transpose();
                MaterialParameter* param = getParameter("u_normalMatrix");
                param->setMatrix(invTransWorld);
                param->_temporary = true;
            }

            else if (name == "u_matrixPalette") {
                Model* model = dynamic_cast<Model*>(drawable);
                if (model)
                {
                    MeshSkin* skin = model->getSkin();
                    if (skin) {
                        MaterialParameter* param = getParameter("u_matrixPalette");
                        param->setVector4Array(skin->getMatrixPalette(&camera->getViewMatrix(), node), skin->getMatrixPaletteSize());
                        param->_temporary = true;
                    }
                }
            }

            else if (name == "u_morphWeights") {
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

            else if (name == "u_ambientColor") {
                Scene* scene = node->getScene();
                MaterialParameter* param = getParameter("u_ambientColor");
                param->setVector3(scene->getAmbientColor());
                param->_temporary = true;
            }
            else {
                ok = false;
            }

            if (ok) 
                continue;
        }

        if (name == "u_viewMatrix") {
            MaterialParameter* param = getParameter("u_worldMatrix");
            param->setMatrix(camera->getViewMatrix());
            param->_temporary = true;
        }

        else if (name == "u_projectionMatrix") {
            MaterialParameter* param = getParameter("u_projectionMatrix");
            param->setMatrix(camera->getProjectionMatrix());
            param->_temporary = true;
        }

        else if (name == "u_inverseProjectionMatrix") {
            MaterialParameter* param = getParameter("u_inverseProjectionMatrix");
            Matrix m = camera->getProjectionMatrix();
            m.invert();
            param->setMatrix(m);
            param->_temporary = true;
        }

        else if (name == "u_viewProjectionMatrix") {
            MaterialParameter* param = getParameter("u_viewProjectionMatrix");
            param->setMatrix(camera->getViewProjectionMatrix());
            param->_temporary = true;
        }

        else if (name == "u_cameraPosition") {
            MaterialParameter* param = getParameter("u_cameraPosition");
            param->setVector3(camera->getNode()->getTranslationWorld());
            param->_temporary = true;
        }

        else if (name == "u_nearPlane") {
            MaterialParameter* param = getParameter("u_nearPlane");
            param->setFloat(camera->getNearPlane());
            param->_temporary = true;
        }

        else if (name == "u_farPlane") {
            MaterialParameter* param = getParameter("u_farPlane");
            param->setFloat(camera->getFarPlane());
            param->_temporary = true;
        }

        else if (name == "u_fovDivisor") {
            MaterialParameter* param = getParameter("u_fovDivisor");
            double fovDivisor = tan(MATH_DEG_TO_RAD(camera->getFieldOfView()) / 2) / (viewport.height / 2);
            param->setFloat(fovDivisor);
            param->_temporary = true;
        }

        else if (name == "u_viewport") {
            MaterialParameter* param = getParameter("u_viewport");
            Vector2 vp(viewport.width, viewport.height);
            param->setVector2(vp);
            param->_temporary = true;
        }

        else if (name == "u_time") {
            MaterialParameter* param = getParameter("u_time");
            double milliTime = Toolkit::cur()->getGameTime();
            param->setFloat(milliTime / (double)1000);
            param->_temporary = true;
        }
    }
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

UPtr<Material> Material::clone() const
{
    Material* material = new Material();
    material->copyFrom(this);
    return UPtr<Material>(material);
}

void Material::copyFrom(const Material* src) {
    Material* material = this;
    material->_parameters.clear();
    for (auto it = src->_parameters.begin(); it != src->_parameters.end(); ++it)
    {
        const MaterialParameter* param = it->second;
        GP_ASSERT(param);

        // If this parameter is a method binding auto binding, don't clone it - it will get setup automatically
        // via the cloned auto bindings instead.
        if (param->_methodBinding && param->_methodBinding->_autoBinding)
            continue;

        MaterialParameter* paramCopy = new MaterialParameter(param->getName());
        param->cloneInto(paramCopy);

        material->_parameters[it->first] = paramCopy;
    }

    // Clone our state block
    src->getStateBlock()->cloneInto(&material->_state);

    //_paramBinding.cloneInto(&material->_paramBinding, context);

    if (src->_shaderProgram) {
        src->_shaderProgram->addRef();
        if (material->_shaderProgram) {
            material->_shaderProgram->release();
        }
        material->_shaderProgram = src->_shaderProgram;
    }

    //material->name = this->name;
    material->vertexShaderPath = src->vertexShaderPath;
    material->fragmentShaderPath = src->fragmentShaderPath;
    material->shaderDefines = src->shaderDefines;

    if (src->_nextPass.get()) material->_nextPass = src->_nextPass->clone();
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
        initialize(NULL, NULL, 0, 0);
    }
    GP_ASSERT(_shaderProgram);
    // Bind our effect.
    _shaderProgram->bind();

    // Bind our render state
    for (auto it = _parameters.begin(); it != _parameters.end(); ++it)
    {
        MaterialParameter* p = it->second;
        GP_ASSERT(p);
        p->bind(this->_shaderProgram);
    }
    _state.bind();

    for (auto uniformItr = _shaderProgram->getUniforms().begin(); uniformItr != _shaderProgram->getUniforms().end(); ++uniformItr) {
        const std::string& name = uniformItr->first;
        Uniform* uniform = uniformItr->second;
        if (_parameters.find(name) == _parameters.end() && _parameters.find(name+"[0]") == _parameters.end()) {
            GP_ERROR("Uniform not set: %s", name.c_str());
        }
    }
}

void Material::setParams(std::vector<Light*>* lights,
        Camera* camera,
        Rectangle* viewport, Drawable* drawable, int instanced)
{
    int lightMask = 0;
    if (drawable) {
        lightMask = drawable->getLightMask();
    }

    if (!initialize(drawable, lights, lightMask, instanced)) {
        return;
    }
    
    if (camera) bindLights(camera, lights, lightMask);

    if (camera && drawable) {
        bindNode(camera, drawable != nullptr ? drawable->getNode() : nullptr, drawable, *viewport);
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

    _state.onSerialize(serializer);

    int count = 0;
    for (auto it = _parameters.begin(); it != _parameters.end(); ++it) {
        MaterialParameter* p = it->second;
        if (p->_temporary) continue;
        ++count;
    }

    serializer->writeList("parameters", count);
    for (auto it = _parameters.begin(); it != _parameters.end(); ++it) {
        MaterialParameter *p = it->second;
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

    _state.onDeserialize(serializer);

    int size = serializer->readList("parameters");
    for (int i = 0; i < size; ++i) {
        MaterialParameter* p = dynamic_cast<MaterialParameter*>(serializer->readObject(NULL).take());
        _parameters[p->getName()] = p;
    }
    serializer->finishColloction();
}

void Material::write(Stream* file) {
    auto stream = SerializerJson::create(file);
    stream->writeObject(nullptr, this);
    stream->flush();
}

bool Material::read(Stream* file) {
    auto stream = SerializerJson::create(file);
    UPtr<Material> m = stream->readObject(nullptr).dynamicCastTo<Material>();
    this->copyFrom(m.get());
    return true;
}

void Material::setStateBlock(StateBlock* state)
{
    _state = *state;
}

StateBlock* Material::getStateBlock() const
{
    return (StateBlock*)(&_state);
}

MaterialParameter* Material::getParameter(const char* name, bool add, bool temporary) const
{
    GP_ASSERT(name);

    // Search for an existing parameter with this name.
    auto it = _parameters.find(name);
    if (it != _parameters.end()) {
        return it->second;
    }

    if (!add) return NULL;

    // Create a new parameter and store it in our list.
    auto param = new MaterialParameter(name);
    _parameters[name] = (param);
    param->_temporary = temporary;

    return param;
}

unsigned int Material::getParameterCount() const
{
    return _parameters.size();
}

//MaterialParameter* Material::getParameterByIndex(unsigned int index)
//{
//
//}

void Material::addParameter(MaterialParameter* param)
{
    _parameters[param->getName()] = (param);
    param->addRef();
}

void Material::removeParameter(const char* name)
{
    auto it = _parameters.find(name);
    if (it != _parameters.end()) {
        MaterialParameter* p = it->second;
        _parameters.erase(it);
        SAFE_RELEASE(p);
    }
}
}
