#include "base/Base.h"
#include "Drawable.h"
#include "scene/Node.h"
#include "Renderer.h"

using namespace mgp;

void RenderInfo::draw(DrawCall* drawCall) {
    _drawList.emplace_back(*drawCall);
}

Drawable::Drawable()
    : _node(NULL), _renderPass(RenderLayer::Qpaque), _lightMask(0), _visiable(true), _pickMask(1), _highlightType(HighlightType::SharedColor), _instanceKey(NULL)
{
}

Drawable::~Drawable()
{
}

Node* Drawable::getNode() const
{
    return _node;
}

void Drawable::setNode(Node* node)
{
    _node = node;
}

bool Drawable::doRaycast(RayQuery& query) {
    return false;
}
bool Drawable::raycast(RayQuery& query) {
    if (!isVisiable()) return Ray::INTERSECTS_NONE;
    if ((_pickMask & query.pickMask) == 0) {
        return Ray::INTERSECTS_NONE;
    }

    if (_node) {
        auto sphere = _node->getBoundingSphere();
        if (sphere.intersectsQuery(query.ray) == Ray::INTERSECTS_NONE) {
            return false;
        }
    }
    Matrix matrix;
    if (_node) {
        matrix = _node->getWorldMatrix();
        matrix.invert();
    }

    RayQuery localQuery = query;
    localQuery.ray.transform(matrix);

    if (doRaycast(localQuery)) {
        _node->getWorldMatrix().transformPoint(&localQuery.target);
        double distance = localQuery.target.distance(query.ray.getOrigin());
        if (query.minDistance == Ray::INTERSECTS_NONE || distance < query.minDistance) {
            query.minDistance = distance;
            query.target = localQuery.target;
            query.path.swap(localQuery.path);
            query.drawable = this;
        }
        return true;
    }
    return false;
}

double Drawable::getDistance(Vector3& cameraPosition) const {
    if (!_node) return 0;
    const BoundingSphere& sphereA = _node->getBoundingSphere();
    double distance = cameraPosition.distanceSquared(sphereA.center);
    return distance;
}

DelayUpdater::DelayUpdater(): maxUpdateDelay(500), viewDirtyTime(0) {

}

void DelayUpdater::setMaxUpdateDelay(uint64_t time) {
    maxUpdateDelay = time;
}

void DelayUpdater::setDirty() {
    if (viewDirtyTime == 0) viewDirtyTime = System::millisTicks();
}

bool DelayUpdater::needUpdate(const Matrix& viewMatrix) {
    uint64_t now = System::millisTicks();
    if (viewDirtyTime > 0 && now - viewDirtyTime > maxUpdateDelay) {
        lastViewMatrix = viewMatrix;
        viewDirtyTime = 0;
        return true;
    }
    else if (viewDirtyTime == 0) {
        if (lastViewMatrix != viewMatrix) {
            viewDirtyTime = now;
        }
    }
    return false;
}

mgp::DrawableGroup::DrawableGroup()
{
}

mgp::DrawableGroup::~DrawableGroup()
{
}

unsigned int mgp::DrawableGroup::draw(RenderInfo* view)
{
    unsigned int a = 0;
    for (UPtr<Drawable>& d : _drawables) {
        a += d->draw(view);
    }
    return a;
}

void mgp::DrawableGroup::update(float elapsedTime)
{
    for (UPtr<Drawable>& d : _drawables) {
        d->setNode(getNode());
        d->update(elapsedTime);
    }
}

bool mgp::DrawableGroup::doRaycast(RayQuery& query)
{
    bool res = false;
    for (UPtr<Drawable>& d : _drawables) {
        if (d->raycast(query)) res = true;
    }
    return res;
}

const BoundingSphere* mgp::DrawableGroup::getBoundingSphere()
{
    bool empty = true;
    for (UPtr<Drawable>& d : _drawables) {
        auto sphere = d->getBoundingSphere();
        if (!sphere) continue;
        if (empty)
        {
            _bounds.set(*sphere);
            empty = false;
        }
        else
        {
            _bounds.merge(*sphere);
        }
    }
    return &_bounds;
}

UPtr<Drawable> mgp::DrawableGroup::clone(NodeCloneContext& context) {
    UPtr<DrawableGroup> ng(new DrawableGroup());
    ng->_bounds = _bounds;

    NodeCloneContext ctx;
    for (UPtr<Drawable>& d : _drawables) {
        auto c = d->clone(ctx);
        if (c.get()) {
            ng->_drawables.push_back(std::move(c));
        }
    }

    return ng;
}

