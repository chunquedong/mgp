#include "base/Base.h"
#include "Mesh.h"
#include "material/ShaderProgram.h"
#include "Model.h"
#include "material/Material.h"
#include "scene/Renderer.h"
#include "scene/Drawable.h"
#include <float.h>

namespace mgp
{
RenderBuffer::RenderBuffer() {
}
RenderBuffer::~RenderBuffer() {
    if (_data) {
        free(_data);
        _data = NULL;
    }
    if (this->_bufferHandle)
    {
        Renderer::cur()->deleteBuffer(_bufferHandle);
        _bufferHandle = 0;
    }
}

void RenderBuffer::setCapacity(int capacity) {
    if (capacity != -1 && _dataCapacity != capacity) {
        _dataCapacity = capacity;
        char* vertexData = (char*)realloc(_data, _dataCapacity);
        if (vertexData != _data) {
            _data = vertexData;
            _pointerDirty = true;
        }
        _contentDirty = true;
    }
}

void RenderBuffer::resize(int size) {
    if (_dataSize == size) return;
    if (_dataCapacity < size) {
        setCapacity(size+_growSize);
    }
    _dataSize = size;
    _contentDirty = true;
}

void RenderBuffer::setData(char* data, int size, bool copy) {
    if (copy) {
        if (!_data) _data = (char*)malloc(size);
        memcpy(_data, data, size);
    }
    else {
        if (!_data) free(_data);
        _data = data;
    }
    _dataSize = size;
    _dataCapacity = size;
    _contentDirty = true;
    _pointerDirty = true;
}

void RenderBuffer::updateData(char* src, int dst_offset, int size) {
    GP_ASSERT(size + dst_offset <= _dataCapacity);
    memcpy(_data + dst_offset, src, size);
    _contentDirty = true;
}

int RenderBuffer::addData(char* data, int size) {
    int offset = _dataSize;
    if (_dataSize + size > _dataCapacity) {
        int newSize = _dataSize + size + _growSize;
        setCapacity(newSize);
    }
    memcpy(_data+ _dataSize, data, size);
    _dataSize += size;
    _contentDirty = true;
    return offset;
}

Mesh::Mesh(const VertexFormat& vertexFormat) : _vertexFormat(vertexFormat)
{
}

Mesh::~Mesh()
{
    _vertexCount = 0;
    if (_vertexAttributeArray) {
        SAFE_RELEASE(_vertexAttributeArray);
        _vertexAttributeArray = NULL;
    }
    _parts.clear();
}

UPtr<Mesh> Mesh::create(const VertexFormat& vertexFormat, IndexFormat indexFormat, bool dynamic) {
    Mesh* mesh = new Mesh(vertexFormat);
    mesh->_dynamic = dynamic;
    mesh->_indexFormat = indexFormat;
    return UPtr<Mesh>(mesh);
}

UPtr<Mesh> Mesh::createMesh(const VertexFormat& vertexFormat, unsigned int vertexCount, IndexFormat indexFormat, bool dynamic)
{
    Mesh* mesh = new Mesh(vertexFormat);
    mesh->_vertexCount = vertexCount;
    mesh->_dynamic = dynamic;
    mesh->_indexFormat = indexFormat;
    return UPtr<Mesh>(mesh);
}

const char* Mesh::getUrl() const
{
    return _url.c_str();
}

VertexFormat& Mesh::getVertexFormat()
{
    return _vertexFormat;
}
const VertexFormat& Mesh::getVertexFormat() const
{
    return _vertexFormat;
}

unsigned int Mesh::getVertexCount() const
{
    return _vertexCount;
}

RenderBuffer* Mesh::getVertexBuffer()
{
    return &_vertexBuffer;
}

bool Mesh::isDynamic() const
{
    return _dynamic;
}

Mesh::PrimitiveType Mesh::getPrimitiveType() const
{
    return _primitiveType;
}

void Mesh::setPrimitiveType(PrimitiveType type)
{
    _primitiveType = type;
}
/*
void* Mesh::mapVertexBuffer()
{
    GL_ASSERT( glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer) );

    return (void*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

bool Mesh::unmapVertexBuffer()
{
    return glUnmapBuffer(GL_ARRAY_BUFFER);
}
*/

unsigned int Mesh::getIndexCount() const
{
    unsigned int count = 0;
    for (int i = 0; i < _parts.size(); ++i) {
        const MeshPart* p = &_parts[i];
        count += p->_indexCount;
    }
    return count;
}

bool Mesh::isIndexed() const {
    return _parts.size() > 0;
}

//
//void Mesh::setIndexCount(int indexCount) {
//    _indexCount = indexCount;
//}

Mesh::IndexFormat Mesh::getIndexFormat() const
{
    return _indexFormat;
}

RenderBuffer* Mesh::getIndexBuffer()
{
    return &_indexBuffer;
}

Mesh::MeshPart* Mesh::addPart(PrimitiveType primitiveType, unsigned int indexCount, unsigned int bufferOffset)
{
    MeshPart part;
    part._primitiveType = primitiveType;
    part._indexCount = indexCount;
    part._bufferOffset = bufferOffset;
    _parts.emplace_back(part);
    return &_parts[_parts.size()-1];
}

unsigned int Mesh::getPartCount()
{
    return _parts.size();
}

Mesh::MeshPart* Mesh::getPart(unsigned int index)
{
    GP_ASSERT(index < _parts.size());
    return &_parts[index];
}

const BoundingBox& Mesh::getBoundingBox()
{
    if (_boundingBox.isEmpty()) {
        ((Mesh*)this)->computeBounds();
    }
    return _boundingBox;
}

void Mesh::setBoundingBox(const BoundingBox& box)
{
    _boundingBox = box;
}

const BoundingSphere& Mesh::getBoundingSphere()
{
    if (_boundingSphere.isEmpty()) {
        ((Mesh*)this)->computeBounds();
    }
    return _boundingSphere;
}

void Mesh::setBoundingSphere(const BoundingSphere& sphere)
{
    _boundingSphere = sphere;
}

void Mesh::write(Stream* file) {
    
    // vertex formats
    file->writeUInt8((unsigned int)_vertexFormat.getElementCount());
    for (int i=0; i<_vertexFormat.getElementCount(); ++i)
    {
        const VertexFormat::Element& element = _vertexFormat.getElement(i);
        element.write(file);
    }

    //file->writeStr(_url);
    file->writeUInt8(_primitiveType);
    file->writeUInt8(_dynamic);

    // vertices
    file->writeUInt32(_vertexCount);
    file->writeUInt32(_vertexBuffer._dataSize);
    file->write((const char*)_vertexBuffer._data, _vertexBuffer._dataSize);

    // write indices buffer
    file->writeUInt16(_indexFormat);
    file->writeUInt32(_indexBuffer._dataSize);
    file->write(_indexBuffer._data, _indexBuffer._dataSize);

    // parts
    file->writeInt16(_parts.size());
    for (int i = 0; i < _parts.size(); ++i) {
        MeshPart* p = &_parts[i];
        file->writeUInt8(p->_primitiveType);
        file->writeUInt32(p->_bufferOffset);
        file->writeUInt32(p->_indexCount);
    }

    getBoundingSphere();

    // Write bounds
    file->writeFloat(_boundingBox.min.x);
    file->writeFloat(_boundingBox.min.y);
    file->writeFloat(_boundingBox.min.z);

    file->writeFloat(_boundingBox.max.x);
    file->writeFloat(_boundingBox.max.y);
    file->writeFloat(_boundingBox.max.z);

    file->writeFloat(_boundingSphere.center.x);
    file->writeFloat(_boundingSphere.center.y);
    file->writeFloat(_boundingSphere.center.z);
    file->writeFloat(_boundingSphere.radius);

}

bool Mesh::read(Stream* file) {
    int elemCount = file->readUInt8();
    std::vector<VertexFormat::Element> elems;
    elems.resize(elemCount);
    for (int i = 0; i < elemCount; ++i) {
        elems[i].read(file);
    }

    VertexFormat format(&elems[0], elemCount);

    Mesh* mesh = this;
    mesh->_vertexFormat = format;
    //mesh->_url = file->readStr();
    mesh->_primitiveType = (Mesh::PrimitiveType)file->readUInt8();
    mesh->_dynamic = file->readUInt8();

    mesh->_vertexCount = file->readUInt32();
    int bufSize = file->readUInt32();
    void *vertexData = malloc(bufSize);
    file->read((char*)vertexData, bufSize);
    mesh->_vertexBuffer.setData((char*)vertexData, bufSize, false);

    //mesh->_vertexDataDirty = true;

    mesh->_indexFormat = (IndexFormat)file->readUInt16();
    int ibufsize = file->readUInt32();
    void* indexData = malloc(ibufsize);
    file->read((char*)indexData, ibufsize);
    mesh->_indexBuffer.setData((char*)indexData, ibufsize, false);

    int _partCount = file->readInt16();
    mesh->_parts.reserve(_partCount);
    for (int i = 0; i < _partCount; ++i) {
        MeshPart p;
        p._primitiveType = (Mesh::PrimitiveType)file->readUInt8();
        p._bufferOffset = file->readUInt32();
        p._indexCount = file->readUInt32();
        mesh->_parts.emplace_back(p);
    }

    mesh->_boundingBox.min.x = file->readFloat();
    mesh->_boundingBox.min.y = file->readFloat();
    mesh->_boundingBox.min.z = file->readFloat();
    mesh->_boundingBox.max.x = file->readFloat();
    mesh->_boundingBox.max.y = file->readFloat();
    mesh->_boundingBox.max.z = file->readFloat();

    mesh->_boundingSphere.center.x = file->readFloat();
    mesh->_boundingSphere.center.y = file->readFloat();
    mesh->_boundingSphere.center.z = file->readFloat();
    mesh->_boundingSphere.radius = file->readFloat();

    return true;
}


void Mesh::computeBounds()
{
    // If we have a Model with a MeshSkin associated with it,
    // compute the bounds from the skin - otherwise compute
    // it from the local mesh data.
    /*if (model && model->getSkin())
    {
        model->getSkin()->computeBounds();
        return;
    }*/

    //LOG(2, "Computing bounds for mesh: %s\n", getId().c_str());

    _boundingBox.min.x = _boundingBox.min.y = _boundingBox.min.z = FLT_MAX;
    _boundingBox.max.x = _boundingBox.max.y = _boundingBox.max.z = -FLT_MAX;
    _boundingSphere.center.x = _boundingSphere.center.y = _boundingSphere.center.z = 0.0f;
    _boundingSphere.radius = 0.0f;

    const VertexFormat::Element* positionElement = _vertexFormat.getPositionElement();    

    for (int i = 0; i < _vertexCount; ++i)
    {
        float *p = (float*)((char*)_vertexBuffer._data + (i * positionElement->stride) + positionElement->offset);
        float x = p[0];
        float y = p[1];
        float z = p[2];

        // Update min/max for this vertex
        if (x < _boundingBox.min.x)
            _boundingBox.min.x = x;
        if (y < _boundingBox.min.y)
            _boundingBox.min.y = y;
        if (z < _boundingBox.min.z)
            _boundingBox.min.z = z;
        if (x > _boundingBox.max.x)
            _boundingBox.max.x = x;
        if (y > _boundingBox.max.y)
            _boundingBox.max.y = y;
        if (z > _boundingBox.max.z)
            _boundingBox.max.z = z;
    }

    // Compute center point
    _boundingSphere.center = _boundingBox.getCenter();

    // Compute radius by looping through all points again and finding the max
    // distance between the center point and each vertex position
    for (int i = 0; i < _vertexCount; ++i)
    {
        float* p = (float*)((char*)_vertexBuffer._data + (i * positionElement->stride) + positionElement->offset);
        float x = p[0];
        float y = p[1];
        float z = p[2];
    
        float d = _boundingSphere.center.distanceSquared(Vector3(x, y, z));
        if (d > _boundingSphere.radius)
        {
            _boundingSphere.radius = d;
        }
    }

    // Convert squared distance to distance for radius
    _boundingSphere.radius = sqrt(_boundingSphere.radius);
}

unsigned int Mesh::draw(RenderInfo* view, Drawable* drawable, Material* _material, UPtr<Material>* _partMaterials, int partMaterialCount)
{
    Mesh* _mesh = this;
    GP_ASSERT(_mesh);

    if (_vertexBuffer._bufferHandle == 0) {
        _vertexBuffer._bufferHandle = Renderer::cur()->createBuffer(0);
    }
    if (_vertexBuffer._contentDirty) {
        GP_ASSERT(_vertexBuffer._dataSize >= _vertexCount * _vertexFormat.getVertexSize());
        Renderer::cur()->setBufferData(_vertexBuffer._bufferHandle, 0, 0, (const char*)_vertexBuffer._data, _vertexBuffer._dataSize, _dynamic);
        _vertexBuffer._contentDirty = false;
    }

    if (_mesh->_parts.size() > 0) {
        if (_indexBuffer._bufferHandle == 0) {
            _indexBuffer._bufferHandle = Renderer::cur()->createBuffer(1);
        }
        if (_indexBuffer._contentDirty) {
            GP_ASSERT(_indexBuffer._dataSize >= _mesh->_parts[0]._indexCount * getIndexSize());
            Renderer::cur()->setBufferData(_indexBuffer._bufferHandle, 1, 0, (const char*)_indexBuffer._data, _indexBuffer._dataSize, _dynamic);
            _indexBuffer._contentDirty = false;
        }
    }

    Mesh* mesh = _mesh;
    if (!mesh->_vertexAttributeArray) {
        BufferHandle indexBufferObject = 0;
        if (mesh->_parts.size() > 0) indexBufferObject = _indexBuffer._bufferHandle;
        mesh->_vertexAttributeArray = VertexAttributeBinding::create(mesh->_vertexBuffer._bufferHandle, mesh->getVertexFormat(), NULL, indexBufferObject).take();
        _vertexBuffer._pointerDirty = false;
        _indexBuffer._pointerDirty = false;
    }
    else if (_vertexBuffer._pointerDirty || _indexBuffer._pointerDirty) {
        mesh->_vertexAttributeArray->update();
        _vertexBuffer._pointerDirty = false;
        _indexBuffer._pointerDirty = false;
    }

    unsigned int partCount = mesh->getPartCount();
    if (partCount == 0)
    {
        DrawCall drawCall;
        drawCall._drawable = drawable;
        for (Material* material = _material; material != NULL; material = material->getNextPass())
        {
            drawCall._vertexAttributeArray = mesh->_vertexAttributeArray;
            drawCall._material = material;
            drawCall._primitiveType = mesh->getPrimitiveType();
            drawCall._vertexCount = mesh->getVertexCount();

            /*if (isIndexed()) {
                drawCall._indexFormat = getIndexFormat();
                drawCall._indexBuffer = _indexBuffer._bufferHandle;
                drawCall._indexCount = getIndexCount();
            }*/

            if (view) {
                view->draw(&drawCall);
                if (view->wireframe || view->isDepthPass) break;
            }
            else {
                drawCall._material->setParams(view, drawable);
                Renderer::cur()->draw(&drawCall);
            }
        }
        return 1;
    }

    for (unsigned int i = 0; i < partCount; ++i)
    {
        MeshPart* part = mesh->getPart(i);
        GP_ASSERT(part);

        // Get the material for this mesh part.
        Material* material = NULL;
        if (i < partCount && partMaterialCount > 0) material = _partMaterials[i].get();
        else if (_material) material = _material;

        GP_ASSERT(material);
        DrawCall drawCall;
        drawCall._drawable = drawable;
        for (; material != NULL; material = material->getNextPass())
        {
            drawCall._vertexAttributeArray = mesh->_vertexAttributeArray;
            drawCall._material = material;
            drawCall._indexFormat = getIndexFormat();
            drawCall._indexBuffer = _indexBuffer._bufferHandle;
            drawCall._primitiveType = part->_primitiveType;
            drawCall._vertexCount = mesh->getVertexCount();
            drawCall._indexCount = part->_indexCount;
            drawCall._indexBufferOffset = part->_bufferOffset;
            //drawCall._instanceVbo = view->_instanceVbo;
            //drawCall._instanceCount = view->_instanceCount;

            if (view) {
                view->draw(&drawCall);
                if (view->wireframe || view->isDepthPass) break;
            }
            else {
                drawCall._material->setParams(view, drawable);
                Renderer::cur()->draw(&drawCall);
            }
        }
    }

    return partCount;
}

template<typename T> bool Mesh::raycastPart(RayQuery& query, MeshPart* part, int partIndex) {
    int minPart = -1;
    Vector3 curTarget;

    T* indices = (T*)((char*)this->_indexBuffer._data+part->_bufferOffset);
    char* verteix = (char*)_vertexBuffer._data;
    const VertexFormat::Element* positionElement = _vertexFormat.getPositionElement();
    int count = part->_indexCount;

    Vector3 a;
    Vector3 b;
    Vector3 c;
    for (int j = 0; j < count; j += 3) {
        T ia = indices[j];
        T ib = indices[j + 1];
        T ic = indices[j + 2];
        float* af = (float*)(verteix + (positionElement->stride * ia) + positionElement->offset);
        float* bf = (float*)(verteix + (positionElement->stride * ib) + positionElement->offset);
        float* cf = (float*)(verteix + (positionElement->stride * ic) + positionElement->offset);
        //float to double
        a.x = af[0]; a.y = af[1]; a.z = af[2];
        b.x = bf[0]; b.y = bf[1]; b.z = bf[2];
        c.x = cf[0]; c.y = cf[1]; c.z = cf[2];
        double dis = query.ray.intersectTriangle(a, b, c, query.backfaceCulling, &curTarget);
        if (dis != Ray::INTERSECTS_NONE) {
            if (dis < query.minDistance) {
                query.minDistance = dis;
                minPart = j;
                query.target = curTarget;
            }
        }
    }
    if (minPart != -1) {
        query.path.push_back(partIndex);
        query.path.push_back(minPart);
        return true;
    }
    return false;
}

bool Mesh::doRaycast(RayQuery& query) {
    bool res = false;

    unsigned int partCount = getPartCount();
    for (unsigned int i = 0; i < partCount; ++i)
    {
        MeshPart* part = getPart(i);
        GP_ASSERT(part);
        if (part->_primitiveType == Mesh::TRIANGLES) {
            if (getIndexFormat() == Mesh::INDEX16) {
                if (raycastPart<uint16_t>(query, part, i)) res = true;
            }
            else if (getIndexFormat() == Mesh::INDEX32) {
                if (raycastPart<uint32_t>(query, part, i)) res = true;
            }
        }
    }

    return res;
}

unsigned int Mesh::getIndexSize() const {
    unsigned int indexSize = 0;
    switch (_indexFormat)
    {
    //case Mesh::INDEX8:
    //    indexSize = 1;
        break;
    case Mesh::INDEX16:
        indexSize = 2;
        break;
    case Mesh::INDEX32:
        indexSize = 4;
        break;
    default:
        GP_ERROR("Unsupported index format (%d).", this->getIndexFormat());
        return 0;
    }
    return indexSize;
}


void Mesh::merge(const void* vertices, unsigned int vertexCount, const void* indices, unsigned int indexCount) {
    GP_ASSERT(vertices);
    
    unsigned int newVertexCount = _vertexCount + vertexCount;
    unsigned int vertexSize = _vertexFormat.getVertexSize();
    unsigned int vBytes = vertexCount * vertexSize;
    _vertexBuffer.addData((char*)vertices, vBytes);

    // Copy index data.
    if (indices)
    {
        MeshPart* part;
        if (_parts.size() == 0) {
            part = addPart(this->_primitiveType, 0);
        }
        else {
            part = &_parts[0];
        }

        unsigned int newIndexCount = part->_indexCount + indexCount;
        unsigned int indexSize = getIndexSize();
        
        if (_vertexCount == 0)
        {
            // Simply copy values directly into the start of the index array.
            _indexBuffer.addData((char*)indices, indexCount * indexSize);
        }
        else
        {
            if (_primitiveType == Mesh::TRIANGLE_STRIP && _vertexCount > 0) {
                newIndexCount += 2; // need an extra 2 indices for connecting strips with degenerate triangles
            }
            _indexBuffer.resize(newIndexCount * indexSize);

            if (_indexFormat == INDEX16) {
                uint16_t* _indicesPtr = (uint16_t*)(_indexBuffer._data + (part->_indexCount * indexSize));
                if (_primitiveType == Mesh::TRIANGLE_STRIP)
                {
                    // Create a degenerate triangle to connect separate triangle strips
                    // by duplicating the previous and next vertices.
                    _indicesPtr[0] = *(_indicesPtr - 1);
                    _indicesPtr[1] = _vertexCount;
                    _indicesPtr += 2;
                }

                // Loop through all indices and insert them, with their values offset by
                // 'vertexCount' so that they are relative to the first newly inserted vertex.
                for (unsigned int i = 0; i < indexCount; ++i)
                {
                    _indicesPtr[i] = ((uint16_t*)indices)[i] + _vertexCount;
                }
            }
            else if (_indexFormat == INDEX32) {
                uint32_t* _indicesPtr = (uint32_t*)(_indexBuffer._data + (part->_indexCount * indexSize));
                if (_primitiveType == Mesh::TRIANGLE_STRIP)
                {
                    // Create a degenerate triangle to connect separate triangle strips
                    // by duplicating the previous and next vertices.
                    _indicesPtr[0] = *(_indicesPtr - 1);
                    _indicesPtr[1] = _vertexCount;
                    _indicesPtr += 2;
                }

                // Loop through all indices and insert them, with their values offset by
                // 'vertexCount' so that they are relative to the first newly inserted vertex.
                for (unsigned int i = 0; i < indexCount; ++i)
                {
                    _indicesPtr[i] = ((uint32_t*)indices)[i] + _vertexCount;
                }
            }
        }
        part->_indexCount = newIndexCount;
    }
    else {
        GP_ASSERT(_parts.size() == 0);
    }
    _vertexCount = newVertexCount;
}

void Mesh::clearData() {
    _vertexBuffer._dataSize = 0;
    _indexBuffer._dataSize = 0;
    _vertexCount = 0;
    for (MeshPart& part : _parts) {
        part._indexCount = 0;
    }
    _boundingSphere = BoundingSphere::empty();
    _boundingBox = BoundingBox::empty();
}

}
