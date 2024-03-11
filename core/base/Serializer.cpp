#include "Base.h"
#include "Serializer.h"
#include "SerializerBinary.h"
#include "SerializerJson.h"
#include "FileSystem.h"

namespace mgp
{

Serializer::Serializer(Type type, Stream* stream, uint32_t versionMajor, uint32_t versionMinor) : 
    _type(type),
    //_path(path),
    _stream(stream)
{
    _version[0] = versionMajor;
    _version[1] = versionMinor;
}

Serializer::~Serializer()
{
    _stream->close();
    SAFE_DELETE(_stream);
}

UPtr<Serializer> Serializer::createReader(const std::string& path)
{
    Stream* stream = FileSystem::open(path.c_str()).take();
    if (!stream)
        return UPtr<Serializer>();

    UPtr<Serializer> serializer = SerializerBinary::create(stream);
    if (!serializer.get())
    {
        stream->rewind();
        serializer = SerializerJson::create(stream);
    }
    return serializer;
}

UPtr<Serializer> Serializer::createReader(Stream* stream) {
    UPtr<Serializer> serializer = SerializerBinary::create(stream);
    if (!serializer.get())
    {
        stream->rewind();
        serializer = SerializerJson::create(stream);
    }
    return serializer;
}

//std::string Serializer::getPath() const
//{
//    return _path;
//}
   
uint32_t Serializer::getVersionMajor() const
{
    return (uint32_t)_version[0];
}

uint32_t Serializer::getVersionMinor() const
{
    return (uint32_t)_version[1];
}

}
