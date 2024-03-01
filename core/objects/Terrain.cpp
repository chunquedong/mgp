#include "base/Base.h"
#include "Terrain.h"
#include "TerrainPatch.h"
#include "scene/Node.h"
#include "base/FileSystem.h"

namespace mgp
{

// Default terrain material path
#define TERRAIN_MATERIAL "res/materials/terrain.material"

// The default square size of terrain patches for a terrain that
// does not have an explicitly specified patch size.
static const unsigned int DEFAULT_TERRAIN_PATCH_SIZE = 32;

// The default height of a terrain that does not have an explicitly
// specified terrain size, expressed as a ratio of the average
// of the dimensions of the terrain heightfield:
//
//   heightMax = (image.width + image.height) / 2 * DEFAULT_TERRAIN_HEIGHT_RATIO
//
static const float DEFAULT_TERRAIN_HEIGHT_RATIO = 0.3f;

// Terrain dirty flags
static const unsigned int DIRTY_FLAG_INVERSE_WORLD = 1;

static float getDefaultHeight(unsigned int width, unsigned int height);

Terrain::Terrain() : Drawable(),
    _heightfield(NULL), _normalMap(NULL), _flags(FRUSTUM_CULLING | LEVEL_OF_DETAIL),
    _dirtyFlags(DIRTY_FLAG_INVERSE_WORLD)
{
    setLightMask(1);
}

Terrain::~Terrain()
{
    for (size_t i = 0, count = _patches.size(); i < count; ++i)
    {
        SAFE_DELETE(_patches[i]);
    }
    SAFE_RELEASE(_normalMap);
    //SAFE_RELEASE(_heightfield);

    for (Layer* lyr : _layers)
    {
        delete lyr;
    }
    _layers.clear();

    for (Texture* t : _samplers)
    {
        SAFE_RELEASE(t);
    }
    _samplers.clear();
}

UPtr<Terrain> Terrain::create(const char* path)
{
    return create(path, NULL);
}

UPtr<Terrain> Terrain::create(Properties* properties)
{
    return create(properties->getNamespace(), properties);
}

UPtr<Terrain> Terrain::create(const char* path, Properties* properties)
{
    // Terrain properties
    Properties* p = properties;
    Properties* pTerrain = NULL;
    bool externalProperties = (p != NULL);
    UPtr<HeightField> heightfield;
    Vector3 terrainSize;
    int patchSize = 0;
    int detailLevels = 1;
    float skirtScale = 0;
    const char* normalMap = NULL;
    std::string materialPath;

    if (!p && path)
    {
        p = Properties::create(path).take();
    }

    if (!p)
    {
        GP_WARN("Failed to properties for terrain: %s", path ? path : "");
        return UPtr<Terrain>(NULL);
    }

    pTerrain = strlen(p->getNamespace()) > 0 ? p : p->getNextNamespace();
    if (pTerrain == NULL)
    {
        GP_WARN("Invalid terrain definition.");
        if (!externalProperties)
            SAFE_DELETE(p);
        return UPtr<Terrain>(NULL);
    }

    // Read heightmap info
    Properties* pHeightmap = pTerrain->getNamespace("heightmap", true);
    if (pHeightmap)
    {
        // Read heightmap path
        std::string heightmap;
        if (!pHeightmap->getPath("path", &heightmap))
        {
            GP_WARN("No 'path' property supplied in heightmap section of terrain definition: %s", path);
            if (!externalProperties)
                SAFE_DELETE(p);
            return UPtr<Terrain>(NULL);
        }

        std::string ext = FileSystem::getExtension(heightmap.c_str());
        if (ext == ".PNG")
        {
            // Read normalized height values from heightmap image
            heightfield = HeightField::createFromImage(heightmap.c_str(), 0, 1);
        }
        else if (ext == ".RAW" || ext == ".R16")
        {
            // Require additional properties to be specified for RAW files
            Vector2 imageSize;
            if (!pHeightmap->getVector2("size", &imageSize))
            {
                GP_WARN("Invalid or missing 'size' attribute in heightmap defintion of terrain definition: %s", path);
                if (!externalProperties)
                    SAFE_DELETE(p);
                return UPtr<Terrain>(NULL);
            }

            // Read normalized height values from RAW file
            heightfield = HeightField::createFromRAW(heightmap.c_str(), (unsigned int)imageSize.x, (unsigned int)imageSize.y, 0, 1);
        }
        else
        {
            // Unsupported heightmap format
            GP_WARN("Unsupported heightmap format ('%s') in terrain definition: %s", heightmap.c_str(), path);
            if (!externalProperties)
                SAFE_DELETE(p);
            return UPtr<Terrain>(NULL);
        }
    }
    else
    {
        // Try to read 'heightmap' as a simple string property
        std::string heightmap;
        if (!pTerrain->getPath("heightmap", &heightmap))
        {
            GP_WARN("No 'heightmap' property supplied in terrain definition: %s", path);
            if (!externalProperties)
                SAFE_DELETE(p);
            return UPtr<Terrain>(NULL);
        }

        std::string ext = FileSystem::getExtension(heightmap.c_str());
        if (ext == ".PNG")
        {
            // Read normalized height values from heightmap image
            heightfield = HeightField::createFromImage(heightmap.c_str(), 0, 1);
        }
        else if (ext == ".RAW" || ext == ".R16")
        {
            GP_WARN("RAW heightmaps must be specified inside a heightmap block with width and height properties.");
            if (!externalProperties)
                SAFE_DELETE(p);
            return UPtr<Terrain>(NULL);
        }
        else
        {
            GP_WARN("Unsupported 'heightmap' format ('%s') in terrain definition: %s.", heightmap.c_str(), path);
            if (!externalProperties)
                SAFE_DELETE(p);
            return UPtr<Terrain>(NULL);
        }
    }

    // Read terrain 'size'
    if (pTerrain->exists("size"))
    {
        if (!pTerrain->getVector3("size", &terrainSize))
        {
            GP_WARN("Invalid 'size' value ('%s') in terrain definition: %s", pTerrain->getString("size"), path);
        }
    }

    // Read terrain 'patch size'
    if (pTerrain->exists("patchSize"))
    {
        patchSize = pTerrain->getInt("patchSize");
    }

    // Read terrain 'detailLevels'
    if (pTerrain->exists("detailLevels"))
    {
        detailLevels = pTerrain->getInt("detailLevels");
    }

    // Read 'skirtScale'
    if (pTerrain->exists("skirtScale"))
    {
        skirtScale = pTerrain->getFloat("skirtScale");
    }

    // Read 'normalMap'
    normalMap = pTerrain->getString("normalMap");

    // Read 'material'
    materialPath = pTerrain->getString("material", "");

    if (heightfield.isNull())
    {
        GP_WARN("Failed to read heightfield heights for terrain definition: %s", path);
        if (!externalProperties)
            SAFE_DELETE(p);
        return UPtr<Terrain>(NULL);
    }

    if (terrainSize.isZero())
    {
        terrainSize.set(heightfield->getColumnCount(), getDefaultHeight(heightfield->getColumnCount(), heightfield->getRowCount()), heightfield->getRowCount());
    }

    if (patchSize <= 0 || patchSize > (int)heightfield->getColumnCount() || patchSize > (int)heightfield->getRowCount())
    {
        patchSize = std::min(heightfield->getRowCount(), std::min(heightfield->getColumnCount(), DEFAULT_TERRAIN_PATCH_SIZE));
    }

    if (detailLevels <= 0)
        detailLevels = 1;

    if (skirtScale < 0)
        skirtScale = 0;

    // Compute terrain scale
    Vector3 scale(terrainSize.x / (heightfield->getColumnCount()-1), terrainSize.y, terrainSize.z / (heightfield->getRowCount()-1));

    // Create terrain
    UPtr<Terrain> terrain = create(std::move(heightfield), scale, (unsigned int)patchSize, (unsigned int)detailLevels, skirtScale, normalMap, materialPath.c_str(), pTerrain);

    if (!externalProperties)
        SAFE_DELETE(p);

    return terrain;
}

UPtr<Terrain> Terrain::create(UPtr<HeightField> heightfield, const Vector3& scale, unsigned int patchSize, unsigned int detailLevels, float skirtScale, const char* normalMapPath, const char* materialPath)
{
    return create(std::move(heightfield), scale, patchSize, detailLevels, skirtScale, normalMapPath, materialPath, NULL);
}

UPtr<Terrain> Terrain::create(UPtr<HeightField> heightfield, const Vector3& scale,
    unsigned int patchSize, unsigned int detailLevels, float skirtScale,
    const char* normalMapPath, const char* materialPath, Properties* properties)
{
    GP_ASSERT(heightfield.get());

    unsigned int width = heightfield->getColumnCount();
    unsigned int height = heightfield->getRowCount();

    // Create the terrain object
    Terrain* terrain = new Terrain();
    terrain->_heightfield = std::move(heightfield);
    terrain->_materialPath = (materialPath == NULL || strlen(materialPath) == 0) ? TERRAIN_MATERIAL : materialPath;

    // Store terrain local scaling so it can be applied to the heightfield
    terrain->_localScale.set(scale);

    // Store reference to bounding box (it is calculated and updated from TerrainPatch)
    BoundingBox& bounds = terrain->_boundingBox;

    if (normalMapPath)
    {
        terrain->_normalMap = Texture::create(normalMapPath, true).take();
        terrain->_normalMap->setWrapMode(Texture::CLAMP, Texture::CLAMP);
        GP_ASSERT( terrain->_normalMap->getType() == Texture::TEXTURE_2D );
    }

    float halfWidth = (width - 1) * 0.5f;
    float halfHeight = (height - 1) * 0.5f;

    // Compute the maximum step size, which is a function of our lowest level of detail.
    // This determines how many vertices will be skipped per triange/quad on the lowest
    // level detail terrain patch.
    //unsigned int maxStep = (unsigned int)std::pow(2.0, (double)(detailLevels-1));

    float verticalSkirtSize = skirtScale * (terrain->_heightfield->getHeightMax() - terrain->_heightfield->getHeightMin());

    // Create terrain patches
    unsigned int x1, x2, z1, z2;
    unsigned int row = 0, column = 0;
    for (unsigned int z = 0; z < height-1; z = z2, ++row)
    {
        z1 = z;
        z2 = std::min(z1 + patchSize, height-1);

        for (unsigned int x = 0; x < width-1; x = x2, ++column)
        {
            x1 = x;
            x2 = std::min(x1 + patchSize, width-1);

            // Create this patch
            TerrainPatch* patch = TerrainPatch::create(terrain, terrain->_patches.size(), row, column, 
                x1, z1, x2, z2, -halfWidth, -halfHeight, detailLevels, verticalSkirtSize);
            terrain->_patches.push_back(patch);

            // Append the new patch's local bounds to the terrain local bounds
            bounds.merge(patch->getBoundingBox(false));
        }
    }

    // Read additional layer information from properties (if specified)
    /*if (properties)
    {
        // Parse terrain layers
        Properties* lp;
        int index = -1;
        while ((lp = properties->getNextNamespace()) != NULL)
        {
            if (strcmp(lp->getNamespace(), "layer") == 0)
            {
                // If there is no explicitly specified index for this layer, assume it's the 'next' layer
                if (lp->exists("index"))
                    index = lp->getInt("index");
                else
                    ++index;

                std::string textureMap;
                const char* textureMapPtr = NULL;
                std::string blendMap;
                const char* blendMapPtr = NULL;
                Vector2 textureRepeat;
                int blendChannel = 0;
                int row = -1, column = -1;
                Vector4 temp;

                // Read layer textures
                Properties* t = lp->getNamespace("texture", true);
                if (t)
                {
                    if (t->getPath("path", &textureMap))
                    {
                        textureMapPtr = textureMap.c_str();
                    }
                    if (!t->getVector2("repeat", &textureRepeat))
                        textureRepeat.set(1,1);
                }

                Properties* b = lp->getNamespace("blend", true);
                if (b)
                {
                    if (b->getPath("path", &blendMap))
                    {
                        blendMapPtr = blendMap.c_str();
                    }
                    const char* channel = b->getString("channel");
                    if (channel && strlen(channel) > 0)
                    {
                        char c = std::toupper(channel[0]);
                        if (c == 'R' || c == '0')
                            blendChannel = 0;
                        else if (c == 'G' || c == '1')
                            blendChannel = 1;
                        else if (c == 'B' || c == '2')
                            blendChannel = 2;
                        else if (c == 'A' || c == '3')
                            blendChannel = 3;
                    }
                }

                // Get patch row/columns that this layer applies to.
                if (lp->exists("row"))
                    row = lp->getInt("row");
                if (lp->exists("column"))
                    column = lp->getInt("column");

                if (!terrain->setLayer(index, textureMapPtr, textureRepeat, blendMapPtr, blendChannel, row, column))
                {
                    GP_WARN("Failed to load terrain layer: %s", textureMap.c_str());
                }
            }
        }
    }
    */
    // Load materials for all patches
    //for (size_t i = 0, count = terrain->_patches.size(); i < count; ++i)
    //    terrain->_patches[i]->updateMaterial();

    return UPtr<Terrain>(terrain);
}

void Terrain::setNode(Node* node)
{

    if (_node != node)
    {
        if (_node)
            _node->removeListener(this);

        Drawable::setNode(node);

        if (_node)
            _node->addListener(this);

        // Update patch node bindings
        for (size_t i = 0, count = _patches.size(); i < count; ++i)
        {
            _patches[i]->updateNodeBindings();
        }
        _dirtyFlags |= DIRTY_FLAG_INVERSE_WORLD;
    }
}

void Terrain::transformChanged(Transform* transform, long cookie)
{
    _dirtyFlags |= DIRTY_FLAG_INVERSE_WORLD;
}

const Matrix& Terrain::getInverseWorldMatrix() const
{
    if (_dirtyFlags & DIRTY_FLAG_INVERSE_WORLD)
    {
        _dirtyFlags &= ~DIRTY_FLAG_INVERSE_WORLD;

        if (_node)
        {
            _inverseWorldMatrix.set(_node->getWorldMatrix());
        }
        else
        {
            _inverseWorldMatrix = Matrix::identity();
        }
        // Apply local scale and invert
        _inverseWorldMatrix.scale(_localScale);
        _inverseWorldMatrix.invert();
        
    }
    return _inverseWorldMatrix;
}

bool Terrain::addLayer(const char* texturePath, const Vector2& textureRepeat, Texture* blendPath, int blendChannel, int row, int column)
{
    if (!texturePath)
        return false;

    // Load texture sampler
    auto texture = Texture::create(texturePath, true);
    int textureIndex = addSampler(texture.get());
    if (textureIndex == -1)
        return false;

    // Load blend sampler
    int blendIndex = -1;
    if (blendPath)
    {
        blendIndex = addSampler(blendPath);
    }

    // Create the layer
    Layer* layer = new Layer();
    layer->index = _layers.size();
    layer->textureIndex = textureIndex;
    layer->textureRepeat = textureRepeat;
    layer->blendIndex = blendIndex;
    layer->blendChannel = blendChannel;

    _layers.push_back(layer);

    // Set layer on applicable patches
    setMaterialDirty();

    if (blendPath) {
        bool found = false;
        for (Texture* t : _blendTextures) {
            if (t == blendPath) {
                found = true;
                break;
            }
        }
        if (!found) {
            _blendTextures.push_back(blendPath);
        }
    }
    return true;
}

bool Terrain::isFlagSet(Flags flag) const
{
    return (_flags & flag) == flag;
}

void Terrain::setFlag(Flags flag, bool on)
{
    bool changed = false;

    if (on)
    {
        if ((_flags & flag) == 0)
        {
            _flags |= flag;
            changed = true;
        }
    }
    else
    {
        if ((_flags & flag) == flag)
        {
            _flags &= ~flag;
            changed = true;
        }
    }

    if ((flag & DEBUG_PATCHES) && changed)
    {
        // Dirty all materials since they need to be updated to support debug drawing
        for (size_t i = 0, count = _patches.size(); i < count; ++i)
        {
            _patches[i]->setMaterialDirty();
        }
    }
}

void Terrain::setMaterialDirty() {
    for (size_t i = 0, count = _patches.size(); i < count; ++i)
    {
        _patches[i]->setMaterialDirty();
    }
}

unsigned int Terrain::getPatchCount() const
{
    return _patches.size();
}

TerrainPatch* Terrain::getPatch(unsigned int index) const
{
    return _patches[index];
}

const BoundingBox& Terrain::getBoundingBox() const
{
    return _boundingBox;
}

float Terrain::getHeight(float x, float z) const
{
    // Calculate the correct x, z position relative to the heightfield data.
    float cols = _heightfield->getColumnCount();
    float rows = _heightfield->getRowCount();

    GP_ASSERT(cols > 0);
    GP_ASSERT(rows > 0);

    // Since the specified coordinates are in world space, we need to use the 
    // inverse of our world matrix to transform the world x,z coords back into
    // local heightfield coordinates for indexing into the height array.
    Vector3 v = getInverseWorldMatrix() * Vector3(x, 0.0f, z);
    x = v.x + (cols - 1) * 0.5f;
    z = v.z + (rows - 1) * 0.5f;

    // Get the unscaled height value from the HeightField
    float height = _heightfield->getHeight(x, z);

    // Apply world scale to the height value
    if (_node)
    {
        Vector3 worldScale;
        _node->getWorldMatrix().getScale(&worldScale);
        height *= worldScale.y;
    }

    // Apply local scale
    height *= _localScale.y;

    return height;
}

unsigned int Terrain::draw(RenderInfo* view)
{
    size_t visibleCount = 0;
    for (size_t i = 0, count = _patches.size(); i < count; ++i)
    {
        visibleCount += _patches[i]->draw(view);
    }
    return visibleCount;
}

void Terrain::update(float elapsedTime) {

}

void Terrain::resetMesh() {
    for (size_t i = 0, count = _patches.size(); i < count; ++i)
    {
        _patches[i]->resetMesh();
    }
}

UPtr<Drawable> Terrain::clone(NodeCloneContext& context)
{
    // TODO:
    return UPtr<Drawable>(NULL);
}

static float getDefaultHeight(unsigned int width, unsigned int height)
{
    // When terrain height is not specified, we'll use a default height of ~ 0.3 of the image dimensions
    return ((width + height) * 0.5f) * DEFAULT_TERRAIN_HEIGHT_RATIO;
}

int Terrain::addSampler(Texture* texture)
{
    // TODO: Support shared samplers stored in Terrain class for layers that span all patches
    // on the terrain (row == col == -1).

    // Load the texture. If this texture is already loaded, it will return
    // a pointer to the same one, with its ref count incremented.
    //Texture* texture = Texture::create(path, true).take();
    if (!texture)
        return -1;

    // Textures should only be 2D
    if (texture->getType() != Texture::TEXTURE_2D)
    {
        //SAFE_RELEASE(texture);
        return -1;
    }

    int firstAvailableIndex = -1;
    for (size_t i = 0, count = _samplers.size(); i < count; ++i)
    {
        Texture* sampler = _samplers[i];

        if (sampler == NULL && firstAvailableIndex == -1)
        {
            firstAvailableIndex = (int)i;
        }
        else if (sampler == texture)
        {
            // A sampler was already added for this texture.
            // Increase the ref count for the sampler to indicate that a new
            // layer will be referencing it.
            //texture->release();
            //sampler->addRef();
            return (int)i;
        }
    }

    // Add a new sampler to the list
    Texture* sampler = texture;// Texture::Sampler::create(texture);
    texture->addRef();
    //texture->release();

    // This may need to be clamp in some cases to prevent edge bleeding?  Possibly a
    // configuration variable in the future.
    sampler->setWrapMode(Texture::REPEAT, Texture::REPEAT);
    if (sampler->isMipmapped()) {
        sampler->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);
    }
    if (firstAvailableIndex != -1)
    {
        _samplers[firstAvailableIndex] = sampler;
        return firstAvailableIndex;
    }

    _samplers.push_back(sampler);
    return (int)(_samplers.size() - 1);
}

Terrain::Layer::Layer() :
    index(0), row(-1), column(-1), textureIndex(-1), blendIndex(-1)
{
}

Terrain::Layer::~Layer()
{
}


static void calculateNormal(
    float x1, float y1, float z1,
    float x2, float y2, float z2,
    float x3, float y3, float z3,
    Vector3* normal)
{
    Vector3 E(x1, y1, z1);
    Vector3 F(x2, y2, z2);
    Vector3 G(x3, y3, z3);

    Vector3 P, Q;
    Vector3::subtract(F, E, &P);
    Vector3::subtract(G, E, &Q);

    Vector3::cross(Q, P, normal);
}

static float normalizedHeightPacked(float r, float g, float b)
{
    // This formula is intended for 24-bit packed heightmap images (that are generated
    // with gameplay-encoder. However, it is also compatible with normal grayscale 
    // heightmap images, with an error of approximately 0.4%. This can be seen by
    // setting r=g=b=x and comparing the grayscale height expression to the packed
    // height expression: the error is 2^-8 + 2^-16 which is just under 0.4%.
    return (256.0f*r + g + 0.00390625f*b) / 65536.0f;
}

void Terrain::generateNormalMap()
{
    // Load the input heightmap
    int _resolutionX = _heightfield->getColumnCount();
    int _resolutionY = _heightfield->getRowCount();
    ///////////////////////////////////////////////////////////////////////////////////////////////
    //
    // NOTE: This method assumes the heightmap geometry is generated as follows.
    //
    //   -----------
    //  | / | / | / |
    //  |-----------|
    //  | / | / | / |
    //  |-----------|
    //  | / | / | / |
    //   -----------
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////

    struct NormalPixel
    {
        unsigned char r, g, b, a;
    };
    NormalPixel* normalPixels = new NormalPixel[_resolutionX * _resolutionY];

    struct Face
    {
        Vector3 normal1;
        Vector3 normal2;
    };

    int progressMax = (_resolutionX-1) * (_resolutionY-1) + _resolutionX * _resolutionY;
    int progress = 0;

    Vector2 scale(_localScale.x, _localScale.z);

    // First calculate all face normals for the heightmap
    //LOG(1, "Calculating normals... 0%%");
    Face* faceNormals = new Face[(_resolutionX - 1) * (_resolutionY - 1)];
    Vector3 v1, v2;
    for (int z = 0; z < _resolutionY-1; z++)
    {
        for (int x = 0; x < _resolutionX-1; x++)
        {
            float topLeftHeight = _heightfield->getHeight(x, z);
            float bottomLeftHeight = _heightfield->getHeight(x, z + 1);
            float bottomRightHeight = _heightfield->getHeight(x + 1, z + 1);
            float topRightHeight = _heightfield->getHeight(x + 1, z);

            // Triangle 1
            calculateNormal(
                (float)x*scale.x, bottomLeftHeight, (float)(z + 1)*scale.y,
                (float)x*scale.x, topLeftHeight, (float)z*scale.y,
                (float)(x + 1)*scale.x, topRightHeight, (float)z*scale.y,
                &faceNormals[z*(_resolutionX-1)+x].normal1);

            // Triangle 2
            calculateNormal(
                (float)x*scale.x, bottomLeftHeight, (float)(z + 1)*scale.y,
                (float)(x + 1)*scale.x, topRightHeight, (float)z*scale.y,
                (float)(x + 1)*scale.x, bottomRightHeight, (float)(z + 1)*scale.y,
                &faceNormals[z*(_resolutionX-1)+x].normal2);

            ++progress;
            //LOG(1, "\rCalculating normals... %d%%", (int)(((float)progress / progressMax) * 100));
        }
    }

    // Smooth normals by taking an average for each vertex
    Vector3 normal;
    for (int z = 0; z < _resolutionY; z++)
    {
        for (int x = 0; x < _resolutionX; x++)
        {
            // Reset normal sum
            normal.set(0, 0, 0);

            if (x > 0)
            {
                if (z > 0)
                {
                    // Top left
                    normal.add(faceNormals[(z-1)*(_resolutionX-1) + (x-1)].normal2);
                }

                if (z < (_resolutionY - 1))
                {
                    // Bottom left
                    normal.add(faceNormals[z*(_resolutionX-1) + (x - 1)].normal1);
                    normal.add(faceNormals[z*(_resolutionX-1) + (x - 1)].normal2);
                }
            }

            if (x < (_resolutionX - 1))
            {
                if (z > 0)
                {
                    // Top right
                    normal.add(faceNormals[(z-1)*(_resolutionX-1) + x].normal1);
                    normal.add(faceNormals[(z-1)*(_resolutionX-1) + x].normal2);
                }

                if (z < (_resolutionY - 1))
                {
                    // Bottom right
                    normal.add(faceNormals[z*(_resolutionX-1) + x].normal1);
                }
            }

            // We don't have to worry about weighting the normals by
            // the surface area of the triangles since a heightmap 
            // guarantees that all triangles have the same surface area.
            normal.normalize();

            // Store this vertex normal
            NormalPixel& pixel = normalPixels[z*_resolutionX + x];
            pixel.r = (unsigned char)((normal.x + 1.0f) * 0.5f * 255.0f);
            pixel.g = (unsigned char)((normal.y + 1.0f) * 0.5f * 255.0f);
            pixel.b = (unsigned char)((normal.z + 1.0f) * 0.5f * 255.0f);
            pixel.a = 1.0;

            ++progress;
            //LOG(1, "\rCalculating normals... %d%%", (int)(((float)progress / progressMax) * 100));
        }
    }

    //LOG(1, "\rCalculating normals... Done.\n");

    // Create and save an image for the normal map
    UPtr<Texture> texture = Texture::create(Texture::Format::RGBA, _resolutionX, _resolutionY, 
        (const unsigned char*)normalPixels, true);
    texture->setWrapMode(Texture::CLAMP, Texture::CLAMP);
    if (_normalMap) {
        SAFE_RELEASE(_normalMap);
    }
    _normalMap = texture.take();

    // Free temp data
    delete[] normalPixels;
    normalPixels = NULL;

    this->resetMesh();
}

}
