#include "base/Base.h"
#include "Terrain.h"
#include "TerrainPatch.h"
#include "scene/Node.h"
#include "base/FileSystem.h"
#include "base/Resource.h"
#include "scene/AssetManager.h"
#include "base/StringUtil.h"

namespace mgp
{

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

void Terrain::initPatchs() {
    Terrain* terrain = this;
    // Store reference to bounding box (it is calculated and updated from TerrainPatch)
    BoundingBox& bounds = terrain->_boundingBox;

    unsigned int width = _heightfield->getColumnCount();
    unsigned int height = _heightfield->getRowCount();

    float halfWidth = (width - 1) * 0.5f;
    float halfHeight = (height - 1) * 0.5f;
    float verticalSkirtSize = _skirtScale * (terrain->_heightfield->getHeightMax() - terrain->_heightfield->getHeightMin());

    // Create terrain patches
    unsigned int x1, x2, z1, z2;
    unsigned int row = 0, column = 0;
    for (unsigned int z = 0; z < height - 1; z = z2, ++row)
    {
        z1 = z;
        z2 = std::min(z1 + _patchSize, height - 1);

        column = 0;
        for (unsigned int x = 0; x < width - 1; x = x2, ++column)
        {
            x1 = x;
            x2 = std::min(x1 + _patchSize, width - 1);

            // Create this patch
            TerrainPatch* patch = TerrainPatch::create(terrain, terrain->_patches.size(), row, column,
                x1, z1, x2, z2, -halfWidth, -halfHeight, _detailLevels, verticalSkirtSize);
            terrain->_patches.push_back(patch);

            // Append the new patch's local bounds to the terrain local bounds
            bounds.merge(patch->getBoundingBox(false));
        }
    }
}

UPtr<Terrain> Terrain::create(UPtr<HeightField> heightfield, const Vector3& scale,
    unsigned int patchSize, unsigned int detailLevels, float skirtScale,
    const char* normalMapPath)
{
    GP_ASSERT(heightfield.get());

    // Create the terrain object
    Terrain* terrain = new Terrain();
    terrain->_heightfield = std::move(heightfield);
    //terrain->_materialPath = (materialPath == NULL || strlen(materialPath) == 0) ? TERRAIN_MATERIAL : materialPath;

    // Store terrain local scaling so it can be applied to the heightfield
    terrain->_localScale.set(scale);
    terrain->_patchSize = patchSize;
    terrain->_detailLevels = detailLevels;
    terrain->_skirtScale = skirtScale;

    if (normalMapPath)
    {
        terrain->_normalMap = Texture::create(normalMapPath, true).take();
        terrain->_normalMap->setWrapMode(Texture::CLAMP, Texture::CLAMP);
        GP_ASSERT( terrain->_normalMap->getType() == Texture::TEXTURE_2D );
    }

    terrain->initPatchs();

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
    //layer->index = _layers.size();
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
    textureIndex(-1), blendIndex(-1)
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
    UPtr<Texture> texture = Texture::create(Image::Format::RGBA, _resolutionX, _resolutionY, 
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

Serializable* Terrain::createLayerObject() {
    return new Terrain::Layer();
}

std::string Terrain::Layer::getClassName() {
    return "mgp::Terrain::Layer";
}

void Terrain::Layer::onSerialize(Serializer* serializer) {
    serializer->writeInt("textureIndex", textureIndex, -1);
    serializer->writeInt("blendIndex", blendIndex, -1);
    serializer->writeInt("blendChannel", blendChannel, -1);
    serializer->writeVector("textureRepeat", textureRepeat, Vector2::one());
}

void Terrain::Layer::onDeserialize(Serializer* serializer) {
    textureIndex = serializer->readInt("textureIndex", -1);
    blendIndex = serializer->readInt("blendIndex", -1);
    blendChannel = serializer->readInt("blendChannel", -1);
    textureRepeat = serializer->readVector("textureRepeat", Vector2::one());
}

Serializable* Terrain::createObject() {
    return new Terrain();
}

std::string Terrain::getClassName() {
    return "mgp::Terrain";
}

void Terrain::onSerialize(Serializer* serializer) {
    serializer->writeInt("renderLayer", this->getRenderLayer(), 0);
    serializer->writeInt("lightMask", this->getLightMask(), 0);

    serializer->writeInt("patchSize", _patchSize, -1);
    serializer->writeInt("detailLevels", _detailLevels, -1);
    serializer->writeFloat("skirtScale", _skirtScale, 0);
    serializer->writeVector("localScale", _localScale, Vector3::one());

    serializer->writeInt("heightfield_row", _heightfield->getRowCount(), 0);
    serializer->writeInt("heightfield_column", _heightfield->getColumnCount(), 0);
    serializer->writeFloat("heightfield_min", _heightfield->getHeightMin(), 0);
    serializer->writeFloat("heightfield_max", _heightfield->getHeightMax(), 0);
    
    if (_heightfield->getPath().size() == 0) {
        _heightfield->getPath() = "image/" + Resource::genId() + ".raw";
        std::string file = AssetManager::getInstance()->getPath() + "/" + _heightfield->getPath();
        _heightfield->save(file.c_str());
    }
    serializer->writeString("heightfield_path", _heightfield->getPath().c_str(), "");

    serializer->writeObject("normalMap", _normalMap);

    serializer->writeList("samplers", _samplers.size());
    for (Texture* t : _samplers) {
        serializer->writeObject(NULL, t);
    }
    serializer->finishColloction();

    serializer->writeList("layers", _layers.size());
    for (Layer* t : _layers) {
        serializer->writeObject(NULL, t);
    }
    serializer->finishColloction();
}

void Terrain::onDeserialize(Serializer* serializer) {
    setRenderLayer((Drawable::RenderLayer)serializer->readInt("renderLayer", 0));
    setLightMask(serializer->readInt("lightMask", 0));

    _patchSize = serializer->readInt("patchSize", -1);
    _detailLevels = serializer->readInt("detailLevels", -1);
    _skirtScale = serializer->readFloat("skirtScale", 0);
    _localScale = serializer->readVector("localScale", Vector3::one());

    int heightfield_row = serializer->readInt("heightfield_row", 0);
    int heightfield_column = serializer->readInt("heightfield_column", 0);
    float heightfield_min = serializer->readFloat("heightfield_min", 0);
    float heightfield_max = serializer->readFloat("heightfield_max", 0);
    std::string heightfield_path;
    serializer->readString("heightfield_path", heightfield_path, "");
    if (StringUtil::startsWith(heightfield_path, "image/")) {
        heightfield_path = AssetManager::getInstance()->getPath() + "/" + heightfield_path;
    }
    _heightfield = HeightField::createFromRAW(heightfield_path.c_str(), heightfield_row, heightfield_column, heightfield_min, heightfield_max);

    auto normalMap = serializer->readObject("normalMap");
    if (normalMap.get()) {
        _normalMap = normalMap.dynamicCastTo<Texture>().take();
    }

    int n = serializer->readList("samplers");
    for (int i = 0; i < n; ++i) {
        Texture * t = serializer->readObject(NULL).dynamicCastTo<Texture>().take();
        _samplers.push_back(t);
    }
    serializer->finishColloction();

    n = serializer->readList("layers");
    for (int i = 0; i < n; ++i) {
        Layer* t = serializer->readObject(NULL).dynamicCastTo<Layer>().take();
        _layers.push_back(t);
    }
    serializer->finishColloction();

    this->initPatchs();
}

}
