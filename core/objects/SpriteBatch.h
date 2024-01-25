#ifndef SPRITEBATCH_H_
#define SPRITEBATCH_H_

#include "material/Texture.h"
#include "material/ShaderProgram.h"
#include "scene/Mesh.h"
#include "math/Rectangle.h"
#include "math/Matrix.h"
#include "material/Material.h"
#include "scene/MeshBatch.h"

namespace mgp
{
/**
* use in Form for keep z order
*/
class BatchableLayer {
public:
    int zorder = 1;
    virtual void start() = 0;
    virtual void finish(RenderInfo* view) = 0;
    virtual void setProjectionMatrix(const Matrix& matrix) = 0;
    virtual bool isStarted() const = 0;
    virtual ~BatchableLayer() {}
};

/**
 * Defines a class for drawing groups of sprites.
 *
 * This class provides efficient rendering and sorting of two-dimensional
 * sprites. Only a single texture and effect can be used with a SpriteBatch.
 * This limitation promotes efficient batching by using texture atlases and
 * implicit sorting to minimize state changes. Therefore, it is highly
 * recommended to combine multiple small textures into larger texture atlases
 * where possible when drawing sprites.
 */
class SpriteBatch : public BatchableLayer
{
    friend class Bundle;
    friend class Font;
    friend class Text;

public:

    /**
     * Creates a new SpriteBatch for drawing sprites with the given texture.
     *
     * If the effect parameter is NULL, a default effect is used which
     * applies an orthographic projection for the currently bound viewport.
     * A custom projection matrix can be used with the default effect by passing
     * a new projection matrix into the SpriteBatch via the setProjectionMatrix
     * method.
     *
     * If a custom effect is specified, it must meet the following requirements:
     * <ol>
     * <li>The vertex shader inputs must include a vec3 position, a vec2 tex coord
     * and a vec4 color.
     * <li>The names of the the vertex shader inputs must match the names defined
     * by the VERTEX_ATTRIBUTE_XXX constants.
     * <li>The fragment shader must define at least a single sampler/texture uniform.
     * </ol>
     *
     * @param texturePath The path of the texture for this sprite batch.
     * @param effect An optional effect to use with the SpriteBatch.
     * @param initialCapacity An optional initial capacity of the batch (number of sprites).
     * 
     * @return A new SpriteBatch for drawing sprites using the given texture.
     * @script{create}
     */
    static UPtr<SpriteBatch> create(const char* texturePath, ShaderProgram* effect = NULL, unsigned int initialCapacity = 0);

    /**
     * Creates a new SpriteBatch for drawing sprites with the given texture.
     *
     * If the effect parameter is NULL, a default effect is used which
     * applies an orthographic projection for the currently bound viewport.
     * A custom projection matrix can be used with the default effect by passing
     * a new projection matrix into the SpriteBatch via the setProjectionMatrix
     * method.
     *
     * If a custom effect is specified, it must meet the following requirements:
     * <ol>
     * <li>The vertex shader inputs must include a vec3 position, a vec2 tex coord
     * and a vec4 color.
     * <li>The names of the the vertex shader inputs must match the names defined
     * by the VERTEX_ATTRIBUTE_XXX constants.
     * <li>The fragment shader must define at least a single sampler/texture uniform.
     * </ol>
     *
     * @param texture The texture for this sprite batch.
     * @param effect An optional effect to use with the SpriteBatch.
     * @param initialCapacity An optional initial capacity of the batch (number of sprites).
     * 
     * @return A new SpriteBatch for drawing sprites using the given texture.
     * @script{create}
     */
    static UPtr<SpriteBatch> create(Texture* texture, ShaderProgram* effect = NULL, unsigned int initialCapacity = 0);

    /**
    * Color draw without texture
    */
    static UPtr<SpriteBatch> createColord(ShaderProgram* effect = NULL, unsigned int initialCapacity = 0);

    /**
     * Destructor.
     */
    virtual ~SpriteBatch();

    /**
     * Starts drawing sprites.
     *
     * This method must be called before drawing any sprites and it must eventually be
     * followed by a call to finish().
     */
    void start();

    /**
     * Determines if the sprite batch has been started but not yet finished.
     *
     * @return True if the batch has been started and not finished.
     */
    bool isStarted() const;

    /**
     * Draws a single sprite, rotated around rotationPoint by rotationAngle.
     * 
     * @param dst The destination rectangle.
     * @param src The source rectangle.
     * @param color The color to tint the sprite. Use white for no tint.
     * @param rotationPoint The point to rotate around, relative to dst's x and y values.
     *                      (e.g. Use Vector2(0.5f, 0.5f) to rotate around the quad's center.)
     * @param rotationAngle The rotation angle in radians.
     * @param positionIsCenter Specified whether the given destination is to be the center of the sprite or not (if not, it is treated as the bottom-left).
     */
    void drawImageRotated(const Vector3& dst, const Rectangle& src, const Vector2& scale, const Vector4& color,
              const Vector2& rotationPoint, float rotationAngle, bool positionIsCenter = false);

    /**
     * Draws a single sprite, rotated about the implied up vector.
     * 
     * @param position The destination position.
     * @param right The right vector of the sprite quad (should be normalized).
     * @param forward The forward vector of the sprite quad (should be normalized).
     * @param scale The X and Y scale.
     * @param src The source rectangle.
     * @param color The color to tint the sprite. Use white for no tint.
     * @param rotationPoint The point to rotate around, relative to dst's x and y values.
     *                      (e.g. Use Vector2(0.5f, 0.5f) to rotate around the quad's center.)
     * @param rotationAngle The rotation angle in radians.
     * @param positionIsCenter Specified whether the given destination is to be the center of the sprite or not (if not, it is treated as the bottom-left).
     */
    void drawImageUpVector(const Vector3& aposition, const Vector3& right, const Vector3& forward, const Vector2& scale,
        float u1, float v1, float u2, float v2, const Vector4& color, const Vector2& rotationPoint, float rotationAngle, bool positionIsCenter = false);

    /**
     * Draws a single sprite, clipped within a rectangle.
     * 
     * @param dst The destination rectangle.
     * @param src The source rectangle.
     * @param color The color to tint the sprite. Use white for no tint.
     * @param clip The clip rectangle.
     * @param positionIsCenter Specified whether the given destination is to be the center of the sprite or not (if not, it is treated as the bottom-left).
     */
    void drawImage(const Rectangle& dst, const Rectangle& src, const Vector4& color, const Rectangle* clip = NULL, bool positionIsCenter = false);

    /**
     * Draws a single sprite, clipped within a rectangle.
     *
     * @param x The x coordinate.
     * @param y The y coordinate.
     * @param width The sprite width.
     * @param height The sprite height
     * @param u1 Texture coordinate.
     * @param v1 Texture coordinate.
     * @param u2 Texture coordinate.
     * @param v2 Texture coordinate.
     * @param color The color to tint the sprite. Use white for no tint.
     * @param clip The clip rectangle.
     */
    void draw(float x, float y, float width, float height, float u1, float v1, float u2, float v2, const Vector4& color, const Rectangle* clip = NULL, bool positionIsCenter = false);

    /**
    * draw filled Rect without texture
    */
    void drawRect(const Rectangle &rect, const Vector4& color, const Rectangle* clip = NULL);

    /**
     * Sprite vertex structure used for batching.
     */
    struct SpriteVertex
    {
        /** Vertex position x */
        float x;
        /** Vertex position y */
        float y;
        /** Vertex position z */
        float z;
        /** Vertex texture u */
        float u;
        /** Vertex texture v */
        float v;
        /** Vertex color red component */
        float r;
        /** Vertex color green component */
        float g;
        /** Vertex color blue component */
        float b;
        /** Vertex color alpha component */
        float a;
    };
    
    /**
     * Draws an array of vertices.
     *
     * This is for more advanced usage.
     *
     * @param vertices The vertices to draw.
     * @param vertexCount The number of vertices within the vertex array.
     * @param indices The vertex indices.
     * @param indexCount The number of indices within the index array.
     */
    void drawVertices(SpriteBatch::SpriteVertex* vertices, unsigned int vertexCount, unsigned short* indices, unsigned int indexCount);
    
    /**
     * Finishes sprite drawing.
     *
     * This method flushes the batch and commits rendering of all sprites that were
     * drawn since the last call to start().
     */
    void finish(RenderInfo* view);

    /**
     * Gets the texture sampler. 
     *
     * This return texture sampler is used when sampling the texture in the
     * effect. This can be modified for controlling sampler setting such as
     * filtering modes.
     */
    Texture* getSampler() const;

    /**
     * Gets the StateBlock for the SpriteBatch.
     *
     * The returned state block controls the renderer state used when drawing items
     * with this sprite batch. Modification can be made to the returned state block
     * to change how primitives are rendered.
     *
     * @return The StateBlock for this SpriteBatch.
     */
    StateBlock* getStateBlock() const;

    /**
     * Gets the material used by this batch.
     * 
     * @return The material.
     */
    Material* getMaterial() const;

    /**
     * Sets a custom projection matrix to use with the sprite batch.
     *
     * When the default effect is used with a SpriteBatch (i.e. when
     * NULL is passed into the 'effect' parameter of SpriteBatch::create),
     * this method sets a custom projection matrix to be used instead
     * of the default orthographic projection.
     *
     * @param matrix The new projection matrix to be used with the default effect.
     */
    void setProjectionMatrix(const Matrix& matrix);

    /**
     * Gets the projection matrix for the SpriteBatch.
     * 
     * @return The projection matrix.
     */
    const Matrix& getProjectionMatrix() const;

    MeshBatch* getBatch() { return _batch; }
private:

    /**
     * Constructor.
     */
    SpriteBatch();

    /**
     * Copy constructor.
     * 
     * @param copy The SpriteBatch to copy.
     */
    SpriteBatch(const SpriteBatch& copy);


    bool clipSprite(const Rectangle& clip, float& x, float& y, float& width, float& height, float& u1, float& v1, float& u2, float& v2);

    MeshBatch* _batch;
    Texture* _sampler;
    bool _customEffect;
    float _textureWidthRatio;
    float _textureHeightRatio;
    mutable Matrix _projectionMatrix;
};

}

#endif
