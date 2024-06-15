#include "base/Base.h"
#include "Drawable.h"
#include "scene/Node.h"
#include "Renderer.h"

using namespace mgp;

void RenderInfo::draw(DrawCall* drawCall) {
    _drawList.emplace_back(*drawCall);
}

Drawable::Drawable()
    : _renderLayer(RenderLayer::Qpaque), _lightMask(0), _visiable(true), _pickMask(1), _highlightType(HighlightType::Silhouette)
{
}

Drawable::~Drawable()
{
}

Node* Drawable::getNode() const
{
    return _node;
}

void mgp::Drawable::copyFrom(Drawable* drawable)
{
    _renderLayer = drawable->_renderLayer;
    _lightMask = drawable->_lightMask;
    _visiable = drawable->_visiable;
    _pickMask = drawable->_pickMask;
    _highlightType = drawable->_highlightType;
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
            query.id = localQuery.id;
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
