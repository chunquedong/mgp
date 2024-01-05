#include "base/Base.h"
#include "SpriteBatch.h"
#include "platform/Toolkit.h"
#include "material/Material.h"
#include "material/MaterialParameter.h"

// Default size of a newly created sprite batch
#define SPRITE_BATCH_DEFAULT_SIZE 128

// Factor to grow a sprite batch by when its size is exceeded
#define SPRITE_BATCH_GROW_FACTOR 2.0f

// Macro for adding a sprite to the batch
#define SPRITE_ADD_VERTEX(vtx, vx, vy, vz, vu, vv, vr, vg, vb, va) \
    vtx.x = vx; vtx.y = vy; vtx.z = vz; \
    vtx.u = vu; vtx.v = vv; \
    vtx.r = vr; vtx.g = vg; vtx.b = vb; vtx.a = va

// Default sprite shaders
#define SPRITE_VSH "res/shaders/sprite.vert"
#define SPRITE_FSH "res/shaders/sprite.frag"

namespace mgp
{

static ShaderProgram* __spriteEffect = NULL;

SpriteBatch::SpriteBatch()
    : _batch(NULL), _sampler(NULL), _textureWidthRatio(0.0f), _textureHeightRatio(0.0f)
{
}

SpriteBatch::~SpriteBatch()
{
    SAFE_RELEASE(_batch);
    SAFE_RELEASE(_sampler);
    if (!_customEffect)
    {
        if (__spriteEffect && __spriteEffect->getRefCount() == 1)
        {
            __spriteEffect->release();
            __spriteEffect = NULL;
        }
        else
        {
            __spriteEffect->release();
        }
    }
}

UPtr<SpriteBatch> SpriteBatch::create(const char* texturePath, ShaderProgram* effect, unsigned int initialCapacity)
{
    UPtr<Texture> texture = Texture::create(texturePath);
    UPtr<SpriteBatch> batch = SpriteBatch::create(texture.get(), effect, initialCapacity);
    //SAFE_RELEASE(texture);
    return batch;
}

UPtr<SpriteBatch> SpriteBatch::createColord(ShaderProgram* effect, unsigned int initialCapacity) {
    // Wrap the effect in a material
    UPtr<Material> material = Material::create("res/shaders/colored.vert", "res/shaders/colored.frag", "VERTEX_COLOR");

    // Set initial material state
    material->getStateBlock()->setBlend(true);
    material->getStateBlock()->setBlendSrc(StateBlock::BLEND_SRC_ALPHA);
    material->getStateBlock()->setBlendDst(StateBlock::BLEND_ONE_MINUS_SRC_ALPHA);
    material->getStateBlock()->setDepthTest(false);
    material->getStateBlock()->setCullFace(false);

    // Define the vertex format for the batch
    VertexFormat::Element vertexElements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::TEXCOORD0, 2),
        VertexFormat::Element(VertexFormat::COLOR, 4)
    };
    VertexFormat vertexFormat(vertexElements, 3);

    // Create the mesh batch
    UPtr<MeshBatch> meshBatch = MeshBatch::create(vertexFormat, Mesh::TRIANGLE_STRIP, std::move(material), Mesh::INDEX16, initialCapacity > 0 ? initialCapacity : SPRITE_BATCH_DEFAULT_SIZE);
    //material->release(); // don't call SAFE_RELEASE since material is used below

    // Create the batch
    SpriteBatch* batch = new SpriteBatch();
    batch->_sampler = NULL;
    batch->_customEffect = true;
    batch->_batch = meshBatch.take();
    batch->_textureWidthRatio = 1.0f;
    batch->_textureHeightRatio = 1.0f;

    // Bind an ortho projection to the material by default (user can override with setProjectionMatrix)
    Toolkit* game = Toolkit::cur();
    Matrix::createOrthographicOffCenter(0, game->getDpWidth(), game->getDpHeight(), 0, 0, 1, &batch->_projectionMatrix);
    meshBatch->getMaterial()->getParameter("u_projectionMatrix")->bindValue(batch, &SpriteBatch::getProjectionMatrix);

    return UPtr<SpriteBatch>(batch);
}

UPtr<SpriteBatch> SpriteBatch::create(Texture* texture, ShaderProgram* effect, unsigned int initialCapacity)
{
    GP_ASSERT(texture != NULL);
    GP_ASSERT(texture->getType() == Texture::TEXTURE_2D);

    bool customEffect = (effect != NULL);
    if (!customEffect)
    {
        // Create our static sprite effect.
        if (__spriteEffect == NULL)
        {
            __spriteEffect = ShaderProgram::createFromFile(SPRITE_VSH, SPRITE_FSH);
            if (__spriteEffect == NULL)
            {
                GP_ERROR("Unable to load sprite effect.");
                return UPtr<SpriteBatch>(NULL);
            }
            effect = __spriteEffect;
        }
        else
        {
            effect = __spriteEffect;
            __spriteEffect->addRef();
        }
    }

    // Search for the first sampler uniform in the effect.
    Uniform* samplerUniform = NULL;
    for (unsigned int i = 0, count = effect->getUniformCount(); i < count; ++i)
    {
        Uniform* uniform = effect->getUniform(i);
        if (uniform && uniform->isSampler2d())
        {
            samplerUniform = uniform;
            break;
        }
    }
    if (!samplerUniform)
    {
        GP_ERROR("No uniform of type GL_SAMPLER_2D found in sprite effect.");
        SAFE_RELEASE(effect);
        return UPtr<SpriteBatch>(NULL);
    }

    // Wrap the effect in a material
    UPtr<Material> material = Material::create(effect);

    // Set initial material state
    material->getStateBlock()->setBlend(true);
    material->getStateBlock()->setBlendSrc(StateBlock::BLEND_SRC_ALPHA);
    material->getStateBlock()->setBlendDst(StateBlock::BLEND_ONE_MINUS_SRC_ALPHA);
    material->getStateBlock()->setDepthTest(false);
    material->getStateBlock()->setCullFace(false);

    // Bind the texture to the material as a sampler
    //Texture::Sampler* sampler = Texture::Sampler::create(texture);
    material->getParameter(samplerUniform->getName())->setSampler(texture);
    
    // Define the vertex format for the batch
    VertexFormat::Element vertexElements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::TEXCOORD0, 2),
        VertexFormat::Element(VertexFormat::COLOR, 4)
    };
    VertexFormat vertexFormat(vertexElements, 3);

    // Create the mesh batch
    MeshBatch* meshBatch = MeshBatch::create(vertexFormat, Mesh::TRIANGLE_STRIP, std::move(material), Mesh::INDEX16, initialCapacity > 0 ? initialCapacity : SPRITE_BATCH_DEFAULT_SIZE).take();
    //material->release(); // don't call SAFE_RELEASE since material is used below

    // Create the batch
    SpriteBatch* batch = new SpriteBatch();
    texture->addRef();
    batch->_sampler = texture;
    batch->_customEffect = customEffect;
    batch->_batch = meshBatch;
    batch->_textureWidthRatio = 1.0f / (float)texture->getWidth();
    batch->_textureHeightRatio = 1.0f / (float)texture->getHeight();

	// Bind an ortho projection to the material by default (user can override with setProjectionMatrix)
	Toolkit* game = Toolkit::cur();
    Matrix::createOrthographicOffCenter(0, game->getDpWidth(), game->getDpHeight(), 0, 0, 1, &batch->_projectionMatrix);
    meshBatch->getMaterial()->getParameter("u_projectionMatrix")->bindValue(batch, &SpriteBatch::getProjectionMatrix);
	
    return UPtr<SpriteBatch>(batch);
}

void SpriteBatch::start()
{
    _batch->start();
}

bool SpriteBatch::isStarted() const
{
    return _batch->isStarted();
}

void SpriteBatch::drawImageRotated(const Vector3& dst, const Rectangle& src, const Vector2& scale, const Vector4& color,
          const Vector2& rotationPoint, float rotationAngle, bool positionIsCenter)
{
    float x = dst.x;
    float y = dst.y;
    float z = dst.z;
    float width = scale.x;
    float height = scale.y;

    // Calculate uvs.
    float u1 = _textureWidthRatio * src.x;
    float v1 = 1.0f - _textureHeightRatio * src.y;
    float u2 = u1 + _textureWidthRatio * src.width;
    float v2 = v1 - _textureHeightRatio * src.height;

    // Treat the given position as the center if the user specified it as such.
    if (positionIsCenter)
    {
        x -= 0.5f * width;
        y -= 0.5f * height;
    }

    // Expand the destination position by scale into 4 points.
    float x2 = x + width;
    float y2 = y + height;
    
    Vector2 upLeft(x, y);
    Vector2 upRight(x2, y);
    Vector2 downLeft(x, y2);
    Vector2 downRight(x2, y2);

    // Rotate points around rotationAxis by rotationAngle.
    if (rotationAngle != 0)
    {
        Vector2 pivotPoint(rotationPoint);
        pivotPoint.x *= width;
        pivotPoint.y *= height;
        pivotPoint.x += x;
        pivotPoint.y += y;
        upLeft.rotate(pivotPoint, rotationAngle);
        upRight.rotate(pivotPoint, rotationAngle);
        downLeft.rotate(pivotPoint, rotationAngle);
        downRight.rotate(pivotPoint, rotationAngle);
    }

    // Write sprite vertex data.
    static SpriteVertex v[4];
    SPRITE_ADD_VERTEX(v[0], downLeft.x, downLeft.y, z, u1, v1, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[1], upLeft.x, upLeft.y, z, u1, v2, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[2], downRight.x, downRight.y, z, u2, v1, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[3], upRight.x, upRight.y, z, u2, v2, color.x, color.y, color.z, color.w);
    
    static unsigned short indices[4] = { 0, 1, 2, 3 };

    _batch->add(v, 4, indices, 4);
}

void SpriteBatch::drawImageUpVector(const Vector3& aposition, const Vector3& right, const Vector3& forward, const Vector2& scale,
    float u1, float v1, float u2, float v2, const Vector4& color, const Vector2& rotationPoint, float rotationAngle, bool positionIsCenter)
{
    float width = scale.x;
    float height = scale.y;

    Vector3 position = aposition;
    if (positionIsCenter)
    {
        position.x -= 0.5f * width;
        position.y -= 0.5f * height;
    }

    // Calculate the vertex positions.
    Vector3 tRight(right);
    tRight *= width * 0.5f;
    Vector3 tForward(forward);
    tForward *= height * 0.5f;
    
    Vector3 p0 = position;
    p0 -= tRight;
    p0 -= tForward;

    Vector3 p1 = position;
    p1 += tRight;
    p1 -= tForward;

    tForward = forward;
    tForward *= height;
    Vector3 p2 = p0;
    p2 += tForward;
    Vector3 p3 = p1;
    p3 += tForward;

    // Calculate the rotation point.
    if (rotationAngle != 0)
    {
        Vector3 rp = p0;
        tRight = right;
        tRight *= width * rotationPoint.x;
        tForward *= rotationPoint.y;
        rp += tRight;
        rp += tForward;

        // Rotate all points the specified amount about the given point (about the up vector).
        static Vector3 u;
        Vector3::cross(right, forward, &u);
        static Matrix rotation;
        Matrix::createRotation(u, rotationAngle, &rotation);
        p0 -= rp;
        p0 *= rotation;
        p0 += rp;
        p1 -= rp;
        p1 *= rotation;
        p1 += rp;
        p2 -= rp;
        p2 *= rotation;
        p2 += rp;
        p3 -= rp;
        p3 *= rotation;
        p3 += rp;
    }


    // Add the sprite vertex data to the batch.
    static SpriteVertex v[4];
    SPRITE_ADD_VERTEX(v[0], p0.x, p0.y, p0.z, u1, v1, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[1], p1.x, p1.y, p1.z, u2, v1, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[2], p2.x, p2.y, p2.z, u1, v2, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[3], p3.x, p3.y, p3.z, u2, v2, color.x, color.y, color.z, color.w);
    
    static const unsigned short indices[4] = { 0, 1, 2, 3 };
    _batch->add(v, 4, const_cast<unsigned short*>(indices), 4);
}

void SpriteBatch::drawImage(const Rectangle& dst, const Rectangle& src, const Vector4& color, const Rectangle* clip, bool positionIsCenter)
{
    float x = dst.x;
    float y = dst.y;
    //float z = 0;
    float width = dst.width;
    float height = dst.height;

    // Calculate uvs.
    /*float u1 = _textureWidthRatio * src.x;
    float v1 = 1.0f - _textureHeightRatio * src.y;
    float u2 = u1 + _textureWidthRatio * src.width;
    float v2 = v1 - _textureHeightRatio * src.height;*/
    float u1 = _textureWidthRatio * src.x;
    float v1 = _textureHeightRatio * src.y;
    float u2 = _textureWidthRatio *  (src.x+ src.width);
    float v2 = _textureHeightRatio * (src.y+ src.height);

    draw(x, y, width, height, u1, v1, u2, v2, color, clip, positionIsCenter);
}

void SpriteBatch::draw(float x, float y, float width, float height, float u1, float v1, float u2, float v2, const Vector4& color, const Rectangle* clip, bool positionIsCenter) {
    float z = 0;

    if (clip) {
        if (!clipSprite(*clip, x, y, width, height, u1, v1, u2, v2)) {
            return;
        }
    }

    // Treat the given position as the center if the user specified it as such.
    if (positionIsCenter)
    {
        x -= 0.5f * width;
        y -= 0.5f * height;
    }

    // Write sprite vertex data.
    const float x2 = x + width;
    const float y2 = y + height;
    static SpriteVertex v[4];
    SPRITE_ADD_VERTEX(v[0], x, y, z, u1, v1, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[1], x, y2, z, u1, v2, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[2], x2, y, z, u2, v1, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[3], x2, y2, z, u2, v2, color.x, color.y, color.z, color.w);

    static unsigned short indices[4] = { 0, 1, 2, 3 };

    _batch->add(v, 4, indices, 4);
}

void SpriteBatch::drawVertices(SpriteBatch::SpriteVertex* vertices, unsigned int vertexCount, unsigned short* indices, unsigned int indexCount)
{
    GP_ASSERT(vertices);
    GP_ASSERT(indices);

    _batch->add(vertices, vertexCount, indices, indexCount);
}

void SpriteBatch::drawRect(const Rectangle& rect, const Vector4& color, const Rectangle* clip) {
    float x = rect.x;
    float y = rect.y;
    float width = rect.width;
    float height = rect.height;
    float u1 = 0;
    float v1 = 0;
    float u2 = 0;
    float v2 = 0;

    if (clip) {
        if (!clipSprite(*clip, x, y, width, height, u1, v1, u2, v2)) {
            return;
        }
    }

    const float x2 = x + width;
    const float y2 = y + height;
    static SpriteVertex v[4];
    SPRITE_ADD_VERTEX(v[0], x, y, 0, u1, v1, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[1], x, y2, 0, u1, v2, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[2], x2, y, 0, u2, v1, color.x, color.y, color.z, color.w);
    SPRITE_ADD_VERTEX(v[3], x2, y2, 0, u2, v2, color.x, color.y, color.z, color.w);

    static unsigned short indices[4] = { 0, 1, 2, 3 };

    _batch->add(v, 4, indices, 4);
}

void SpriteBatch::finish(RenderInfo* view)
{
    // Finish and draw the batch
    _batch->finish();
    _batch->draw(view, NULL);
}

StateBlock* SpriteBatch::getStateBlock() const
{
    return _batch->getMaterial()->getStateBlock();
}

Texture* SpriteBatch::getSampler() const
{
    return _sampler;
}

Material* SpriteBatch::getMaterial() const
{
    return _batch->getMaterial();
}

void SpriteBatch::setProjectionMatrix(const Matrix& matrix)
{
    _projectionMatrix = matrix;
}

const Matrix& SpriteBatch::getProjectionMatrix() const
{
    return _projectionMatrix;
}

bool SpriteBatch::clipSprite(const Rectangle& clip, float& x, float& y, float& width, float& height, float& u1, float& v1, float& u2, float& v2)
{
    // Clip the rectangle given by { x, y, width, height } into clip.
    // We need to scale the uvs accordingly as we do this.

    // First check to see if we need to draw at all.
    if (x + width < clip.x || x > clip.x + clip.width ||
        y + height < clip.y || y > clip.y + clip.height)
    {
        return false;
    }

    float uvWidth = u2 - u1;
    float uvHeight = v2 - v1;

    // Moving x to the right.
    if (x < clip.x)
    {
        const float percent = (clip.x - x) / width;
        const float dx = clip.x - x;
        const float du = uvWidth * percent;
        x = clip.x;
        width -= dx;
        u1 += du;
        uvWidth -= du;
    }

    // Moving y down.
    if (y < clip.y)
    {
        const float percent = (clip.y - y) / height;
        const float dy = clip.y - y;
        const float dv = uvHeight * percent;
        y = clip.y;
        height -= dy;
        v1 += dv;
        uvHeight -= dv;
    }

    // Moving width to the left.
    const float clipX2 = clip.x + clip.width;
    float x2 = x + width;
    if (x2 > clipX2)
    {
        const float percent = (x2 - clipX2) / width;
        width = clipX2 - x;
        u2 -= uvWidth * percent;
    }

    // Moving height up.
    const float clipY2 = clip.y + clip.height;
    float y2 = y + height;
    if (y2 > clipY2)
    {
        const float percent = (y2 - clipY2) / height;
        height = clipY2 - y;
        v2 -= uvHeight * percent;
    }

    return true;
}

}
