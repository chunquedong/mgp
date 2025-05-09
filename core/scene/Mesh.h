#ifndef MESH_H_
#define MESH_H_

#include "base/Ref.h"
#include "base/Ptr.h"
#include "base/Stream.h"
#include "VertexFormat.h"
#include "math/Vector3.h"
#include "math/BoundingBox.h"
#include "math/BoundingSphere.h"
#include "Drawable.h"
#include "base/Resource.h"
#include "material/VertexAttributeBinding.h"
#include "math/LineSegment.h"

namespace mgp
{

class Material;
class Model;
//class VertexAttributeBinding;
class RenderInfo;
class Drawable;

typedef uint64_t BufferHandle;

struct RenderBuffer : public Refable {
    BufferHandle _bufferHandle = 0;
    char* _data = NULL;
    int _dataSize = 0;
    unsigned int _growSize = 1024;
    unsigned int _dataCapacity = 0;
    bool _contentDirty = false;
    bool _pointerDirty = false;

    RenderBuffer();
    ~RenderBuffer();

    void setCapacity(int capacity);
    void resize(int size);
    void setData(char* data, int size, bool copy = true);
    void updateData(char* src, int dst_offset, int size);
    int addData(char* data, int size);

    void* mapVertexBuffer();
    bool unmapVertexBuffer();
};

/**
 * Defines a mesh supporting various vertex formats and define how the vertices are connected.
 */
class Mesh : public Resource
{
    friend class Model;
    friend class Bundle;
    friend class MeshFactory;
    friend class PhysicsController;
    friend class MeshBatch;
public:

    /**
     * Defines supported index formats.
     */
    enum IndexFormat
    {
        //INDEX8 = 0x1401,
        INDEX16 = 0x1403,
        INDEX32 = 0x1405
    };

    /**
     * Defines supported primitive types.
     */
    enum PrimitiveType
    {
        TRIANGLES = 0x0004,
        TRIANGLE_STRIP = 0x0005,
        LINES = 0x0001,
        LINE_STRIP = 0x0003,
        POINTS = 0x0000,
        TRIANGLE_FAN = 0x0006,
        LINE_LOOP = 0x0007,
    };

    /**
     * Constructs a new mesh with the specified vertex format.
     *
     * @param vertexFormat The vertex format.
     * @param vertexCount The number of vertices.
     * @param dynamic true if the mesh is dynamic; false otherwise.
     * 
     * @return The created mesh.
     * @script{create}
     */
    static UPtr<Mesh> create(const VertexFormat& vertexFormat, IndexFormat indexFormat = INDEX16, bool dynamic = false);
    static UPtr<Mesh> createMesh(const VertexFormat& vertexFormat, unsigned int vertexCount, IndexFormat indexFormat = INDEX16, bool dynamic = false);


    /**
     * Returns a URL from which the mesh was loaded from.
     *
     * For meshes loaded from a Bundle, this URL will point
     * to the file and ID of the mesh within the bundle. For
     * all other meshes, an empty string will be returned.
     */
    const char* getUrl() const;

    /**
     * Gets the vertex format for the mesh.
     *
     * @return The vertex format.
     */
    const VertexFormat& getVertexFormat() const;
    VertexFormat& getVertexFormat();

    /**
     * Gets the number of vertices in the mesh.
     *
     * @return The number of vertices in the mesh.
     */
    unsigned int getVertexCount() const;
    void setVertexCount(unsigned int c);

    /**
     * Returns a handle to the vertex buffer for the mesh.
     *
     * @return The vertex buffer object handle.
     */
    RenderBuffer* getVertexBuffer();

    /**
     * Determines if the mesh is dynamic.
     *
     * @return true if the mesh is dynamic; false otherwise.
     */
    bool isDynamic() const;

    /**
     * Returns the primitive type of the vertices in the mesh.
     *
     * The default primitive type for a Mesh is TRIANGLES.
     * 
     * @return The primitive type.
     *
     * @see setPrimitiveType(PrimitiveType)
     */
    PrimitiveType getPrimitiveType() const;

    /**
     * Sets the primitive type for the vertices in the mesh.
     *
     * The primitive type for a Mesh is only meaningful for meshes that do not
     * have any MeshParts. When there are no MeshParts associated with a mesh,
     * the Mesh is drawn as non-indexed geometry and the PrimitiveType of the Mesh
     * determines how the vertices are interpreted when drawn.
     *
     * @param type The new primitive type.
     */
    void setPrimitiveType(Mesh::PrimitiveType type);

    /**
     * Gets the number of indices in the part.
     *
     * @return The number of indices in the part.
     */
    unsigned int getIndexCount() const;
    //void setIndexCount(int indexCount);
    bool isIndexed() const;

    /**
    * per index byte size
    */
    unsigned int getIndexSize() const;

    /**
     * Returns the format of the part indices.
     *
     * @return The part index format.
     */
    Mesh::IndexFormat getIndexFormat() const;

    /**
     * Returns a handle to the index buffer for the mesh part.
     *
     * @return The index buffer object handle.
     */
    RenderBuffer* getIndexBuffer();

    /**
     * Creates and adds a new part of primitive data defining how the vertices are connected.
     *
     * @param primitiveType The type of primitive data to connect the indices as.
     * @param indexFormat The format of the indices. SHORT or INT.
     * @param indexCount The number of indices to be contained in the part.
     */
    void setIndex(PrimitiveType primitiveType, unsigned int indexCount, unsigned int bufferOffset = 0);

    /**
    * Creates a mesh that shared buffer with this mesh
    * 
    * @param primitiveType The type of primitive data to connect the indices as.
    * @param indexFormat The format of the indices. SHORT or INT.
    * @param indexCount The number of indices to be contained in the part.
    */
    UPtr<Mesh> createMeshPart(PrimitiveType primitiveType, unsigned int indexCount, unsigned int bufferOffset = 0);

    /**
     * Returns the bounding box for the points in this mesh.
     * 
     * Only meshes loaded from bundle files are imported with valid
     * bounding volumes. Programmatically created meshes will contain
     * empty bounding volumes until the setBoundingBox and/or
     * setBoundingSphere methods are called to specify the mesh's
     * local bounds.
     *
     * Meshes that are attached to a Model with a MeshSkin will have
     * a bounding volume that is not necessarily tight fighting on the
     * Mesh vertices. Instead, the bounding volume will be an approximation
     * that contains all possible vertex positions in all possible poses after
     * skinning is applied. This is necessary since skinning vertices 
     * result in vertex positions that lie outside the original mesh bounds
     * and could otherwise result in a bounding volume that does not fully
     * contain an animated/skinned mesh.
     *
     * @return The bounding box for the mesh.
     */
    const BoundingBox& getBoundingBox();

    /**
     * Sets the bounding box for this mesh.
     *
     * @param box The new bounding box for the mesh.
     */
    void setBoundingBox(const BoundingBox& box);

    /**
     * Returns the bounding sphere for the points in the mesh.
     *
     * Only meshes loaded from bundle files are imported with valid
     * bounding volumes. Programmatically created meshes will contain
     * empty bounding volumes until the setBoundingBox and/or
     * setBoundingSphere methods are called to specify the mesh's
     * local bounds.
     *
     * Meshes that are attached to a Model with a MeshSkin will have
     * a bounding volume that is not necessarily tight fighting on the
     * Mesh vertices. Instead, the bounding volume will be an approximation
     * that contains all possible vertex positions in all possible poses after
     * skinning is applied. This is necessary since skinning vertices 
     * result in vertex positions that lie outside the original mesh bounds
     * and could otherwise result in a bounding volume that does not fully
     * contain an animated/skinned mesh.
     *
     * @return The bounding sphere for the mesh.
     */
    const BoundingSphere& getBoundingSphere();

    /**
     * Sets the bounding sphere for this mesh.
     *
     * @param sphere The new bounding sphere for the mesh.
     */
    void setBoundingSphere(const BoundingSphere& sphere);

    /**
     * Destructor.
     */
    virtual ~Mesh();

    void write(Stream* file);
    bool read(Stream* file);

    unsigned int draw(RenderInfo* view, Drawable* drawable, Material* _material);

    /**
    * Return the intersection point distance to ray origin.
    * 
    * @target intersection point
    * @index output intersection position info. index[0] = part index; index[1] = indices index;
    **/
    bool doRaycast(RayQuery& query);

    /**
     * Adds a group of primitives to the batch.
     *
     * The vertex list passed in should be a pointer of floats where every X floats represent a
     * single vertex (e.g. {x,y,z,u,v}).
     *
     * If the batch was created with 'indexed' set to true, then valid index data should be
     * passed in this method. However, if 'indexed' was set to false, the indices and indexCount
     * parameters can be omitted since only vertex data will be used.
     *
     * If the batch created to draw triangle strips, this method assumes that separate calls to
     * add specify separate triangle strips. In this case, this method will automatically stitch
     * separate triangle strips together using degenerate (zero-area) triangles.
     *
     * @param vertices Array of vertices.
     * @param vertexCount Number of vertices.
     * @param indices Array of indices into the vertex array (should be NULL for non-indexed batches).
     * @param indexCount Number of indices (should be zero for non-indexed batches).
     */
    void merge(const void* vertices, unsigned int vertexCount, const void* indices, unsigned int indexCount);

    void clearData();

    void setVertexFormatDirty();

    int getPartIndex() { return _partIndex; }

    bool isVisiable() { return _visiable; }
    void setVisiable(bool b) { _visiable = b; }

    void setVertexBuffer(SPtr<RenderBuffer> b) { _vertexBuffer = b; }
    void setIndexBuffer(SPtr<RenderBuffer> b) { _indexBuffer = b; }
public:
    template<typename T> 
    bool raycastPart(RayQuery& query, int _bufferOffset, int _indexCount, int partIndex, PrimitiveType _primitiveType, int id = -1);

    /**
     * Constructor.
     */
    Mesh();
protected:
    /**
     * Constructor.
     */
    Mesh(const Mesh& copy) = delete;

    /**
     * Hidden copy assignment operator.
     */
    Mesh& operator=(const Mesh&) = delete;

    void computeBounds();

    std::string _url;
    
    BoundingBox _boundingBox;
    BoundingSphere _boundingSphere;

    VertexFormat _vertexFormat;
    PrimitiveType _primitiveType = TRIANGLES;
    bool _dynamic = false;

    //vertices
    SPtr<RenderBuffer> _vertexBuffer;
    unsigned int _vertexCount = 0;

    //indices
    SPtr<RenderBuffer> _indexBuffer;
    Mesh::IndexFormat _indexFormat = INDEX16;
    int _indexCount = 0;
    int _bufferOffset = 0;
    bool _isIndexed = false;

    //index in parent
    int _partIndex = 0;
    bool _visiable = true;

    SPtr<VertexAttributeBinding> _vertexAttributeArray;
    bool _dirtyVertexFormat = false;
};


template<typename T> bool Mesh::raycastPart(RayQuery& query, int _bufferOffset,
        int _indexCount, int partIndex, PrimitiveType _primitiveType, int id) {
    if (_primitiveType == Mesh::LINE_LOOP || _primitiveType == Mesh::LINE_STRIP) {
        return false;
    }
    int minTriangle = -1;
    Vector3 curTarget;

    T* indices = (T*)((char*)this->_indexBuffer->_data + _bufferOffset);
    char* verteix = (char*)_vertexBuffer->_data;
    const VertexFormat::Element* positionElement = _vertexFormat.getPositionElement();
    if (!positionElement || positionElement->size != 3) return false;
    int count = _indexCount;

    if (_primitiveType == Mesh::TRIANGLE_STRIP) {
        Vector3 a;
        Vector3 b;
        Vector3 c;
        for (int j = 0; j+2 < count; j += 1) {
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
                    minTriangle = j;
                    query.target = curTarget;
                    if (query.getNormal) {
                        triangleNormal(a, b, c, &query.normal);
                    }
                }
            }
        }
    }
    else if (_primitiveType == Mesh::TRIANGLE_FAN) {
        Vector3 a;
        Vector3 b;
        Vector3 c;
        for (int j = 1; j + 1 < count; j += 1) {
            T ia = indices[0];
            T ib = indices[j];
            T ic = indices[j + 1];
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
                    minTriangle = j;
                    query.target = curTarget;
                    if (query.getNormal) {
                        triangleNormal(a, b, c, &query.normal);
                    }
                }
            }
        }
    }
    else if (_primitiveType == Mesh::TRIANGLES) {
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
                    minTriangle = j;
                    query.target = curTarget;
                    if (query.getNormal) {
                        triangleNormal(a, b, c, &query.normal);
                    }
                }
            }
        }
    }
    else if (_primitiveType == Mesh::LINES) {
        Vector3 a;
        Vector3 b;
        uint32_t ia;
        uint32_t ib;
        for (int j = 0; j < count; j += 2)
        {
            ia = indices[j];
            ib = indices[j + 1];

            float* af = (float*)(verteix + (positionElement->stride * ia) + positionElement->offset);
            float* bf = (float*)(verteix + (positionElement->stride * ib) + positionElement->offset);
            //float to double
            a.x = af[0]; a.y = af[1]; a.z = af[2];
            b.x = bf[0]; b.y = bf[1]; b.z = bf[2];
            Vector3 optionalPointOnRay;
            Vector3 optionalPointOnSegment;
            double disSq = query.ray.distanceSqToSegment(a, b, &optionalPointOnRay, &optionalPointOnSegment);
            double disToOrigin = query.ray.getOrigin().distance(optionalPointOnRay);
            double limitDis = query.fovDivisor * disToOrigin * query.tolerance;
            if (disToOrigin < query.minDistance && disSq < limitDis * limitDis) {
                query.minDistance = disToOrigin;
                query.target = optionalPointOnSegment;
                minTriangle = j;
            }
            //printf("dis:%f\n", sqrt(disSq));
        }
    }
    else if (_primitiveType == Mesh::POINTS) {
        Vector3 a;
        uint32_t ia;
        for (int j = 0; j < count; j += 2)
        {
            ia = indices[j];

            float* af = (float*)(verteix + (positionElement->stride * ia) + positionElement->offset);
            //float to double
            a.x = af[0]; a.y = af[1]; a.z = af[2];
            double disSq = query.ray.distanceSqToPoint(a);
            double disToOrigin = query.ray.getOrigin().distance(a);
            double limitDis = query.fovDivisor * disToOrigin * query.tolerance;
            if (disToOrigin < query.minDistance && disSq < limitDis * limitDis) {
                query.minDistance = disToOrigin;
                query.target = a;
                minTriangle = j;
            }
            //printf("dis:%f\n", sqrt(disSq));
        }
    }

    if (minTriangle != -1) {
        std::vector<int> path = { partIndex, minTriangle };
        query.path = path;
        query.id = id;
        return true;
    }
    return false;
}


}

#endif
