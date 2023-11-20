#include "Base.h"
#include "SerializerManager.h"
//#include "platform/Toolkit.h"
//#include "scene/Scene.h"
//#include "scene/Node.h"
//#include "scene/Camera.h"
//#include "scene/Light.h"
//#include "scene/Model.h"
//#include "../material/Material.h"
//#include "../material/MaterialParameter.h"
//#include "../material/Texture.h"

namespace mgp
{

static SerializerManager *g_activator;

SerializerManager::SerializerManager()
{
}

SerializerManager::~SerializerManager()
{
}

void SerializerManager::releaseStatic() {
    SAFE_DELETE(g_activator);
}

SerializerManager* SerializerManager::getActivator()
{
    if (!g_activator) {
        g_activator = new SerializerManager();
        g_activator->registerSystemTypes();
    }
    return g_activator;
}

Serializable *SerializerManager::createObject(const std::string& className)
{
    Serializable *object = nullptr;
    std::map<std::string, CreateObjectCallback>::const_iterator itr = _classes.find(className);
    if (itr == _classes.end())
    {
        return nullptr;
    }
    
    CreateObjectCallback createObject = itr->second;
    if (createObject)
    {
        object = createObject();
    }
    return object;
}

std::string SerializerManager::enumToString(const std::string& enumName, int value)
{
    std::map<std::string,std::pair<EnumToStringCallback, EnumParseCallback> >::const_iterator itr = _enums.find(enumName);
    if (itr != _enums.end())
    {
        EnumToStringCallback enumToString = itr->second.first;
        if (enumToString)
        {
            return enumToString(enumName, value);
        }
    }
    return "";
}

int SerializerManager::enumParse(const std::string& enumName, const std::string& str)
{
    std::map<std::string,std::pair<EnumToStringCallback, EnumParseCallback> >::const_iterator itr = _enums.find(enumName);
    if (itr != _enums.end())
    {
        EnumParseCallback enumParse = itr->second.second;
        if (enumParse)
        {
            return enumParse(enumName, str);
        }
    }
    return 0;
}

void SerializerManager::registerType(const std::string&  className, CreateObjectCallback createObject)
{
    std::map<std::string,CreateObjectCallback>::const_iterator itr = _classes.find(className);
    if ( itr == _classes.end() )
    {
        _classes[className] = createObject;
    }
    else
    {
        GP_ERROR("className already registered:%s", className.c_str());
    }
}
    
void SerializerManager::registerEnum(const std::string& enumName, EnumToStringCallback toString, EnumParseCallback parse)
{
    std::map<std::string, std::pair<EnumToStringCallback, EnumParseCallback> >::const_iterator itr = _enums.find(enumName);
    if (itr == _enums.end())
    {
        _enums[enumName] = std::make_pair(toString, parse);
    }
    else
    {
        GP_ERROR("enumName already registered:%s", enumName.c_str());
    }
}

void SerializerManager::registerSystemTypes()
{
    // Register engine types with
    //this->registerType("mgp::Game::Config", Game::Config::createObject);
    //this->registerType("mgp::Scene", Scene::createObject);
    //this->registerType("mgp::Node", Node::createObject);
    //this->registerType("mgp::Camera", Camera::createObject);
    //this->registerType("mgp::Light", Light::createObject);
    //this->registerType("mgp::Model", Model::createObject);
    //this->registerType("mgp::Material", Material::createObject);
    //this->registerType("mgp::Texture", Texture::createObject);
    //this->registerType("mgp::MaterialParameter", MaterialParameter::createObject);

    //// Register engine enums
    //this->registerEnum("mgp::Camera::Mode", Camera::enumToString, Camera::enumParse);
    //this->registerEnum("mgp::Light::Type", Light::enumToString, Light::enumParse);
    //this->registerEnum("mgp::Light::Mode", Light::enumToString, Light::enumParse);
    //this->registerEnum("mgp::Light::Shadows", Light::enumToString, Light::enumParse);

    //this->registerEnum("mgp::Texture::Format", Texture::enumToString, Texture::enumParse);
    //this->registerEnum("mgp::Texture::Type", Texture::enumToString, Texture::enumParse);
    //this->registerEnum("mgp::Texture::Wrap", Texture::enumToString, Texture::enumParse);
    //this->registerEnum("mgp::Texture::Filter", Texture::enumToString, Texture::enumParse);

    //this->registerEnum("mgp::MaterialParameter::Type", MaterialParameter::enumToString, MaterialParameter::enumParse);
}

}
