#include "base/Base.h"
#include "TerrainPatch.h"
#include "Terrain.h"
#include "scene/Mesh.h"
#include "scene/Scene.h"
#include "platform/Toolkit.h"
#include "material/MaterialParameter.h"
#include "scene/Renderer.h"

#include <iostream>
#include <sstream>
#include <float.h>
#include <climits>

namespace mgp
{

/**
 * @script{ignore}
 */
template <class T> T clamp(T value, T min, T max) { return value < min ? min : (value > max ? max : value); }

#define TERRAINPATCH_DIRTY_MATERIAL 1
#define TERRAINPATCH_DIRTY_BOUNDS 2
#define TERRAINPATCH_DIRTY_LEVEL 4
#define TERRAINPATCH_DIRTY_ALL (TERRAINPATCH_DIRTY_MATERIAL | TERRAINPATCH_DIRTY_BOUNDS | TERRAINPATCH_DIRTY_LEVEL)


TerrainPatch::TerrainPatch() :
    _terrain(NULL), _row(0), _column(0), _camera(NULL), _level(0), _bits(TERRAINPATCH_DIRTY_ALL)
{
}

TerrainPatch::~TerrainPatch()
{
    for (size_t i = 0, count = _levels.size(); i < count; ++i)
    {
        Level& level = _levels[i];
        if (!level.model) continue;

        SAFE_RELEASE(level.model);
    }
    
    if (_camera != NULL)
    {
    	_camera->removeListener(this);
    	SAFE_RELEASE(_camera);
    }
}

TerrainPatch* TerrainPatch::create(Terrain* terrain, unsigned int index,
                                   unsigned int row, unsigned int column,
                                   unsigned int x1, unsigned int z1, unsigned int x2, unsigned int z2,
                                   float xOffset, float zOffset,
                                   unsigned int detailLevels, float verticalSkirtSize)
{
    // Create patch
    TerrainPatch* patch = new TerrainPatch();
    patch->_terrain = terrain;
    patch->_index = index;
    patch->_row = row;
    patch->_column = column;
    patch->_detailLevels = detailLevels;
    patch->_heightfield = terrain->getHeightfield();
    patch->_x1 = x1;
    patch->_x2 = x2;
    patch->_z1 = z1;
    patch->_z2 = z2;
    patch->_xOffset = xOffset;
    patch->_zOffset = zOffset;
    patch->_verticalSkirtSize = verticalSkirtSize;

    patch->_levels.resize(detailLevels);

    patch->initLOD(detailLevels - 1);
    // Set our bounding box using the base LOD mesh
    BoundingBox& bounds = patch->_boundingBox;
    bounds.set(patch->_levels[detailLevels - 1].model->getMesh()->getBoundingBox());

    return patch;
}

void TerrainPatch::resetMesh() {
    for (size_t i = 0, count = _levels.size(); i < count; ++i)
    {
        Level& level = _levels[i];
        if (!level.model) continue;

        SAFE_RELEASE(level.model);
        //SAFE_DELETE(level);
        level.model = nullptr;
    }

    this->initLOD(this->_detailLevels - 1);
    // Set our bounding box using the base LOD mesh
    BoundingBox& bounds = this->_boundingBox;
    bounds.set(this->_levels[this->_detailLevels - 1].model->getMesh()->getBoundingBox());

    _bits = TERRAINPATCH_DIRTY_ALL;

    _hasPositinCache = false;
    _positionCache.clear();
}

unsigned int TerrainPatch::getMaterialCount() const
{
    return _levels.size();
}

Material* TerrainPatch::getMaterial(int index) const
{
    if (index == -1)
    {
        Scene* scene = _terrain->_node ? _terrain->_node->getScene() : NULL;
        Camera* camera = scene ? scene->getActiveCamera() : NULL;
        if (!camera)
        {
            _level = const_cast<TerrainPatch*>(this)->computeLOD(camera, getBoundingBox(true));
        }
        else
        {
            _level = 0;
        }
        return _levels[_level].model->getMaterial();
    }
    return _levels[index].model->getMaterial();
}

void TerrainPatch::initLOD(int dlevel)
{
    unsigned int step = pow(2, dlevel);

    float* heights = _heightfield->getArray();
    unsigned int width = _heightfield->getColumnCount();
    unsigned int height = _heightfield->getRowCount();
    unsigned int x1 = _x1;
    unsigned int z1 = _z1;
    unsigned int x2 = _x2;
    unsigned int z2 = _z2;
    float xOffset = _xOffset;
    float zOffset = _zOffset;
    float verticalSkirtSize = _verticalSkirtSize;

    // Allocate vertex data for this patch
    unsigned int patchWidth;
    unsigned int patchHeight;
    if (step == 1)
    {
        patchWidth = (x2 - x1) + 1;
        patchHeight = (z2 - z1) + 1;
    }
    else
    {
        patchWidth = (x2 - x1) / step + ((x2 - x1) %step == 0 ? 0 : 1) + 1;
        patchHeight = (z2 - z1) / step + ((z2 - z1) % step == 0 ? 0 : 1) + 1;
    }

    if (patchWidth < 2 || patchHeight < 2)
        return; // ignore this level, not enough geometry

    if (verticalSkirtSize > 0.0f)
    {
        patchWidth += 2;
        patchHeight += 2;
    }

    unsigned int vertexCount = patchHeight * patchWidth;
    unsigned int vertexElements = _terrain->_normalMap ? 5 : 8; //<x,y,z>[i,j,k]<u,v>
    float* vertices = new float[vertexCount * vertexElements];
    unsigned int index = 0;
    Vector3 min(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    float stepXScaled = step * _terrain->_localScale.x;
    float stepZScaled = step * _terrain->_localScale.z;
    bool zskirt = verticalSkirtSize > 0 ? true : false;
    for (unsigned int z = z1; ; )
    {
        bool xskirt = verticalSkirtSize > 0 ? true : false;
        for (unsigned int x = x1; ; )
        {
            GP_ASSERT(index < vertexCount);

            float* v = vertices + (index * vertexElements);
            index++;

            // Compute position - apply the local scale of the terrain into the vertex data
            v[0] = (x + xOffset) * _terrain->_localScale.x;
            v[1] = computeHeight(heights, width, x, z);
            if (xskirt || zskirt)
                v[1] -= verticalSkirtSize * _terrain->_localScale.y;
            v[2] = (z + zOffset) * _terrain->_localScale.z;

            // Update bounding box min/max (don't include vertical skirt vertices in bounding box)
            if (!(xskirt || zskirt))
            {
                if (v[0] < min.x)
                    min.x = v[0];
                if (v[1] < min.y)
                    min.y = v[1];
                if (v[2] < min.z)
                    min.z = v[2];
                if (v[0] > max.x)
                    max.x = v[0];
                if (v[1] > max.y)
                    max.y = v[1];
                if (v[2] > max.z)
                    max.z = v[2];
            }

            // Compute normal
            if (!_terrain->_normalMap)
            {
                Vector3 p(v[0], computeHeight(heights, width, x, z), v[2]);
                Vector3 w(Vector3(x>=step ? v[0]-stepXScaled : v[0], computeHeight(heights, width, x>=step ? x-step : x, z), v[2]), p);
                Vector3 e(Vector3(x<width-step ? v[0]+stepXScaled : v[0], computeHeight(heights, width, x<width-step ? x+step : x, z), v[2]), p);
                Vector3 s(Vector3(v[0], computeHeight(heights, width, x, z>=step ? z-step : z), z>=step ? v[2]-stepZScaled : v[2]), p);
                Vector3 n(Vector3(v[0], computeHeight(heights, width, x, z<height-step ? z+step : z), z<height-step ? v[2]+stepZScaled : v[2]), p);
                Vector3 normals[4];
                Vector3::cross(n, w, &normals[0]);
                Vector3::cross(w, s, &normals[1]);
                Vector3::cross(e, n, &normals[2]);
                Vector3::cross(s, e, &normals[3]);
                Vector3 normal = -(normals[0] + normals[1] + normals[2] + normals[3]);
                normal.normalize();
                v[3] = normal.x;
                v[4] = normal.y;
                v[5] = normal.z;
                v += 3;
            }

            v += 3;

            // Compute texture coord
            v[0] = (float)x / (width-1);
            v[1] = 1.0f - (float)z / (height-1);
            if (xskirt)
            {
                float offset = verticalSkirtSize / width;
                v[0] = x == x1 ? v[0]-offset : v[0]+offset;
            }
            else if (zskirt)
            {
                float offset = verticalSkirtSize / height;
                v[1] = z == z1 ? v[1]-offset : v[1]+offset;
            }

            if (x == x2)
            {
                if ((verticalSkirtSize == 0) || xskirt)
                    break;
                else
                    xskirt = true;
            }
            else if (xskirt)
            {
                xskirt = false;
            }
            else
            {
                x = std::min(x + step, x2);
            }
        }

        if (z == z2)
        {
            if ((verticalSkirtSize == 0) || zskirt)
                break;
            else
                zskirt = true;
        }
        else if (zskirt)
        {
            zskirt = false;
        }
        else
        {
            z = std::min(z + step, z2);
        }
    }
    GP_ASSERT(index == vertexCount);

    Vector3 center(min + ((max - min) * 0.5f));

    // Create mesh
    VertexFormat::Element elements[3];
    elements[0] = VertexFormat::Element(VertexFormat::POSITION, 3);
    if (_terrain->_normalMap)
    {
        elements[1] = VertexFormat::Element(VertexFormat::TEXCOORD0, 2);
    }
    else
    {
        elements[1] = VertexFormat::Element(VertexFormat::NORMAL, 3);
        elements[2] = VertexFormat::Element(VertexFormat::TEXCOORD0, 2);
    }
    VertexFormat format(elements, _terrain->_normalMap ? 2 : 3);
    UPtr<Mesh> mesh = Mesh::createMesh(format, vertexCount);
    mesh->getVertexBuffer()->setData((char*)vertices, vertexCount* vertexElements * sizeof(float));
    mesh->setBoundingBox(BoundingBox(min, max));
    mesh->setBoundingSphere(BoundingSphere(center, center.distance(max)));

    // Add mesh part for indices
    unsigned int indexCount =
        (patchWidth * 2) *      // # indices per row of tris
        (patchHeight - 1) +     // # rows of tris
        (patchHeight-2) * 2;    // # degenerate tris

    // Support a maximum number of indices of USHRT_MAX. Any more indices we will require breaking up the
    // terrain into smaller patches.
    if (indexCount > USHRT_MAX)
    {
        GP_WARN("Index count of %d for terrain patch exceeds the limit of 65535. Please specifiy a smaller patch size.", indexCount);
        GP_ASSERT(indexCount <= USHRT_MAX);
    }

    mesh->setIndex(Mesh::TRIANGLE_STRIP, indexCount);
    unsigned short* indices = new unsigned short[indexCount];
    index = 0;
    for (unsigned int z = 0; z < patchHeight-1; ++z)
    {
        unsigned int i1 = z * patchWidth;
        unsigned int i2 = (z+1) * patchWidth;

        // Move left to right for even rows and right to left for odd rows.
        // Note that this results in two degenerate triangles between rows
        // for stitching purposes, but actually does not require any extra
        // indices to achieve this.
        if (z % 2 == 0)
        {
            if (z > 0)
            {
                // Add degenerate indices to connect strips
                indices[index] = indices[index-1];
                ++index;
                indices[index++] = i1;
            }

            // Add row strip
            for (unsigned int x = 0; x < patchWidth; ++x)
            {
                indices[index++] = i1 + x;
                indices[index++] = i2 + x;
            }
        }
        else
        {
            // Add degenerate indices to connect strips
            if (z > 0)
            {
                indices[index] = indices[index-1];
                ++index;
                indices[index++] = i2 + ((int)patchWidth-1);
            }

            // Add row strip
            for (int x = (int)patchWidth-1; x >= 0; --x)
            {
                indices[index++] = i2 + x;
                indices[index++] = i1 + x;
            }
        }
    }
    GP_ASSERT(index == indexCount);
    mesh->getIndexBuffer()->setData((char*)indices, indexCount* sizeof(unsigned short));

    SAFE_DELETE_ARRAY(vertices);
    SAFE_DELETE_ARRAY(indices);

    // Create model
    UPtr<Model> model = Model::create(std::move(mesh));
    //mesh->release();
    model->setNode(_terrain->_node);
    model->setLightMask(_terrain->getLightMask());

    // Add this level
    Level level;
    level.model = model.take();
    _levels[dlevel] = level;
}

//std::string TerrainPatch::passCallback(Material *pass, void* cookie)
//{
//    TerrainPatch* patch = reinterpret_cast<TerrainPatch*>(cookie);
//    GP_ASSERT(patch);
//
//    return patch->passCreated(pass);
//}

std::string TerrainPatch::passCreated(Material* pass)
{
    // Build preprocessor string to be passed to the terrain shader.
    // NOTE: I make heavy use of preprocessor definitions, rather than passing in arrays and doing
    // non-constant array access in the shader. This is due to the fact that non-constant array access
    // in GLES is very slow on some hardware.
    std::ostringstream defines;
    defines << "NO_SPECULAR;";
    defines << "LAYER_COUNT " << _terrain->_layers.size();
    defines << ";SAMPLER_COUNT " << _terrain->_samplers.size();

    if (_terrain->isFlagSet(Terrain::DEBUG_PATCHES))
    {
        defines << ";DEBUG_PATCHES";
        pass->getParameter("u_row")->setFloat(_row);
        pass->getParameter("u_column")->setFloat(_column);
    }

    if (_terrain->_normalMap)
        defines << ";NORMAL_MAP";

    // Append texture and blend index constants to preprocessor definition.
    // We need to do this since older versions of GLSL only allow sampler arrays
    // to be indexed using constant expressions (otherwise we could simply pass an
    // array of indices to use for sampler lookup).
    //
    // Rebuild layer lists while we're at it.
    //
    int layerIndex = 0;
    for (auto itr = _terrain->_layers.begin(); itr != _terrain->_layers.end(); ++itr, ++layerIndex)
    {
        Terrain::Layer* layer = *itr;

        defines << ";TEXTURE_INDEX_" << layerIndex << " " << layer->textureIndex;
        defines << ";TEXTURE_REPEAT_" << layerIndex << " vec2(" << layer->textureRepeat.x << "," << layer->textureRepeat.y << ")";

        if (layerIndex > 0)
        {
            defines << ";BLEND_INDEX_" << layerIndex << " " << layer->blendIndex;
            defines << ";BLEND_CHANNEL_" << layerIndex << " " << layer->blendChannel;
        }
    }

    return defines.str();
}

bool TerrainPatch::updateLevelMaterial(int level) {

    if (!_levels[level].model) return false;

    UPtr<Material> material = Material::create("res/shaders/terrain.vert", "res/shaders/terrain.frag");
    GP_ASSERT(material.get());
    std::string defines = passCreated(material.get());
    material->setShaderDefines(defines);

    //material->setNodeBinding(_terrain->_node);

    if (_terrain->_layers.size() > 0) {
        MaterialParameter* parameter = material->getParameter("u_surfaceLayerMaps");
        parameter->setSamplerArray((const Texture**)&_terrain->_samplers[0], (unsigned int)_terrain->_samplers.size());
    }
    if (_terrain && _terrain->_normalMap) {
        MaterialParameter* parameter = material->getParameter("u_normalMap");
        parameter->setSampler(_terrain->_normalMap);
    }
    //TODO u_normalMatrix

    //material->getParameter("u_specularExponent")->setFloat(1.0);

    // Set material on this lod level
    _levels[level].model->setMaterial(std::move(material));
    return true;
}

bool TerrainPatch::updateMaterial()
{
    if (!(_bits & TERRAINPATCH_DIRTY_MATERIAL))
        return true;

    _bits &= ~TERRAINPATCH_DIRTY_MATERIAL;

    //__currentPatchIndex = _index;

    for (size_t i = 0, count = _levels.size(); i < count; ++i)
    {
        updateLevelMaterial(i);
    }

    //__currentPatchIndex = -1;

    return true;
}

void TerrainPatch::updateNodeBindings()
{
    //__currentPatchIndex = _index;
    for (size_t i = 0, count = _levels.size(); i < count; ++i)
    {
        if (!_levels[i].model) continue;
        _levels[i].model->setNode(_terrain->_node);
    }
    //__currentPatchIndex = -1;
}

unsigned int TerrainPatch::draw(RenderInfo* view)
{
    Scene* scene = _terrain->_node ? _terrain->_node->getScene() : NULL;
    Camera* camera = scene ? scene->getActiveCamera() : NULL;
    if (!camera)
        return 0;

    // Get our world-space bounding box
    BoundingBox bounds = getBoundingBox(true);

    // If the box does not intersect the view frustum, cull it
    if (_terrain->isFlagSet(Terrain::FRUSTUM_CULLING) && !camera->getFrustum().intersects(bounds))
        return 0;

    if (!updateMaterial())
        return 0;

    // Compute the LOD level from the camera's perspective
    _level = computeLOD(camera, bounds);

    if (!_levels[_level].model) {
        initLOD(_level);
        updateLevelMaterial(_level);
    }

    // Draw the model for the current LOD
    return _levels[_level].model->draw(view);
}

const BoundingBox& TerrainPatch::getBoundingBox(bool worldSpace) const
{
    if (!worldSpace)
        return _boundingBox;

    if (!(_bits & TERRAINPATCH_DIRTY_BOUNDS))
        return _boundingBoxWorld;

    _bits &= ~TERRAINPATCH_DIRTY_BOUNDS;

    // Apply a world-space transformation to our bounding box
    _boundingBoxWorld.set(_boundingBox);

    // Transform the bounding box by the terrain node's world transform.
    if (_terrain->_node)
        _boundingBoxWorld.transform(_terrain->_node->getWorldMatrix());

    return _boundingBoxWorld;
}

void TerrainPatch::cameraChanged(Camera* camera)
{
    _bits |= TERRAINPATCH_DIRTY_LEVEL;
}

unsigned int TerrainPatch::computeLOD(Camera* camera, const BoundingBox& worldBounds) 
{
    if (camera != _camera)
    {
        if (_camera != NULL)
        {
            _camera->removeListener(this);
            _camera->release();
        }
        _camera = camera;
        _camera->addRef();
        _camera->addListener(this);
        _bits |= TERRAINPATCH_DIRTY_LEVEL;
    }

    // base level
    if (!_terrain->isFlagSet(Terrain::LEVEL_OF_DETAIL) || _levels.size() == 0)
        return 0;

    if (!(_bits & TERRAINPATCH_DIRTY_LEVEL))
        return _level;

    _bits &= ~TERRAINPATCH_DIRTY_LEVEL;

    // Compute LOD to use based on very simple distance metric. TODO: Optimize me.
    int width = Renderer::cur()->getWidth();
    int height = Renderer::cur()->getHeight();
    Rectangle vp(0, 0, width, height);
    Vector3 corners[8];
    Vector2 min(FLT_MAX, FLT_MAX);
    Vector2 max(-FLT_MAX, -FLT_MAX);
    worldBounds.getCorners(corners);
    for (unsigned int i = 0; i < 8; ++i)
    {
        const Vector3& corner = corners[i];
        float x, y;
        camera->project(vp, corners[i], &x, &y);
        if (x < min.x)
            min.x = x;
        if (y < min.y)
            min.y = y;
        if (x > max.x)
            max.x = x;
        if (y > max.y)
            max.y = y;
    }
    float area = (max.x - min.x) * (max.y - min.y);
    float screenArea = width * height / 10.0f;
    float error = screenArea / area;

    // Level LOD based on distance from camera
    size_t maxLod = _levels.size()-1;
    size_t lod = (size_t)error;
    lod = std::max(lod, (size_t)0);
    lod = std::min(lod, maxLod);
    _level = lod;

    return _level;
}

const Vector3& TerrainPatch::getAmbientColor() const
{
    Scene* scene = _terrain->_node ? _terrain->_node->getScene() : NULL;
    return scene ? scene->getAmbientColor() : Vector3::zero();
}

void TerrainPatch::setMaterialDirty()
{
    _bits |= TERRAINPATCH_DIRTY_MATERIAL;
}

float TerrainPatch::computeHeight(float* heights, unsigned int width, unsigned int x, unsigned int z)
{
    return heights[z * width + x] * _terrain->_localScale.y;
}

TerrainPatch::Level::Level() : model(NULL)
{
}

void TerrainPatch::genLayerVertex(std::vector<float>& position, int layerIndex, int random, float randomRange) {
    if (_level == 0 && _levels[_level].model) {
        if (_hasPositinCache) {
            position.insert(position.end(), _positionCache.begin(), _positionCache.end());
            return;
        }

        auto layer =_terrain->_layers.at(layerIndex);
        Texture* blendTexture = _terrain->_samplers[layer->blendIndex];
        int bbp = Image::getFormatBPP(blendTexture->getFormat());
        unsigned char* blendData = (unsigned char*)blendTexture->lock();
        int channel = layer->blendChannel;

        Mesh* mesh = _levels[_level].model->getMesh();
        const VertexFormat& format = mesh->getVertexFormat();
        const VertexFormat::Element posAttr = format.getElement(0);
        const VertexFormat::Element uvAttr = format.getElement(format.getElementCount()-1);
        const float* buffer = (float*)mesh->getVertexBuffer()->_data;
        int n = mesh->getVertexCount();
        int vertexSize = format.getVertexSize();

        int limit = 128;
        for (int i = 0; i < n; ++i) {
            int p2 = i * uvAttr.stride + uvAttr.offset;
            p2 /= 4;
            float u = buffer[p2];
            float v = 1.0f- buffer[p2 + 1];
            u = MATH_CLAMP(u, 0, 1.0f);
            v = MATH_CLAMP(v, 0, 1.0f);

            int tx = u * (blendTexture->getWidth()-1);
            int ty = v * (blendTexture->getHeight()-1);
            unsigned char value = blendData[(tx + ty * blendTexture->getWidth()) * bbp + channel];
            if (value > limit) {
                int p1 = i * posAttr.stride + posAttr.offset;
                p1 /= 4;
                float x = buffer[p1];
                float y = buffer[p1 + 1];
                float z = buffer[p1 + 2];

                if (random) {
                    int randomCount = ((value- limit) / (255.0- limit)) * random;
                    for (int k = 0; k < randomCount; ++k) {
                        float r = ((rand() / (float)RAND_MAX) - 0.5) * randomRange;
                        float r2 = ((rand() / (float)RAND_MAX) - 0.5) * randomRange;
                        float h = _terrain->getHeight(x + r, z + r2);
                        _positionCache.push_back(x+r);
                        _positionCache.push_back(h);
                        _positionCache.push_back(z+r2);
                    }
                }
                else {
                    _positionCache.push_back(x);
                    _positionCache.push_back(y);
                    _positionCache.push_back(z);
                }
            }
        }

        _hasPositinCache = true;
        position.insert(position.end(), _positionCache.begin(), _positionCache.end());
    }
}

}
