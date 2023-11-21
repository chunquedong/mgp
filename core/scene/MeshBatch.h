#ifndef MESHBATCH_H_
#define MESHBATCH_H_

#include "Mesh.h"
#include "Drawable.h"

namespace mgp
{

class Material;
class VertexAttributeBinding;
class RenderInfo;
class Drawable;

/**
 * Defines a class for rendering multiple mesh into a single draw call on the graphics device.
 */
class MeshBatch: public Drawable
{
    friend class GLRenderer;
public:

    /**
     * Creates a new mesh batch.
     *
     * @param vertexFormat The format of vertices in the new batch.
     * @param primitiveType The type of primitives that will be added to the batch.
     * @param materialPath Path to a material file to be used for drawing the batch.
     * @param indexed True if the batched primitives will contain index data, false otherwise.
     * @param initialCapacity The initial capacity of the batch, in triangles.
     * @param growSize Amount to grow the batch by when it overflows (a value of zero prevents batch growing).
     *
     * @return A new mesh batch.
     * @script{create}
     */
    static UPtr<MeshBatch> create(const VertexFormat& vertexFormat, Mesh::PrimitiveType primitiveType, const char* materialPath, Mesh::IndexFormat indexFormat = Mesh::INDEX16, unsigned int initialCapacity = 1024, unsigned int growSize = 1024);

    /**
     * Creates a new mesh batch.
     *
     * @param vertexFormat The format of vertices in the new batch.
     * @param primitiveType The type of primitives that will be added to the batch.
     * @param material Material to be used for drawing the batch.
     * @param indexed True if the batched primitives will contain index data, false otherwise.
     * @param initialCapacity The initial capacity of the batch, in triangles.
     * @param growSize Amount to grow the batch by when it overflows (a value of zero prevents batch growing).
     *
     * @return A new mesh batch.
     * @script{create}
     */
    static UPtr<MeshBatch> create(const VertexFormat& vertexFormat, Mesh::PrimitiveType primitiveType, UPtr<Material> material, Mesh::IndexFormat indexFormat = Mesh::INDEX16, unsigned int initialCapacity = 1024, unsigned int growSize = 1024);

    /**
     * Destructor.
     */
    ~MeshBatch();

    /**
     * Explicitly sets a new capacity for the batch.
     *
     * @param capacity The new batch capacity (element count).
     */
    void setCapacity(unsigned int capacity);

    /**
     * Returns the material for this mesh batch.
     *
     * @return The material used to draw the batch.
     */
    inline Material* getMaterial() const { return _material.get(); }
    Material* getMainMaterial() const { return getMaterial(); };

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
    void add(const void* vertices, unsigned int vertexCount, const void* indices = NULL, unsigned int indexCount = 0);

    /**
     * Starts batching.
     *
     * This method should be called before calling add() to add primitives to the batch.
     * After all primitives have been added to the batch, call the finish() method to
     * complete the batch.
     *
     * Calling this method will clear any primitives currently in the batch and set the
     * position of the batch back to the beginning.
     */
    void start();

    /**
    * Determines if the batch has been started and not yet finished.
    */
    bool isStarted() const;

    /**
     * Indicates that batching is complete and prepares the batch for drawing.
     */
    void finish();

    /**
     * Draws the primitives currently in batch.
     */
    void draw(RenderInfo* view, Drawable* drawable);

    unsigned int draw(RenderInfo* view) override;

    /**
    * Return the intersection point distance to ray origin.
    *
    * @target intersection point
    * @index output intersection position info. index[0] = indices index or triangle index;
    **/
    bool doRaycast(RayQuery& query);


    Mesh* getMesh() { return &_mesh; }

    int getBatchSize() const { return _batchIndex.size(); }
    const std::vector<uint32_t>& getBatchIndex() { return _batchIndex; }

    void write(Stream* file);
    bool read(Stream* file);

    const BoundingSphere* getBoundingSphere() override;
protected:

    /**
     * Constructor.
     */
    MeshBatch(const VertexFormat& vertexFormat, Mesh::PrimitiveType primitiveType, UPtr<Material> material, Mesh::IndexFormat indexFormat, unsigned int initialCapacity, unsigned int growSize);

    /**
     * Hidden copy constructor.
     */
    MeshBatch(const MeshBatch& copy);

    /**
     * Hidden copy assignment operator.
     */
    MeshBatch& operator=(const MeshBatch&);

    bool _started;

    UPtr<Material> _material;
    Mesh _mesh;
    std::vector<uint32_t> _batchIndex;
};


}


#endif
