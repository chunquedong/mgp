#ifndef TERRAINPATCH_H_
#define TERRAINPATCH_H_

#include "scene/Model.h"
#include "scene/Camera.h"
#include "../material/Texture.h"

namespace mgp
{

class Terrain;
class HeightField;

/**
 * Defines a single patch for a Terrain.
 */
class TerrainPatch : public Camera::Listener
{
    friend class Terrain;
    //friend class TerrainAutoBindingResolver;

public:

    /**
     * Gets the number of material for this patch for all level of details.
     *
     * @return The number of material for this patch for all level of details. 
     */
    unsigned int getMaterialCount() const;

    /**
     * Gets the material for the specified level of detail index or -1 for the current level of detail
     * based on the scene camera.
     *
     * @param index The index for the level of detail to get the material for.
     */
    Material* getMaterial(int index = -1) const;

    /**
     * Gets the local bounding box for this patch, at the base LOD level.
     */
    const BoundingBox& getBoundingBox(bool worldSpace) const;

    /**
     * @see Camera::Listener
     */
    void cameraChanged(Camera* camera);

    /**
     * Internal use only.
     *
     * @script{ignore}
     */
    static std::string passCallback(Material* pass, void* cookie);

private:

    /**
     * Constructor.
     */
    TerrainPatch();

    /**
     * Hidden copy constructor.
     */
    TerrainPatch(const TerrainPatch&);

    /**
     * Hidden copy assignment operator.
     */
    TerrainPatch& operator=(const TerrainPatch&);

    /**
     * Destructor.
     */
    ~TerrainPatch();

    struct Layer
    {
        Layer();

        Layer(const Layer&);

        ~Layer();

        Layer& operator=(const Layer&);

        int index;
        int row;
        int column;
        int textureIndex;
        Vector2 textureRepeat;
        int blendIndex;
        int blendChannel;
    };

    struct Level
    {
        Model* model;

        Level();
    };

    struct LayerCompare
    {
        bool operator() (const Layer* lhs, const Layer* rhs) const;
    };

    /**
    * @index patch index id
    * @row row of patch in terrain
    * @column column of path in terrain
    * @heights height field data
    * @hfWidth height field size
    * @hfHeight height field size
    * @x1, z2, x2, z2 coordinate in pixcel of height field
    * @xOffset zOffset coordinate offset in pixcel of height field
    * @maxStep for LOD
    * @verticalSkirtSize skirt size value
    */
    static TerrainPatch* create(Terrain* terrain, unsigned int index,
                                unsigned int row, unsigned int column,
                                unsigned int x1, unsigned int z1, unsigned int x2, unsigned int z2,
                                float xOffset, float zOffset, unsigned int detailLevels, float verticalSkirtSize);

    void initLOD(int dlevel);


    void resetMesh();


    bool setLayer(int index, Texture* texturePath, const Vector2& textureRepeat, Texture* blendPath, int blendChannel);

    void deleteLayer(Layer* layer);

    int addSampler(Texture* path);

    unsigned int draw(RenderInfo* view);

    bool updateMaterial();
    bool updateLevelMaterial(int level);

    unsigned int computeLOD(Camera* camera, const BoundingBox& worldBounds);

    const Vector3& getAmbientColor() const;

    void setMaterialDirty();

    float computeHeight(float* heights, unsigned int width, unsigned int x, unsigned int z);

    void updateNodeBindings();

    std::string passCreated(Material *pass);

    Terrain* _terrain;
    unsigned int _index;
    unsigned int _row;
    unsigned int _column;
    std::vector<Level*> _levels;
    std::set<Layer*, LayerCompare> _layers;
    std::vector<Texture*> _samplers;
    mutable BoundingBox _boundingBox;
    mutable BoundingBox _boundingBoxWorld;
    mutable Camera* _camera;
    mutable unsigned int _level;
    mutable int _bits;
    HeightField* _heightfield = nullptr;

    unsigned int _x1;
    unsigned int _z1;
    unsigned int _x2;
    unsigned int _z2;
    float _xOffset;
    float _zOffset;
    unsigned int _maxStep;
    float _verticalSkirtSize;
    int _detailLevels;
};

}

#endif
