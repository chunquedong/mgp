#include "base/Base.h"
#include "Mesh.h"
#include "material/ShaderProgram.h"
#include "Model.h"
#include "material/Material.h"
#include "scene/Renderer.h"
#include "scene/Drawable.h"
#include <float.h>
#include "math/LineSegment.h"

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



Mesh::Mesh() :
    _vertexBuffer(new RenderBuffer()), _indexBuffer(new RenderBuffer())
{
}

Mesh::~Mesh()
{
    _vertexCount = 0;
    _indexCount = 0;
}

UPtr<Mesh> Mesh::create(const VertexFormat& vertexFormat, IndexFormat indexFormat, bool dynamic) {
    Mesh* mesh = new Mesh();
    mesh->_vertexFormat = vertexFormat;
    mesh->_dynamic = dynamic;
    mesh->_indexFormat = indexFormat;
    return UPtr<Mesh>(mesh);
}

UPtr<Mesh> Mesh::createMesh(const VertexFormat& vertexFormat, unsigned int vertexCount, IndexFormat indexFormat, bool dynamic)
{
    Mesh* mesh = new Mesh();
    mesh->_vertexFormat = vertexFormat;
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

void Mesh::setVertexCount(unsigned int c) {
    _vertexCount = c;
}

RenderBuffer* Mesh::getVertexBuffer()
{
    return _vertexBuffer.get();
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
    return _indexCount;
}

bool Mesh::isIndexed() const {
    return _isIndexed;
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
    return _indexBuffer.get();
}

void Mesh::setIndex(PrimitiveType primitiveType, unsigned int indexCount, unsigned int bufferOffset)
{
    _primitiveType = primitiveType;
    _indexCount = indexCount;
    _bufferOffset = bufferOffset;
    _isIndexed = true;
}

UPtr<Mesh> Mesh::createMeshPart(PrimitiveType primitiveType, unsigned int indexCount, unsigned int bufferOffset)
{
    UPtr<Mesh> mesh(new Mesh());
    mesh->_vertexFormat = this->_vertexFormat;
    mesh->_indexFormat = this->_indexFormat;
    mesh->_vertexBuffer = this->_vertexBuffer;
    mesh->_indexBuffer = this->_indexBuffer;
    mesh->_vertexCount = this->_vertexCount;
    mesh->_boundingBox = this->_boundingBox;
    mesh->_boundingSphere = this->_boundingSphere;
    mesh->_url = this->_url;
    mesh->_dynamic = this->_dynamic;
    mesh->_vertexAttributeArray = this->_vertexAttributeArray;

    mesh->setIndex(primitiveType, indexCount, bufferOffset);
    return mesh;
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
    file->writeUInt8(0);
    file->writeUInt8(_dynamic);

    // vertices
    file->writeUInt32(_vertexCount);
    file->writeUInt32(_vertexBuffer->_dataSize);
    file->write((const char*)_vertexBuffer->_data, _vertexBuffer->_dataSize);

    // write indices buffer
    file->writeUInt16(_indexFormat);
    file->writeUInt32(_indexBuffer->_dataSize);
    file->write(_indexBuffer->_data, _indexBuffer->_dataSize);

    // parts
    file->writeUInt8(_primitiveType);
    file->writeUInt32(_bufferOffset);
    file->writeUInt32(_indexCount);

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
    file->readUInt8();
    mesh->_dynamic = file->readUInt8();

    mesh->_vertexCount = file->readUInt32();
    int bufSize = file->readUInt32();
    void *vertexData = malloc(bufSize);
    file->read((char*)vertexData, bufSize);
    mesh->_vertexBuffer->setData((char*)vertexData, bufSize, false);

    //mesh->_vertexDataDirty = true;

    mesh->_indexFormat = (IndexFormat)file->readUInt16();
    int ibufsize = file->readUInt32();
    void* indexData = malloc(ibufsize);
    file->read((char*)indexData, ibufsize);
    mesh->_indexBuffer->setData((char*)indexData, ibufsize, false);

    _primitiveType = (Mesh::PrimitiveType)file->readUInt8();
    _bufferOffset = file->readUInt32();
    _indexCount = file->readUInt32();

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
    if (_vertexCount == 0) return;
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
    if (!positionElement || positionElement->size != 3) return;

    for (int i = 0; i < _vertexCount; ++i)
    {
        float *p = (float*)((char*)_vertexBuffer->_data + (i * positionElement->stride) + positionElement->offset);
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
        float* p = (float*)((char*)_vertexBuffer->_data + (i * positionElement->stride) + positionElement->offset);
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

unsigned int Mesh::draw(RenderInfo* view, Drawable* drawable, Material* _material)
{
    Mesh* _mesh = this;
    GP_ASSERT(_mesh);

    if (!_visiable) {
        return 0;
    }

    if (_vertexBuffer->_bufferHandle == 0) {
        _vertexBuffer->_bufferHandle = Renderer::cur()->createBuffer(0);
    }
    if (_vertexBuffer->_contentDirty) {
        GP_ASSERT(_vertexBuffer->_dataSize >= _vertexCount * _vertexFormat.getVertexSize());
        Renderer::cur()->setBufferData(_vertexBuffer->_bufferHandle, 0, 0, (const char*)_vertexBuffer->_data, _vertexBuffer->_dataSize, _dynamic);
        _vertexBuffer->_contentDirty = false;
    }

    if (_isIndexed) {
        if (_indexBuffer->_bufferHandle == 0) {
            _indexBuffer->_bufferHandle = Renderer::cur()->createBuffer(1);
        }
        if (_indexBuffer->_contentDirty) {
            GP_ASSERT(_indexBuffer->_dataSize >= _mesh->_indexCount * getIndexSize());
            Renderer::cur()->setBufferData(_indexBuffer->_bufferHandle, 1, 0, (const char*)_indexBuffer->_data, _indexBuffer->_dataSize, _dynamic);
            _indexBuffer->_contentDirty = false;
        }
    }

    Mesh* mesh = _mesh;
    if (!mesh->_vertexAttributeArray.get()) {
        BufferHandle indexBufferObject = 0;
        if (_isIndexed) indexBufferObject = _indexBuffer->_bufferHandle;
        mesh->_vertexAttributeArray = VertexAttributeBinding::create(mesh->_vertexBuffer->_bufferHandle, mesh->getVertexFormat(), NULL, indexBufferObject).get();
        _vertexBuffer->_pointerDirty = false;
        _indexBuffer->_pointerDirty = false;
    }
    else if (_dirtyVertexFormat || _vertexBuffer->_pointerDirty || _indexBuffer->_pointerDirty) {

        if (mesh->_vertexAttributeArray->_indexBufferObject != _indexBuffer->_bufferHandle) {
            mesh->_vertexAttributeArray->_indexBufferObject = _indexBuffer->_bufferHandle;
        }

        if (_dirtyVertexFormat) {
            mesh->_vertexAttributeArray->vertexFormat = mesh->getVertexFormat();
        }

        mesh->_vertexAttributeArray->update();
        _vertexBuffer->_pointerDirty = false;
        _indexBuffer->_pointerDirty = false;
        _dirtyVertexFormat = false;
    }

    if (!_isIndexed)
    {
        DrawCall drawCall;
        drawCall._drawable = drawable;
        for (Material* material = _material; material != NULL; material = material->getNextPass())
        {
            drawCall._vertexAttributeArray = mesh->_vertexAttributeArray.get();
            drawCall._material = material;
            drawCall._primitiveType = mesh->getPrimitiveType();
            drawCall._vertexCount = mesh->getVertexCount();
            if (drawable) drawCall._renderLayer = drawable->getRenderLayer();
            else drawCall._renderLayer = Drawable::Overlay;
            drawCall._mesh = mesh;

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
                drawCall._material->setParams(NULL, NULL, NULL, drawable, 0);
                Renderer::cur()->draw(&drawCall);
            }
        }
        return 1;
    }
    else {
        DrawCall drawCall;
        drawCall._drawable = drawable;
        for (Material* material = _material; material != NULL; material = material->getNextPass())
        {
            drawCall._vertexAttributeArray = mesh->_vertexAttributeArray.get();
            drawCall._material = material;
            drawCall._indexFormat = getIndexFormat();
            drawCall._indexBuffer = _indexBuffer->_bufferHandle;
            drawCall._primitiveType = this->_primitiveType;
            drawCall._vertexCount = mesh->getVertexCount();
            drawCall._indexCount = this->_indexCount;
            drawCall._indexBufferOffset = this->_bufferOffset;
            if (drawable) drawCall._renderLayer = drawable->getRenderLayer();
            else drawCall._renderLayer = Drawable::Overlay;
            //drawCall._instanceVbo = view->_instanceVbo;
            //drawCall._instanceCount = view->_instanceCount;
            drawCall._mesh = mesh;

            if (view) {
                view->draw(&drawCall);
                if (view->wireframe || view->isDepthPass) break;
            }
            else {
                drawCall._material->setParams(NULL, NULL, NULL, drawable, 0);
                Renderer::cur()->draw(&drawCall);
            }
        }
    }

    return 1;
}

bool Mesh::doRaycast(RayQuery& query) {
    bool res = false;

    if (!_isIndexed) {
        int minTriangle = -1;
        Vector3 curTarget;
        char* verteix = (char*)_vertexBuffer->_data;
        const VertexFormat::Element* positionElement = _vertexFormat.getPositionElement();
        if (!positionElement || positionElement->size != 3) return false;
        int count = getVertexCount();

        if (_primitiveType == Mesh::TRIANGLES) {
            Vector3 a;
            Vector3 b;
            Vector3 c;
            for (int j = 0; j < count; j += 3) {
                float* af = (float*)(verteix + (positionElement->stride * j) + positionElement->offset);
                float* bf = (float*)(verteix + (positionElement->stride * (j+1)) + positionElement->offset);
                float* cf = (float*)(verteix + (positionElement->stride * (j+2)) + positionElement->offset);
                //float to double
                a.x = af[0]; a.y = af[1]; a.z = af[2];
                b.x = bf[0]; b.y = bf[1]; b.z = bf[2];
                c.x = cf[0]; c.y = cf[1]; c.z = cf[2];
                double dis = query.ray.intersectTriangle(a, b, c, query.backfaceCulling, &curTarget);
                if (dis != Ray::INTERSECTS_NONE) {
                    if (dis < query.minDistance) {
                        query.minDistance = dis;
                        minTriangle = j;
                        query.target = curTarget;
                        if (query.getNormal) {
                            triangleNormal(a, b, c, &query.normal);
                        }
                    }
                }
            }
        }
        else if (_primitiveType == Mesh::TRIANGLE_STRIP) {
            Vector3 a;
            Vector3 b;
            Vector3 c;
            for (int j = 0; j+2 < count; j += 1) {
                float* af = (float*)(verteix + (positionElement->stride * j) + positionElement->offset);
                float* bf = (float*)(verteix + (positionElement->stride * (j + 1)) + positionElement->offset);
                float* cf = (float*)(verteix + (positionElement->stride * (j + 2)) + positionElement->offset);
                //float to double
                a.x = af[0]; a.y = af[1]; a.z = af[2];
                b.x = bf[0]; b.y = bf[1]; b.z = bf[2];
                c.x = cf[0]; c.y = cf[1]; c.z = cf[2];
                double dis = query.ray.intersectTriangle(a, b, c, query.backfaceCulling, &curTarget);
                if (dis != Ray::INTERSECTS_NONE) {
                    if (dis < query.minDistance) {
                        query.minDistance = dis;
                        minTriangle = j;
                        query.target = curTarget;
                        if (query.getNormal) {
                            triangleNormal(a, b, c, &query.normal);
                        }
                    }
                }
            }
        }

        if (minTriangle != -1) {
            std::vector<int> path = { -1, minTriangle };
            query.path = path;
            res = true;
        }
        return res;
    }
    else
    {
        if (getIndexFormat() == Mesh::INDEX16) {
            if (raycastPart<uint16_t>(query, this->_bufferOffset, this->_indexCount, _partIndex, this->_primitiveType)) res = true;
        }
        else if (getIndexFormat() == Mesh::INDEX32) {
            if (raycastPart<uint32_t>(query, this->_bufferOffset, this->_indexCount, _partIndex, this->_primitiveType)) res = true;
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
    _vertexBuffer->addData((char*)vertices, vBytes);

    // Copy index data.
    if (indices)
    {
        if (!_isIndexed) {
            setIndex(this->_primitiveType, 0);
        }

        unsigned int newIndexCount = this->_indexCount + indexCount;
        unsigned int indexSize = getIndexSize();
        
        if (_vertexCount == 0)
        {
            // Simply copy values directly into the start of the index array.
            _indexBuffer->addData((char*)indices, indexCount * indexSize);
        }
        else
        {
            if (_primitiveType == Mesh::TRIANGLE_STRIP && _vertexCount > 0) {
                newIndexCount += 2; // need an extra 2 indices for connecting strips with degenerate triangles
            }
            _indexBuffer->resize(newIndexCount * indexSize);

            if (_indexFormat == INDEX16) {
                uint16_t* _indicesPtr = (uint16_t*)(_indexBuffer->_data + (this->_indexCount * indexSize));
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
                uint32_t* _indicesPtr = (uint32_t*)(_indexBuffer->_data + (this->_indexCount * indexSize));
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
        this->_indexCount = newIndexCount;
    }
    else {
        GP_ASSERT(!_isIndexed);
    }
    _vertexCount = newVertexCount;
}

void Mesh::clearData() {
    _vertexBuffer->_dataSize = 0;
    _indexBuffer->_dataSize = 0;
    _vertexCount = 0;
    this->_indexCount = 0;
    _boundingSphere = BoundingSphere::empty();
    _boundingBox = BoundingBox::empty();
}

void Mesh::setVertexFormatDirty() {
    _dirtyVertexFormat = true;
}

}
