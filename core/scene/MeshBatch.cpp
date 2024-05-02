#include "base/Base.h"
#include "MeshBatch.h"
#include "material/Material.h"
#include "scene/Renderer.h"
namespace mgp
{

MeshBatch::MeshBatch(const VertexFormat& vertexFormat, Mesh::PrimitiveType primitiveType, UPtr<Material> material, Mesh::IndexFormat indexFormat, unsigned int initialCapacity, unsigned int growSize)
    : _material(std::move(material)), _mesh(vertexFormat), _started(false)
{
    _mesh._indexFormat = indexFormat;
    _mesh._dynamic = true;
    _mesh.setPrimitiveType(primitiveType);
    if (vertexFormat.getVertexSize()) {
        _mesh._vertexBuffer._growSize = growSize * vertexFormat.getVertexSize();
    }
    else {
        _mesh._vertexBuffer._growSize = growSize;
    }
    _mesh._indexBuffer._growSize = growSize * _mesh.getIndexSize();
    setCapacity(initialCapacity);

    _highlightType = Drawable::SharedColor;
}

MeshBatch::~MeshBatch()
{
    _mesh._setRefCount(0);
}

UPtr<MeshBatch> MeshBatch::create(const VertexFormat& vertexFormat, Mesh::PrimitiveType primitiveType, const char* materialPath, Mesh::IndexFormat indexFormat, unsigned int initialCapacity, unsigned int growSize)
{
    UPtr<Material> material = Material::create(materialPath);
    if (material.get() == NULL)
    {
        GP_ERROR("Failed to create material for mesh batch from file '%s'.", materialPath);
        return UPtr<MeshBatch>(NULL);
    }
    UPtr<MeshBatch> batch = create(vertexFormat, primitiveType, std::move(material), indexFormat, initialCapacity, growSize);
    //SAFE_RELEASE(material); // batch now owns the material
    return batch;
}

UPtr<MeshBatch> MeshBatch::create(const VertexFormat& vertexFormat, Mesh::PrimitiveType primitiveType, UPtr<Material> material, Mesh::IndexFormat indexFormat, unsigned int initialCapacity, unsigned int growSize)
{
    GP_ASSERT(material.get());
    MeshBatch* batch = new MeshBatch(vertexFormat, primitiveType, std::move(material), indexFormat, initialCapacity, growSize);
    return UPtr<MeshBatch>(batch);
}

void MeshBatch::write(Stream* file) {
    _mesh.write(file);
    file->writeUInt32(_batchIndex.size());
    file->write((char*)_batchIndex.data(), _batchIndex.size()*sizeof(uint32_t));
}
bool MeshBatch::read(Stream* file) {
    start();
    _mesh.read(file);
    int size = file->readUInt32();
    _batchIndex.resize(size);
    file->read((char*)_batchIndex.data(), size*sizeof(uint32_t));
    return true;
}

void MeshBatch::add(const void* vertices, unsigned int vertexCount, const void* indices, unsigned int indexCount)
{
    if (indices) {
        _batchIndex.push_back(_mesh.getIndexCount());
    }
    else {
        _batchIndex.push_back(_mesh.getVertexCount());
    }
    _mesh.merge(vertices, vertexCount, indices, indexCount);
}

static int getElementVertexCount(Mesh::PrimitiveType _primitiveType, int capacity) {
    bool vertexCapacity = 0;
    switch (_primitiveType)
    {
    case Mesh::LINES:
        vertexCapacity = capacity * 2;
        break;
    case Mesh::LINE_STRIP:
        vertexCapacity = capacity + 1;
        break;
    case Mesh::POINTS:
        vertexCapacity = capacity;
        break;
    case Mesh::TRIANGLES:
        vertexCapacity = capacity * 3;
        break;
    case Mesh::TRIANGLE_STRIP:
        vertexCapacity = capacity + 2;
        break;
    default:
        GP_ERROR("Unsupported primitive type for mesh batch (%d).", _primitiveType);
        break;
    }
    return vertexCapacity;
}

void MeshBatch::setCapacity(unsigned int capacity)
{
    int vertexCount = getElementVertexCount(_mesh.getPrimitiveType(), capacity);
    _mesh.getVertexBuffer()->setCapacity(vertexCount * _mesh.getVertexFormat().getVertexSize());
    _mesh.getIndexBuffer()->setCapacity(vertexCount * _mesh.getIndexSize());
}

const BoundingSphere* MeshBatch::getBoundingSphere() {
    if (_started) return NULL;
    return &_mesh.getBoundingSphere();
}

void MeshBatch::start()
{
    _mesh.clearData();
    _started = true;
    _batchIndex.clear();
}

bool MeshBatch::isStarted() const
{
    return _started;
}

void MeshBatch::finish()
{
    _started = false;
    getBoundingSphere();
}

void MeshBatch::draw(RenderInfo* view, Drawable *drawable)
{
    _mesh.draw(view, drawable, _material.get(), NULL, 0);
}

unsigned int MeshBatch::draw(RenderInfo* view) {
    if (_highlightMaterial.get() && _mesh.getPartCount() > 1) {
        std::vector<Material*> partMaterials(_mesh.getPartCount());
        partMaterials[0] = _material.get();
        for (int i = 1; i < _mesh.getPartCount(); ++i) {
            partMaterials[i] = _highlightMaterial.get();
        }
        return _mesh.draw(view, this, _material.get(), partMaterials.data(), _mesh.getPartCount());
    }
    else {
        return _mesh.draw(view, this, _material.get(), NULL, 0);
    }
}

bool MeshBatch::doRaycast(RayQuery& query) {

    bool res = false;
    for (int i = 0; i < _batchIndex.size(); ++i) {
        int offset = _batchIndex[i];
        int end = i + 1 < _batchIndex.size() ? _batchIndex[i + 1] : getMesh()->getIndexCount();
        int size = end - offset;

        if (_mesh.getIndexFormat() == Mesh::INDEX16) {
            if (_mesh.raycastPart<uint16_t>(query, offset*2, size, i, _mesh.getPrimitiveType())) res = true;
        }
        else if (_mesh.getIndexFormat() == Mesh::INDEX32) {
            if (_mesh.raycastPart<uint32_t>(query, offset*4, size, i, _mesh.getPrimitiveType())) res = true;
        }
    }
    return res;
}

bool MeshBatch::getBatchVertices(int i, std::vector<float>& vertices) {
    if (i < 0 || i >= _batchIndex.size()) return false;
    int offset = _batchIndex[i];
    int end = i + 1 < _batchIndex.size() ? _batchIndex[i + 1] : getMesh()->getIndexCount();

    uint32_t* indices = (uint32_t*)((char*)getMesh()->getIndexBuffer()->_data);
    char* verteix = (char*)getMesh()->getVertexBuffer()->_data;

    const VertexFormat::Element* positionElement = getMesh()->getVertexFormat().getPositionElement();
    if (!positionElement || positionElement->size != 3) return false;

    uint32_t nindex = 0;
    for (int j = offset; j < end; ++j) {
        uint32_t ia = indices[j];
        float* p = (float*)(verteix + (positionElement->stride * ia) + positionElement->offset);
        float x = p[0];
        float y = p[1];
        float z = p[2];
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
    }
    return true;
}
}
