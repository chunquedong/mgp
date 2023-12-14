#include "base/Base.h"
#include "VertexFormat.h"

namespace mgp
{

VertexFormat::VertexFormat() : _vertexSize(0) {

}

VertexFormat::VertexFormat(const Element* elements, unsigned int elementCount)
    : _vertexSize(0)
{
    //GP_ASSERT(elements);

    // Copy elements and compute vertex size
    for (unsigned int i = 0; i < elementCount; ++i)
    {
        // Copy element
        //Element element;
        //memcpy(&element, &elements[i], sizeof(Element));
        _elements.push_back(elements[i]);
    }

    update();
}

VertexFormat::~VertexFormat()
{
}

void VertexFormat::update() {
    _vertexSize = 0;
    for (unsigned int i = 0; i < _elements.size(); ++i)
    {
        if (_elements[i].offset == -1) {
            _elements[i].offset = _vertexSize;
        }
        else {
            continue;
        }
        int byteSize = 0;
        switch (_elements[i].dataType) {
        case FLOAT32:
            byteSize = _elements[i].size * sizeof(float);
            break;
        case INT32:
            byteSize = _elements[i].size * 4;
            break;
        case INT16:
            byteSize = _elements[i].size * 2;
            break;
        case INT8:
            byteSize = _elements[i].size * 1;
            break;
        default:
            GP_ASSERT(0);
        }
        _vertexSize += byteSize;
    }

    for (unsigned int i = 0; i < _elements.size(); ++i)
    {
        if (_elements[i].stride == -1) {
            _elements[i].stride = _vertexSize;
        }
    }
}

const VertexFormat::Element* VertexFormat::getPositionElement() const {
    for (size_t i = 0, count = _elements.size(); i < count; ++i)
    {
        if (_elements[i].usage == POSITION) {
            return &_elements[i];
        }
    }
    return NULL;
}

void VertexFormat::addElement(const Element& element) {
    _elements.push_back(element);
}

VertexFormat::Element& VertexFormat::getElement(unsigned int index)
{
    GP_ASSERT(index < _elements.size());
    return _elements[index];
}

const VertexFormat::Element& VertexFormat::getElement(unsigned int index) const
{
    GP_ASSERT(index < _elements.size());
    return _elements[index];
}

unsigned int VertexFormat::getElementCount() const
{
    return (unsigned int)_elements.size();
}

unsigned int VertexFormat::getVertexSize() const
{
    return _vertexSize;
}

bool VertexFormat::operator == (const VertexFormat& f) const
{
    if (_elements.size() != f._elements.size())
        return false;

    for (size_t i = 0, count = _elements.size(); i < count; ++i)
    {
        if (_elements[i] != f._elements[i])
            return false;
    }

    return true;
}


bool VertexFormat::operator != (const VertexFormat& f) const
{
    return !(*this == f);
}

VertexFormat::Element::Element() :
    usage(POSITION), size(0)
{
}

VertexFormat::Element::Element(Usage usage, unsigned int size) :
    usage(usage), size(size)
{
}

VertexFormat::Element::Element(const std::string &name, unsigned int size) : usage(CUSTEM), name(name), size(size) {
}

bool VertexFormat::Element::operator == (const VertexFormat::Element& e) const
{
    return (size == e.size && usage == e.usage);
}

bool VertexFormat::Element::operator != (const VertexFormat::Element& e) const
{
    return !(*this == e);
}

const char* VertexFormat::toString(Usage usage)
{
    switch (usage)
    {
    case POSITION:
        return "POSITION";
    case NORMAL:
        return "NORMAL";
    case COLOR:
        return "COLOR";
    case TANGENT:
        return "TANGENT";
    case BINORMAL:
        return "BINORMAL";
    case BLENDWEIGHTS:
        return "BLENDWEIGHTS";
    case BLENDINDICES:
        return "BLENDINDICES";
    case TEXCOORD0:
        return "TEXCOORD0";
    case TEXCOORD1:
        return "TEXCOORD1";
    case TEXCOORD2:
        return "TEXCOORD2";
    case TEXCOORD3:
        return "TEXCOORD3";
    case TEXCOORD4:
        return "TEXCOORD4";
    case TEXCOORD5:
        return "TEXCOORD5";
    case TEXCOORD6:
        return "TEXCOORD6";
    case TEXCOORD7:
        return "TEXCOORD7";
    default:
        return "UNKNOWN";
    }
}

void VertexFormat::Element::write(Stream* file) const  {
    file->writeUInt16(usage);
    file->writeUInt16(size);
    file->writeInt16(offset);
    file->writeInt16(stride);
    file->writeInt16(dataType);
    file->writeStr(name);
}
bool VertexFormat::Element::read(Stream* file) {
    usage = (VertexFormat::Usage)file->readUInt16();
    size = file->readUInt16();
    offset = file->readInt16();
    stride = file->readInt16();
    dataType = (VertexFormat::DataType)file->readInt16();
    name = file->readStr();
    return true;
}

}
