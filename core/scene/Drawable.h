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

    std::vector<Light*>* lights = NULL;
    Camera* camera = NULL;
    Rectangle viewport;

    Material* _overridedMaterial = NULL;
    int _overridedDepthState = 0;

    bool wireframe = false;
    bool isDepthPass = false;


    virtual void draw(DrawCall* drawCall);
};

struct RayQuery {
    Ray ray;
    bool backfaceCulling = true;
    
    Vector3 target;
    std::vector<int> path;
    double minDistance = Ray::INTERSECTS_NONE;
    Drawable* drawable;
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

    RenderLayer getRenderPass() const { return _renderPass; }
    void setRenderPass(RenderLayer p) { _renderPass = p; }

    int getLightMask() const { return _lightMask; }
    void setLightMask(int mask) { _lightMask = mask; }

    bool isVisiable() { return _visiable; }
    void setVisiable(bool v) { _visiable = v; }

    bool isClickable() { return _clickable; }
    void setClickable(bool v) { _clickable = v; }

    bool raycast(RayQuery& query);
    virtual bool doRaycast(RayQuery& query);

    virtual const BoundingSphere* getBoundingSphere() { return NULL; }

    virtual Material* getMainMaterial() const { return NULL; };

    virtual double getDistance(Vector3& cameraPosition) const;
protected:

    /**
     * Clones the drawable and returns a new drawable.
     *
     * @param context The clone context.
     * @return The newly created drawable.
     */
    virtual UPtr<Drawable> clone(NodeCloneContext& context) { return UPtr<Drawable>(NULL); }

    /**
     * Sets the node this drawable is attached to.
     *
     * @param node The node this drawable is attached to.
     */
    virtual void setNode(Node* node);

    /**
     * Node this drawable is attached to.
     */
    Node* _node;

    RenderLayer _renderPass;

    int _lightMask;

    bool _visiable;

    bool _clickable;
};

class DrawableGroup : public Drawable {
    std::vector<UPtr<Drawable> > _drawables;
    BoundingSphere _bounds;
public:
    DrawableGroup();
    ~DrawableGroup();
    std::vector<UPtr<Drawable> >& getDrawables() { return _drawables; }

    unsigned int draw(RenderInfo* view) override;
    virtual void update(float elapsedTime) override;
    virtual bool doRaycast(RayQuery& query) override;
    virtual const BoundingSphere* getBoundingSphere() override;
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
