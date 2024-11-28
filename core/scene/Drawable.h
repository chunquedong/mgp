#ifndef DRAWABLE_H_
#define DRAWABLE_H_
#include "scene/Component.h"
#include "Camera.h"
#include "math/BoundingSphere.h"

namespace mgp
{

class Node;
class NodeCloneContext;
class Light;
class Material;
class Drawable;
class DrawCall;

/**
* render context
*/
class RenderInfo {
public:
    std::vector<DrawCall> _drawList;

    //std::vector<Light*>* lights = NULL;
    Camera* camera = NULL;
    Rectangle viewport;

    bool wireframe = false;
    bool isDepthPass = false;

    virtual void draw(DrawCall* drawCall);
};

struct RayQuery {
    Ray ray;
    bool backfaceCulling = true;
    bool autoCullFace = true;
    bool getNormal = false;
    int pickMask = 1;

    //camera fovDivisor = tan(camera.fov*0.5)/(viewport.h/2);
    double fovDivisor = 1.0/1024.0;
    double tolerance = 10;

    //-------pick result
    /**
    * intersection point
    */
    Vector3 target;

    /**
    * intersection point normal direction
    */
    Vector3 normal;

    /**
    * element index: [partIndex/batchIndex,triangleIndex]
    */
    std::vector<int> path;

    /**
    * element id; invalid for MeshBatch
    */
    int id = -1;
    /**
    * min distance to ray origin
    */
    double minDistance = Ray::INTERSECTS_NONE;
    /**
    *pick object
    */
    Drawable* drawable = NULL;
};

/**
 * Defines a drawable object that can be attached to a Node.
 */
class Drawable : public Component, public Refable
{
    friend class Node;
    friend class DrawableGroup;

public:

    // Render queue indexes (in order of drawing).
    enum RenderLayer
    {
        Qpaque = 0,
        Custom,
        Transparent,
        Overlay,
        Count
    };

    /**
     * Constructor.
     */
    Drawable();

    /**
     * Destructor.
     */
    virtual ~Drawable();

    /**
     * Called to update the state.
     *
     * @param elapsedTime Elapsed time in milliseconds.
     */
    virtual void update(float elapsedTime) {}

    /**
     * Draws the object.
     *
     * @param wireframe true if you want to request to draw the wireframe only.
     * @return The number of graphics draw calls required to draw the object.
     */

    virtual unsigned int draw(RenderInfo *view) = 0;

    /**
     * Gets the node this drawable is attached to.
     *
     * @return The node this drawable is attached to.
     */
    Node* getNode() const;

    /**
    * @see AnimationTarget::getAnimation
    */
    virtual Animation* getAnimation(const char* id = NULL) const { return NULL; };

    RenderLayer getRenderLayer() const { return _renderLayer; }
    void setRenderLayer(RenderLayer p) { _renderLayer = p; }

    int getLightMask() const { return _lightMask; }
    void setLightMask(int mask) { _lightMask = mask; }

    bool isVisiable() { return _visiable; }
    void setVisiable(bool v) { _visiable = v; }

    int getPickMask() { return _pickMask; }
    void setPickMask(int v) { _pickMask = v; }

    enum HighlightType
    {
        SharedColor, No, Silhouette, AloneColor,
    };
    HighlightType getHighlightType() { return _highlightType; }
    void setHighlightType(HighlightType v) { _highlightType = v; }

    virtual bool raycast(RayQuery& query);
    virtual bool doRaycast(RayQuery& query);

    virtual const BoundingSphere* getBoundingSphere() { return NULL; }

    virtual Material* getMainMaterial() const { return NULL; };

    virtual double getDistance(Vector3& cameraPosition) const;

    /**
     * Clones the drawable and returns a new drawable.
     *
     * @param context The clone context.
     * @return The newly created drawable.
     */
    virtual UPtr<Drawable> clone(NodeCloneContext& context) { return UPtr<Drawable>(NULL); }
protected:

    void copyFrom(Drawable* drawable);
    /**
     * Sets the node this drawable is attached to.
     *
     * @param node The node this drawable is attached to.
     */
    virtual void setNode(Node* node);

    /**
     * Node this drawable is attached to.
     */
    //Node* _node;

    RenderLayer _renderLayer;

    int _lightMask;

    bool _visiable;

    int _pickMask;

    HighlightType _highlightType;
};

class DelayUpdater {
    uint64_t maxUpdateDelay;
    uint64_t viewDirtyTime;
    Matrix lastViewMatrix;

public:
    DelayUpdater();
    void setMaxUpdateDelay(uint64_t time);
    void setDirty();
    bool needUpdate(const Matrix& viewMatrix);
};

}

#endif
