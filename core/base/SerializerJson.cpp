#include "Base.h"
#include "SerializerJson.h"
#include "SerializerManager.h"
#include "Serializer.h"
#include "FileSystem.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "math/Matrix.h"

#include "jparser.hpp"
#include "HimlParser.hpp"

extern "C" {
#include "3rd/base64.h"
}

#include <iostream>
#include <sstream>

using namespace jc;

namespace mgp
{

static JsonNode* json_new_f(jc::JsonAllocator& allocator, float f) {
    return allocator.alloc_float(f);
}

static char* json_strdup(jc::JsonAllocator& allocator, const char* s) {
    return allocator.strdup(s);
}

static JsonNode* json_new_a(jc::JsonAllocator& allocator, const char* s) {
    return allocator.alloc_str(s);
}

SerializerJson::SerializerJson(Type type,
                               Stream* stream,
                               uint32_t versionMajor,
                               uint32_t versionMinor,
                               jc::JsonNode* root) :
    Serializer(type, stream, versionMajor, versionMinor), 
    _root(root), _isHiml(false)
{
    _nodes.push(root);
}

SerializerJson::~SerializerJson()
{
}

UPtr<Serializer> SerializerJson::create(Stream* stream, bool isHiml)
{
    jc::JsonAllocator allocator;

    size_t length = stream->length();
    char* buffer = (char*)allocator.allocate(length + 1);
    stream->read(buffer, sizeof(char), length);
    buffer[length] = '\0';

    JsonNode* root = nullptr;
    if (isHiml) {
        HimlParser parser(&allocator);
        root = (JsonNode*)parser.parse(buffer);
        if (root->children() && root->get("version") == NULL && root->children()->begin() != root->children()->end()) {
            root = (JsonNode*)*root->children()->begin();
        }
    }
    else {
        JsonParser parser(&allocator);
        root = (JsonNode*)parser.parse(buffer);
    }

    if (root == nullptr)
        return UPtr<Serializer>();

    SerializerJson* serializer = nullptr;

    auto versionNode = root->get("version");
    int versionMajor = GP_ENGINE_VERSION_MAJOR;
    int versionMinor = GP_ENGINE_VERSION_MINOR;
    if (versionNode)
    {
        const char* str = versionNode->as_str();
        std::string version = std::string(str);
        //json_free(str);
        if (version.length() > 0)
        {
            std::string major = version.substr(0, 1);
            versionMajor = std::stoi(major);
        }
        if (version.length() > 2)
        {
            std::string minor = version.substr(2, 1);
            versionMinor = std::stoi(minor);
        }
        serializer = new SerializerJson(Type::eReader, stream, versionMajor, versionMinor, root);
        serializer->_isHiml = isHiml;
        serializer->allocator.swap(allocator);
    }
    //SAFE_DELETE_ARRAY(buffer);
    return UPtr<Serializer>(serializer);
}


UPtr<Serializer> SerializerJson::createWriter(const std::string& path, bool isHiml)
{
    Stream* stream = FileSystem::open(path.c_str(), FileSystem::WRITE).take();
    if (stream == nullptr)
        return UPtr<Serializer>();
    jc::JsonAllocator allocator;
    jc::JsonNode* root = allocator.allocNode(jc::Type::Object);
    std::string version;
    version.append(std::to_string(GP_ENGINE_VERSION_MAJOR));
    version.append(".");
    version.append(std::to_string(GP_ENGINE_VERSION_MINOR));

    root->insert_pair("version", json_new_a(allocator, version.c_str()));
    //json_push_back(root, json_new_a("version", version.c_str()));

    SerializerJson* serializer = new SerializerJson(Type::eWriter, stream, GP_ENGINE_VERSION_MAJOR, GP_ENGINE_VERSION_MINOR, root);
    serializer->allocator.swap(allocator);
    serializer->_isHiml = isHiml;
    return UPtr<Serializer>(serializer);
}

void SerializerJson::close()
{
    if (_stream)
    {
        flush();
        _stream->close();
    }
}

void SerializerJson::flush() {
    if (_type == Type::eWriter && _root)
    {
        std::string str;
        _root->to_json(str, _isHiml);
        _stream->write(str.c_str(), sizeof(char), str.length());
        //json_free(buffer);
        _root = NULL;
    }
}

Serializer::Format SerializerJson::getFormat() const
{
    return Format::eJson;
}

void SerializerJson::writeEnum(const char* propertyName, const char* enumName, int value, int defaultValue)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(enumName);
    
    if (value == defaultValue)
        return;
    
    std::string str = SerializerManager::getActivator()->enumToString(enumName, value);
    writeString(propertyName, str.c_str(), "");
}
    
void SerializerJson::writeBool(const char* propertyName, bool value, bool defaultValue)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);
    
    if (value == defaultValue)
        return;
    
    jc::JsonNode* node = _nodes.top();
    jc::JsonNode* json_value = (JsonNode*)allocator.allocate(sizeof(JsonNode));
    json_value->set_bool(value);
    node->insert_pair(json_strdup(allocator, propertyName), json_value);
}

void SerializerJson::writeInt(const char* propertyName, int value, int defaultValue)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);
    
    if (value == defaultValue)
        return;
    
    jc::JsonNode* node = _nodes.top();
    jc::JsonNode* json_value = (JsonNode*)allocator.allocate(sizeof(JsonNode));
    json_value->set_int(value);
    node->insert_pair(json_strdup(allocator, propertyName), json_value);
}

void SerializerJson::writeFloat(const char* propertyName, float value, float defaultValue)
{
    GP_ASSERT(_type == Type::eWriter);
    
    if (value == defaultValue)
        return;

    jc::JsonNode* node = _nodes.top();
    jc::JsonNode* json_value = (JsonNode*)allocator.allocate(sizeof(JsonNode));
    json_value->set_float(value);
    if (propertyName) {
        node->insert_pair(json_strdup(allocator, propertyName), json_value);
    }
    else {
        node->insert(json_value);
    }
}


void SerializerJson::writeVector(const char* propertyName, const Vector2& value, const Vector2& defaultValue)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);
    
    if (value == defaultValue)
        return;
    
    // "properyName" : [ x, y ]
    jc::JsonNode* node = _nodes.top();
    //jc::JsonNode* array = json_new(JSON_ARRAY);
    //json_set_name(array, propertyName);
    //json_push_back(array, json_new_f(nullptr, value.x));
    //json_push_back(array, json_new_f(nullptr, value.y));
    //json_push_back(node, array);

    jc::JsonNode* array = (JsonNode*)allocator.allocNode(jc::Type::Array);
    auto p1 = json_new_f(allocator, value.x);
    auto p2 = json_new_f(allocator, value.y);
    array->append(p1);
    array->append(p2);
    node->insert_pair(json_strdup(allocator, propertyName), array);
}

void SerializerJson::writeVector(const char* propertyName, const Vector3& value, const Vector3& defaultValue)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);

    if (value == defaultValue)
        return;

    // "properyName" : [ x, y, z ]
    jc::JsonNode* node = _nodes.top();
    /*jc::JsonNode* array = json_new(JSON_ARRAY);
    json_set_name(array, propertyName);
    json_push_back(array, json_new_f(nullptr, value.x));
    json_push_back(array, json_new_f(nullptr, value.y));
    json_push_back(array, json_new_f(nullptr, value.z));
    json_push_back(node, array);*/

    jc::JsonNode* array = (JsonNode*)allocator.allocNode(jc::Type::Array);
    auto p1 = json_new_f(allocator, value.x);
    auto p2 = json_new_f(allocator, value.y);
    auto p3 = json_new_f(allocator, value.z);
    array->append(p1);
    array->append(p2);
    array->append(p3);
    node->insert_pair(json_strdup(allocator, propertyName), array);
}

void SerializerJson::writeVector(const char* propertyName, const Vector4& value, const Vector4& defaultValue)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);
    
    if (value == defaultValue)
        return;
    
    // "properyName" : [ x, y, z, w ]
    jc::JsonNode* node = _nodes.top();
    /*jc::JsonNode* array = json_new(JSON_ARRAY);
    json_set_name(array, propertyName);
    json_push_back(array, json_new_f(nullptr, value.x));
    json_push_back(array, json_new_f(nullptr, value.y));
    json_push_back(array, json_new_f(nullptr, value.z));
    json_push_back(array, json_new_f(nullptr, value.w));
    json_push_back(node, array);*/

    jc::JsonNode* array = (JsonNode*)allocator.allocNode(jc::Type::Array);
    auto p1 = json_new_f(allocator, value.x);
    auto p2 = json_new_f(allocator, value.y);
    auto p3 = json_new_f(allocator, value.z);
    auto p4 = json_new_f(allocator, value.w);
    array->append(p1);
    array->append(p2);
    array->append(p3);
    array->append(p4);
    node->insert_pair(json_strdup(allocator, propertyName), array);
}

void SerializerJson::writeColor(const char* propertyName, const Vector3& value, const Vector3& defaultValue)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);
    
    if (value == defaultValue)
        return;
    
    // "property" : "#rrggbb"
    jc::JsonNode* node = _nodes.top();
    char buffer[128];
    snprintf(buffer, 128, "#%02x%02x%02x", (int)(value.x * 255.0f), (int)(value.y * 255.0f), (int)(value.z * 255.0f));
    /*std::ostringstream s;
    s << "#" << buffer;
    json_push_back(node, json_new_a(propertyName, s.str().c_str()));*/

    auto str = json_new_a(allocator, buffer);
    node->insert_pair(json_strdup(allocator, propertyName), str);
}
    
void SerializerJson::writeColor(const char* propertyName, const Vector4& value, const Vector4& defaultValue)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);

    if (value == defaultValue)
        return;
    
    // "property" : "#rrggbbaa"
    jc::JsonNode* node = _nodes.top();
    std::ostringstream s;
    s << "#" << std::hex << value.toColor();    
    /*json_push_back(node, json_new_a(propertyName, s.str().c_str()));*/
    auto str = json_new_a(allocator, s.str().c_str());
    node->insert_pair(json_strdup(allocator, propertyName), str);
}
    
void SerializerJson::writeMatrix(const char* propertyName, const Matrix& value, const Matrix& defaultValue)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);

    if (value == defaultValue)
        return;

    // "properyName" : [ m0, ... , m15 ]
    jc::JsonNode* node = _nodes.top();
    //jc::JsonNode* array = json_new(JSON_ARRAY);
    //json_set_name(array, propertyName);
    //json_push_back(array, json_new_f(nullptr, value.m[0]));
    //json_push_back(array, json_new_f(nullptr, value.m[1]));
    //json_push_back(array, json_new_f(nullptr, value.m[2]));
    //json_push_back(array, json_new_f(nullptr, value.m[3]));
    //json_push_back(array, json_new_f(nullptr, value.m[4]));
    //json_push_back(array, json_new_f(nullptr, value.m[5]));
    //json_push_back(array, json_new_f(nullptr, value.m[6]));
    //json_push_back(array, json_new_f(nullptr, value.m[7]));
    //json_push_back(array, json_new_f(nullptr, value.m[8]));
    //json_push_back(array, json_new_f(nullptr, value.m[9]));
    //json_push_back(array, json_new_f(nullptr, value.m[10]));
    //json_push_back(array, json_new_f(nullptr, value.m[11]));
    //json_push_back(array, json_new_f(nullptr, value.m[12]));
    //json_push_back(array, json_new_f(nullptr, value.m[13]));
    //json_push_back(array, json_new_f(nullptr, value.m[14]));
    //json_push_back(array, json_new_f(nullptr, value.m[15]));
    //json_push_back(node, array);

    jc::JsonNode* array = (JsonNode*)allocator.allocNode(jc::Type::Array);
    for (int i = 15; i >= 0; --i) {
        auto p1 = json_new_f(allocator, value.m[i]);
        array->insert(p1);
    }
    node->insert_pair(json_strdup(allocator, propertyName), array);
}

jc::JsonNode* SerializerJson::createNode(jc::JsonNode* parent, const char* propertyName)
{
    if (((parent->type() == jc::Type::Object) && propertyName) || parent->type() == jc::Type::Array)
    {
        jc::JsonNode* value = allocator.allocNode(jc::Type::Object);
        if (parent->type() == jc::Type::Object) {
            parent->insert_pair(json_strdup(allocator, propertyName), value);
        }
        else {
            parent->insert(value);
        }
        
        return value;
    }
    else
    {
        return parent;
    }
}

void SerializerJson::writeString(const char* propertyName, const char* value, const char* defaultValue)
{
    //GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);

    if ((value == defaultValue) || (value && defaultValue && strcmp (value, defaultValue) == 0))
        return;

    jc::JsonNode* node = _nodes.top();
    auto str = json_new_a(allocator, value);
    if (node->type() == jc::Type::Object) {
        node->insert_pair(json_strdup(allocator, propertyName), str);
    }
    else if (node->type() == jc::Type::Array) {
        node->insert(str);
    }
}

void SerializerJson::writeMap(const char* propertyName, std::vector<std::string> &keys)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);
    //if (keys.size() == 0)
    //    return;

    jc::JsonNode* parentNode = _nodes.top();
    jc::JsonNode* writeNode = nullptr;
    writeNode = createNode(parentNode, propertyName);
    writeNode->set_type(jc::Type::Object);

    _nodes.push(writeNode);
    _nodesListCounts.push(keys.size());
}

void SerializerJson::writeObject(const char* propertyName, Serializable *value)
{
    GP_ASSERT(_type == Type::eWriter);
    if (value == nullptr)
        return;

    jc::JsonNode* parentNode = _nodes.top();
    jc::JsonNode* writeNode = nullptr;
    jc::JsonNode* xrefNode = nullptr;

    const char* classField = "class";
    if (_isHiml) {
        classField = "_type";
    }
    Refable* refable = dynamic_cast<Refable*>(value);
    if (refable && refable->getRefCount() > 1)
    {
        unsigned long xrefAddress = (unsigned long)value;
        std::map<unsigned long, jc::JsonNode*>::const_iterator itr = _xrefsWrite.find(xrefAddress);
        std::string url;
        if (itr == _xrefsWrite.end())
        {
            writeNode = createNode(parentNode, propertyName);
            writeNode->insert_pair(classField, json_new_a(allocator, value->getClassName().c_str()));
            url = std::to_string(xrefAddress);
            _xrefsWrite[xrefAddress] = writeNode;
        }
        else
        {
            writeNode = createNode(parentNode, propertyName);
            writeNode->insert_pair(classField, json_new_a(allocator, value->getClassName().c_str()));
            std::ostringstream o;
            o << "@" << std::to_string(xrefAddress);
            url = o.str();
            xrefNode = itr->second;
            //json_push_back(writeNode, json_new_a("xref", url.c_str()));
            writeNode->insert_pair("xref", json_new_a(allocator, url.c_str()));
        }
    }
    else
    {
        writeNode = createNode(parentNode, propertyName);
        writeNode->insert_pair(classField, json_new_a(allocator, value->getClassName().c_str()));
    }
    
    if (xrefNode == nullptr) {
        _nodes.push(writeNode);
        value->onSerialize(this);
        auto top = _nodes.top();
        if (top->type() == jc::Type::Object) {
            top->reverse();
        }
        _nodes.pop();
    }
}

void SerializerJson::writeList(const char* propertyName, size_t count)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);
    //if (count == 0)
    //    return;
    
    jc::JsonNode* node = _nodes.top();
    jc::JsonNode* list = (JsonNode*)allocator.allocate(sizeof(JsonNode));
    list->set_type(jc::Type::Array);
    //json_push_back(node, list);
    //json_set_name(list, propertyName);
    node->insert_pair(json_strdup(allocator, propertyName), list);

    _nodes.push(list);
    _nodesListCounts.push(count);
}

void SerializerJson::finishColloction() {
    //jc::JsonNode* parentNode = _nodes.top();
    //if (json_type(parentNode) == JSON_ARRAY || json_type(parentNode) == JSON_NODE)
    //{
    if (_type == Type::eWriter) {
        auto top = _nodes.top();
        if (top->type() == jc::Type::Object || top->type() == jc::Type::Array) {
            top->reverse();
        }
    }
    _nodes.pop();
    _nodesListCounts.pop();
    //}
}

void SerializerJson::writeIntArray(const char* propertyName, const int* data, size_t count)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);
    if (!data || count == 0)
        return;

    // "properyName" : [ 0, ... , count - 1 ]
    jc::JsonNode* node = _nodes.top();
    jc::JsonNode* array = (JsonNode*)allocator.allocate(sizeof(JsonNode));
    array->set_type(jc::Type::Array);

    for (size_t i = 0; i < count; i++)
    {
        auto value = (JsonNode*)allocator.allocate(sizeof(JsonNode));
        value->set_int(data[i]);
        array->insert(value);
    }
    array->reverse();

    node->insert_pair(json_strdup(allocator, propertyName), array);
}

void SerializerJson::writeFloatArray(const char* propertyName, const float* data, size_t count)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);
    if (!data || count == 0)
        return;
    
    // "properyName" : [ 0.0, ... , count - 1 ]
    jc::JsonNode* node = _nodes.top();
    jc::JsonNode* array = (JsonNode*)allocator.allocate(sizeof(JsonNode));
    array->set_type(jc::Type::Array);

    for (size_t i = 0; i < count; i++)
    {
        auto value = (JsonNode*)allocator.allocate(sizeof(JsonNode));
        value->set_float(data[i]);
        array->insert(value);
    }
    array->reverse();

    node->insert_pair(json_strdup(allocator, propertyName), array);
}

void SerializerJson::writeDFloatArray(const char* propertyName, const double* data, size_t count)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);
    if (!data || count == 0)
        return;

    // "properyName" : [ 0.0, ... , count - 1 ]
    jc::JsonNode* node = _nodes.top();
    jc::JsonNode* array = (JsonNode*)allocator.allocate(sizeof(JsonNode));
    array->set_type(jc::Type::Array);

    for (size_t i = 0; i < count; i++)
    {
        auto value = (JsonNode*)allocator.allocate(sizeof(JsonNode));
        value->set_float(data[i]);
        array->insert(value);
    }
    array->reverse();

    node->insert_pair(json_strdup(allocator, propertyName), array);
}

void SerializerJson::writeByteArray(const char* propertyName, const unsigned char* data, size_t count)
{
    GP_ASSERT(propertyName);
    GP_ASSERT(_type == Type::eWriter);
    if (!data || count == 0)
        return;
    
    // "properyName" : "base64_encode(data)"
    jc::JsonNode* node = _nodes.top();
    char* buffer = (char*)allocator.allocate(count * 2);
    base64_encode(data, count, buffer);

    jc::JsonNode* value = (JsonNode*)allocator.allocate(sizeof(JsonNode));
    value->set_str(buffer);
    node->insert_pair(json_strdup(allocator, propertyName), value);
}

int SerializerJson::readEnum(const char* propertyName, const char* enumName, int defaultValue)
{
    GP_ASSERT(enumName);
    
    std::string str;
    readString(propertyName, str, "");
    if (str.size() == 0) {
        return defaultValue;
    }
    
    return SerializerManager::getActivator()->enumParse(enumName, str.c_str());
}

bool SerializerJson::readBool(const char* propertyName, bool defaultValue)
{
    GP_ASSERT(_type == Type::eReader);
    
    jc::JsonNode* node = _nodes.top();
    jc::Value* property = readElement(propertyName);
    if (property)
    {
        if (!_isHiml && property->type() != jc::Type::Boolean)
            GP_ERROR("Invalid json bool for propertyName:%s", propertyName);
        return property->as_bool();
    }
    return defaultValue;
}

int SerializerJson::readInt(const char* propertyName, int defaultValue)
{
    GP_ASSERT(_type == Type::eReader);

    jc::JsonNode* node = _nodes.top();
    jc::Value* property = readElement(propertyName);
    if (property)
    {
        if (!_isHiml && property->type() != jc::Type::Integer)
            GP_ERROR("Invalid json int for propertyName:%s", propertyName);
        return property->as_int();
    }
    return defaultValue;
}

float SerializerJson::readFloat(const char* propertyName, float defaultValue)
{
    GP_ASSERT(_type == Type::eReader);

    jc::JsonNode* node = _nodes.top();
    jc::Value* property = readElement(propertyName);

    if (property)
    {
        if (!_isHiml && property->type() != jc::Type::Float && property->type() != jc::Type::Integer)
            GP_ERROR("Invalid json float for propertyName:%s", propertyName);
        return property->as_float();
    }
    return defaultValue;
}

Vector2 SerializerJson::readVector(const char* propertyName, const Vector2& defaultValue)
{
    GP_ASSERT(_type == Type::eReader);
    
    jc::JsonNode* node = _nodes.top();
    jc::Value* property = readElement(propertyName);
    if (_isHiml && property) {
        property = property->children();
    }
    if (property)
    {
        if (property->type() != jc::Type::Array || property->size() < 2)
            GP_ERROR("Invalid json array from Vector2 for propertyName:%s", propertyName);
        auto it = property->begin();
        Vector2 value;
        jc::Value* x = *it;
        value.x  = x->as_float();
        ++it;
        jc::Value* y = *it;
        value.y  = y->as_float();
        return value;
    }
    return defaultValue;
}

Vector3 SerializerJson::readVector(const char* propertyName, const Vector3& defaultValue)
{
    GP_ASSERT(_type == Type::eReader);

    jc::JsonNode* node = _nodes.top();
    jc::Value* property = readElement(propertyName);
    if (_isHiml && property) {
        property = property->children();
    }
    if (property)
    {
        if (property->type() != jc::Type::Array || property->size() < 3)
            GP_ERROR("Invalid json array from Vector3 for propertyName:%s", propertyName);
        auto it = property->begin();
        Vector3 value;
        jc::Value* x = *it;
        value.x = x->as_float();
        ++it;
        jc::Value* y = *it;
        value.y = y->as_float();
        ++it;
        jc::Value* z = *it;
        value.z = z->as_float();
        return value;
    }
    return defaultValue;
}

Vector4 SerializerJson::readVector(const char* propertyName, const Vector4& defaultValue)
{
    GP_ASSERT(_type == Type::eReader);

    jc::JsonNode* node = _nodes.top();
    jc::Value* property = readElement(propertyName);
    if (_isHiml && property) {
        property = property->children();
    }
    if (property)
    {
        if (property->type() != jc::Type::Array || property->size() < 4)
            GP_ERROR("Invalid json array from Vector4 for propertyName:%s", propertyName);
        auto it = property->begin();
        Vector4 value;
        jc::Value* x = *it;
        value.x = x->as_float();
        ++it;
        jc::Value* y = *it;
        value.y = y->as_float();
        ++it;
        jc::Value* z = *it;
        value.z = z->as_float();
        ++it;
        jc::Value* w = *it;
        value.w = w->as_float();
        return value;
    }
    return defaultValue;
}

Vector3 SerializerJson::readColor(const char* propertyName, const Vector3& defaultValue)
{
    GP_ASSERT(_type == Type::eReader);
    
    jc::JsonNode* node = _nodes.top();
    jc::Value* property = readElement(propertyName);
    if (_isHiml && property) {
        property = property->children();
    }
    if (property)
    {
        if (property->type() != jc::Type::String)
            GP_ERROR("Invalid json string from color for propertyName:%s", propertyName);
        const char* str = property->as_str();
        Vector3 value = Vector3::fromColorString(str);
        //json_free(str);
        return value;
    }
    return defaultValue;
}

Vector4 SerializerJson::readColor(const char* propertyName, const Vector4& defaultValue)
{
    GP_ASSERT(_type == Type::eReader);
    
    jc::JsonNode* node = _nodes.top();
    jc::Value* property = readElement(propertyName);
    if (_isHiml && property) {
        property = property->children();
    }
    if (property)
    {
        if (property->type() != jc::Type::String)
            GP_ERROR("Invalid json string from color for propertyName:%s", propertyName);
        const char* str = property->as_str();
        Vector4 value = Vector4::fromColorString(str);
        //json_free(str);
        return value;
    }
    return defaultValue;
}

Matrix SerializerJson::readMatrix(const char* propertyName, const Matrix& defaultValue)
{
    GP_ASSERT(_type == Type::eReader);

    jc::JsonNode* node = _nodes.top();
    jc::Value* property = readElement(propertyName);
    if (_isHiml && property) {
        property = property->children();
    }
    if (property)
    {
        if (property->type() != jc::Type::Array || property->size() < 16)
            GP_ERROR("Invalid json array from Matrix for propertyName:%s", propertyName);

        auto it = property->begin();
        Matrix value;

        for (int i = 0; i < 16; ++i) {
            value.m[i] = it->as_float();
            ++it;
        }

        return value;
    }
    return defaultValue;
}

jc::Value* SerializerJson::readElement(const char* propertyName) {
    jc::Value* property_ = nullptr;
    jc::JsonNode* node = _nodes.top();

    if (node->type() == jc::Type::Array)
    {
        size_t arraySize = node->size();
        int i = 0;
        for (auto it = node->begin(); it != node->end(); ++it) {
            if (i == (uint32_t)arraySize - (uint32_t)_nodesListCounts.top()) {
                property_ = *it;
                break;
            }
            ++i;
        }
        //property = json_at(node, (uint32_t)arraySize - (uint32_t)_nodesListCounts.top());
        _nodesListCounts.top() -= 1;
    }
    else if (node->type() == jc::Type::Object)
    {
        if (!propertyName) return nullptr;
        property_ = node->get(propertyName);
    }
    return property_;
}

void SerializerJson::readString(const char* propertyName, std::string& value, const char* defaultValue)
{
    GP_ASSERT(_type == Type::eReader);

    jc::JsonNode* node = _nodes.top();
    jc::Value* property = readElement(propertyName);
   
    if (property)
    {
        if (!_isHiml && property->type() != jc::Type::String)
            GP_ERROR("Invalid json string for propertyName:%s", propertyName);

        /*json_char* str = json_as_string(property);
        value.clear();
        value.resize(strlen(str));
        std::strcpy(&value[0], str);
        json_free(str);*/
        value = property->as_str();
    }
    else
    {
        value.clear();
        value.resize(strlen(defaultValue));
        std::strcpy(&value[0], defaultValue);
    }
}

UPtr<Serializable> SerializerJson::readObject(const char* propertyName)
{
    GP_ASSERT(_type == Type::eReader);
    
    jc::JsonNode* parentNode = _nodes.top();
    jc::Value* readNode = readElement(propertyName);

    //read root
    if (!readNode && !propertyName && parentNode->type() == jc::Type::Object) {
        readNode = parentNode;
    }

    if (readNode == nullptr)
        return UPtr<Serializable>();
    
    const char* classField = "class";
    if (_isHiml) {
        classField = "_type";
    }
    jc::Value* classProperty = readNode->get(classField);
    const char* className = classProperty->as_str();
    
    // Look for xref's
    unsigned long xrefAddress = 0L;
    jc::Value* xrefProperty = readNode->get("xref");
    if (xrefProperty)
    {
        //json_char* str = json_as_string(xrefProperty);
        std::string url = xrefProperty->as_str();
        //json_free(str);
        std::string at = "@";
        if (url.compare(0, at.length(), at) != 0)
        {
            // no @ sign. This is xref'ed by others
            xrefAddress = std::strtol(url.c_str(), nullptr, 10);
        }
        else
        {
            // This needs to lookup the node from the xref address. So save it for lookup
            std::string addressStr = url.substr(1, url.length());
            xrefAddress = std::strtol(addressStr.c_str(), nullptr, 10);
         
            std::map<unsigned long, Serializable*>::const_iterator itr = _xrefsRead.find(xrefAddress);
            if (itr != _xrefsRead.end())
            {
                Serializable* ref = itr->second;
                Refable* refable = dynamic_cast<Refable*>(ref);
                if (refable) refable->addRef();
                return UPtr<Serializable>(ref);
            }
            else
            {
                GP_WARN("Unresolved xref:%u for class:%s", xrefAddress, className);
                //json_free(className);
                return UPtr<Serializable>();
            }
        }
    }
    
    Serializable *value = (SerializerManager::getActivator()->createObject(className));
    if (value == nullptr)
    {
        GP_WARN("Failed to deserialize json object:%s for class:", className);
        //json_free(className);
        return UPtr<Serializable>();
    }
    //json_free(className);

    _nodes.push((JsonNode*)readNode);
    value->onDeserialize(this);
    _nodes.pop();
    
    if (xrefAddress)
        _xrefsRead[xrefAddress] = value;

    return UPtr<Serializable>(value);
}

void SerializerJson::readMap(const char* propertyName, std::vector<std::string> &keys)
{
    GP_ASSERT(_type == Type::eReader);

    jc::JsonNode* node = _nodes.top();
    jc::Value* list = readElement(propertyName);
    //size_t count = list->size();
    //if (count > 0)
    //{
        int i = 0;
        for (auto it = list->begin(); it != list->end(); ++it) {
            jc::JsonNode* item = (jc::JsonNode*)(*it);
            const char* name = item->name;
            keys.push_back(name);
            ++i;
        }
        _nodes.push((JsonNode*)list);
        _nodesListCounts.push(i);
    //}
}

size_t SerializerJson::readList(const char* propertyName)
{
    GP_ASSERT(_type == Type::eReader);
    
    jc::JsonNode* node = _nodes.top();
    jc::Value* list = readElement(propertyName);

    size_t count = 0;
    if (list) {
        count = list->size();
    }
    //if (count > 0)
    //{
    _nodes.push((JsonNode*)list);
    _nodesListCounts.push(count);
    //}
    return count;
}

size_t SerializerJson::readIntArray(const char* propertyName, int** data)
{
    GP_ASSERT(_type == Type::eReader);
    
    jc::JsonNode* node = _nodes.top();
    size_t count = 0;
    jc::Value* property = readElement(propertyName);
    if (property)
    {
        if (property->type() != jc::Type::Array )
            GP_ERROR("Invalid json array for propertyName:%s", propertyName);
        
        count = property->size();
        int* buffer = nullptr;
        if (*data == nullptr)
        {
            buffer = new int[count];
        }
        else
        {
            buffer = *data;
        }
        int i = 0;
        for (auto it = property->begin(); it != property->end(); ++it)
        {
            buffer[i] = it->as_int();
            ++i;
        }
        *data = buffer;
    }
    return count;
}

size_t SerializerJson::readFloatArray(const char* propertyName, float** data)
{
    GP_ASSERT(_type == Type::eReader);
    
    jc::JsonNode* node = _nodes.top();
    size_t count = 0;
    jc::Value* property = readElement(propertyName);
    if (property)
    {
        if (property->type() != jc::Type::Array)
            GP_ERROR("Invalid json array for propertyName:%s", propertyName);
        
        count = property->size();
        float* buffer = nullptr;
        if (*data == nullptr)
        {
            buffer = new float[count];
        }
        else
        {
            buffer = *data;
        }
        
        int i = 0;
        for (auto it = property->begin(); it != property->end(); ++it)
        {
            buffer[i] = it->as_float();
            ++i;
        }
        *data = buffer;
    }
    return count;
}

size_t SerializerJson::readDFloatArray(const char* propertyName, double** data)
{
    GP_ASSERT(_type == Type::eReader);

    jc::JsonNode* node = _nodes.top();
    size_t count = 0;
    jc::Value* property = readElement(propertyName);
    if (property)
    {
        if (property->type() != jc::Type::Array)
            GP_ERROR("Invalid json array for propertyName:%s", propertyName);

        count = property->size();
        double* buffer = nullptr;
        if (*data == nullptr)
        {
            buffer = new double[count];
        }
        else
        {
            buffer = *data;
        }

        int i = 0;
        for (auto it = property->begin(); it != property->end(); ++it)
        {
            buffer[i] = it->as_float();
            ++i;
        }
        *data = buffer;
    }
    return count;
}
    
size_t SerializerJson::readByteArray(const char* propertyName, unsigned char** data)
{
    GP_ASSERT(_type == Type::eReader);
    
    unsigned long size = 0L;
    jc::JsonNode* node = _nodes.top();
    jc::Value* property = readElement(propertyName);
    if (property)
    {
        if (property->type() != jc::Type::String)
            GP_ERROR("Invalid json base64 string for propertyName:%s", propertyName);
        
        const char* str = property->as_str();
        int inlen = strlen(str);
        unsigned char* out = new unsigned char[inlen];
        size = base64_decode(str, inlen, out);
        
        if (*data == nullptr)
        {
            *data = (unsigned char*)out;
        }
        else
        {
            memcpy(*data, out, size);
            //json_free(decoded);
            delete[] out;
        }
        //json_free(str);
    }
    return (size_t)size;
}

}
