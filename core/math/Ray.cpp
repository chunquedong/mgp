#include "base/Base.h"
#include "Ray.h"
#include "Plane.h"
#include "Frustum.h"
#include "BoundingSphere.h"
#include "BoundingBox.h"
#include <float.h>
#include <algorithm>

namespace mgp
{
const Float Ray::INTERSECTS_NONE = FLT_MAX;

Ray::Ray()
    : _direction(0, 0, 1)
{
}

Ray::Ray(const Vector3& origin, const Vector3& direction)
{
    set(origin, direction);
}

Ray::Ray(Float originX, Float originY, Float originZ, Float dirX, Float dirY, Float dirZ)
{
    set(Vector3(originX, originY, originZ), Vector3(dirX, dirY, dirZ));
}

Ray::Ray(const Ray& copy)
{
    set(copy);
}

Ray::~Ray()
{
}

const Vector3& Ray::getOrigin() const
{
    return _origin;
}

void Ray::setOrigin(const Vector3& origin)
{
    _origin = origin;
}

void Ray::setOrigin(Float x, Float y, Float z)
{
    _origin.set(x, y, z);
}

const Vector3& Ray::getDirection() const
{
    return _direction;
}

void Ray::setDirection(const Vector3& direction)
{
    _direction = direction;
    normalize();
}

void Ray::setDirection(Float x, Float y, Float z)
{
    _direction.set(x, y, z);
    normalize();
}

Float Ray::intersectsQuery(const BoundingSphere& sphere) const
{
    return sphere.intersectsQuery(*this);
}

Float Ray::intersectsQuery(const BoundingBox& box) const
{
    return box.intersectsQuery(*this);
}

Float Ray::intersectsQuery(const Frustum& frustum) const
{
    Plane n = frustum.getNear();
    Float nD = intersectsQuery(n);
    Float nOD = n.distance(_origin);

    Plane f = frustum.getFar();
    Float fD = intersectsQuery(f);
    Float fOD = f.distance(_origin);

    Plane l = frustum.getLeft();
    Float lD = intersectsQuery(l);
    Float lOD = l.distance(_origin);

    Plane r = frustum.getRight();
    Float rD = intersectsQuery(r);
    Float rOD = r.distance(_origin);

    Plane b = frustum.getBottom();
    Float bD = intersectsQuery(b);
    Float bOD = b.distance(_origin);

    Plane t = frustum.getTop();
    Float tD = intersectsQuery(t);
    Float tOD = t.distance(_origin);

    // If the ray's origin is in the negative half-space of one of the frustum's planes
    // and it does not intersect that same plane, then it does not intersect the frustum.
    if ((nOD < 0.0f && nD < 0.0f) || (fOD < 0.0f && fD < 0.0f) ||
        (lOD < 0.0f && lD < 0.0f)  || (rOD < 0.0f && rD < 0.0f) ||
        (bOD < 0.0f && bD < 0.0f)  || (tOD < 0.0f && tD < 0.0f))
    {
        return Ray::INTERSECTS_NONE;
    }

    // Otherwise, the intersection distance is the minimum positive intersection distance.
    Float d = (nD > 0.0f) ? nD : 0.0f;
    d = (fD > 0.0f) ? ((d == 0.0f) ? fD : min(fD, d)) : d;
    d = (lD > 0.0f) ? ((d == 0.0f) ? lD : min(lD, d)) : d;
    d = (rD > 0.0f) ? ((d == 0.0f) ? rD : min(rD, d)) : d;
    d = (tD > 0.0f) ? ((d == 0.0f) ? bD : min(bD, d)) : d;
    d = (bD > 0.0f) ? ((d == 0.0f) ? tD : min(tD, d)) : d;

    return d;
}

Float Ray::intersectsQuery(const Plane& plane) const
{
    const Vector3& normal = plane.getNormal();
    // If the origin of the ray is on the plane then the distance is zero.
    Float alpha = (normal.dot(_origin) + plane.getNegDistance());
    if (fabs(alpha) < MATH_EPSILON)
    {
        return 0.0f;
    }

    Float dot = normal.dot(_direction);
    
    // If the dot product of the plane's normal and this ray's direction is zero,
    // then the ray is parallel to the plane and does not intersect it.
    if (dot == 0.0f)
    {
        return INTERSECTS_NONE;
    }
    
    // Calculate the distance along the ray's direction vector to the point where
    // the ray intersects the plane (if it is negative the plane is behind the ray).
    Float d = -alpha / dot;
    if (d < 0.0f)
    {
        return INTERSECTS_NONE;
    }
    return d;
}

void Ray::set(const Vector3& origin, const Vector3& direction)
{
    _origin = origin;
    _direction = direction;
    normalize();
}

void Ray::set(const Ray& ray)
{
    _origin = ray._origin;
    _direction = ray._direction;
    normalize();
}

void Ray::transform(const Matrix& matrix)
{
    matrix.transformPoint(&_origin);
    matrix.transformVector(&_direction);
    _direction.normalize();
}

void Ray::normalize()
{
    if (_direction.isZero())
    {
        GP_ERROR("Invalid ray object; a ray's direction must be non-zero.");
        return;
    }

    // Normalize the ray's direction vector.
    Float normalizeFactor = 1.0f / sqrt(_direction.x * _direction.x + _direction.y * _direction.y + _direction.z * _direction.z);
    if (normalizeFactor != 1.0f)
    {
        _direction.x *= normalizeFactor;
        _direction.y *= normalizeFactor;
        _direction.z *= normalizeFactor;
    }
}

Vector3 Ray::closestPointToPoint(Vector3& point) {
    auto directionDistance = (point - _origin).dot(_direction);
	if ( directionDistance < 0 ) {
		return _origin;
	}
    return (_direction * directionDistance) + _origin;
}

Float Ray::distanceToPoint(Vector3& point ) {
	return sqrt(distanceSqToPoint( point ));
}

Float Ray::distanceSqToPoint(Vector3& point ) {
    auto closest = closestPointToPoint(point);
	return point.distanceSquared(closest);
}

Float Ray::distanceSqToSegment(Vector3& v0, Vector3& v1, Vector3* optionalPointOnRay, Vector3* optionalPointOnSegment) {

    // from https://github.com/pmjoniak/GeometricTools/blob/master/GTEngine/Include/Mathematics/GteDistRaySegment.h
    // It returns the min distance between the ray and the segment
    // defined by v0 and v1
    // It can also set two optional targets :
    // - The closest point on the ray
    // - The closest point on the segment

    auto _segCenter = (v0+v1)*0.5;
    auto _segDir = (v1 - v0); _segDir.normalize();
    auto _diff = this->_origin-_segCenter;

    double segExtent = v0.distance(v1) * 0.5;
    double a01 = -_direction.dot(_segDir);
    double b0 = _diff.dot(_direction);
    double b1 = -_diff.dot(_segDir);
    double c = _diff.lengthSquared();
    double det = fabs(1 - a01 * a01);
    double s0, s1, sqrDist, extDet;

    if (det > 0) {
        // The ray and segment are not parallel.
        s0 = a01 * b1 - b0;
        s1 = a01 * b0 - b1;
        extDet = segExtent * det;
        if (s0 >= 0) {
            if (s1 >= -extDet) {
                if (s1 <= extDet) {
                    // region 0
                    // Minimum at interior points of ray and segment.
                    double invDet = 1.0 / det;
                    s0 *= invDet;
                    s1 *= invDet;
                    sqrDist = s0 * (s0 + a01 * s1 + 2 * b0) + s1 * (a01 * s0 + s1 + 2 * b1) + c;
                }
                else {
                    // region 1
                    s1 = segExtent;
                    s0 = std::max(0.0, -(a01 * s1 + b0));
                    sqrDist = -s0 * s0 + s1 * (s1 + 2 * b1) + c;
                }
            }
            else {
                // region 5
                s1 = -segExtent;
                s0 = std::max(0.0, -(a01 * s1 + b0));
                sqrDist = -s0 * s0 + s1 * (s1 + 2 * b1) + c;
            }
        }
        else {
            if (s1 <= -extDet) {
                // region 4
                s0 = std::max(0.0, -(-a01 * segExtent + b0));
                s1 = (s0 > 0) ? -segExtent : std::min(std::max(-segExtent, -b1), segExtent);
                sqrDist = -s0 * s0 + s1 * (s1 + 2 * b1) + c;
            }
            else if (s1 <= extDet) {
                // region 3
                s0 = 0;
                s1 = std::min(std::max(-segExtent, -b1), segExtent);
                sqrDist = s1 * (s1 + 2 * b1) + c;
            }
            else {
                // region 2
                s0 = std::max(0.0, -(a01 * segExtent + b0));
                s1 = (s0 > 0) ? segExtent : std::min(std::max(-segExtent, -b1), segExtent);
                sqrDist = -s0 * s0 + s1 * (s1 + 2 * b1) + c;
            }
        }
    }
    else {
        // Ray and segment are parallel.
        s1 = (a01 > 0) ? -segExtent : segExtent;
        s0 = std::max(0.0, -(a01 * s1 + b0));
        sqrDist = -s0 * s0 + s1 * (s1 + 2 * b1) + c;
    }

    if (optionalPointOnRay) {
        *optionalPointOnRay = (_direction*s0)+(_origin);
    }

    if (optionalPointOnSegment) {
        *optionalPointOnSegment = (_segDir*s1)+(_segCenter);
    }
    return sqrDist;
}

Float Ray::intersectTriangle(Vector3& a, Vector3& b, Vector3& c, bool backfaceCulling, Vector3* target) {

    // Compute the offset origin, edges, and normal.
    // from https://github.com/pmjoniak/GeometricTools/blob/master/GTEngine/Include/Mathematics/GteIntrRay3Triangle3.h

    Vector3 _edge1 = b - a;
    Vector3 _edge2 = c - a;
    Vector3 _normal; Vector3::cross(_edge1, _edge2, &_normal);

    // Solve Q + t*D = b1*E1 + b2*E2 (Q = kDiff, D = ray direction,
    // E1 = kEdge1, E2 = kEdge2, N = Cross(E1,E2)) by
    //   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
    //   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
    //   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
    double DdN = _direction.dot(_normal);
    int sign;
    if (DdN > 0) {
        if (backfaceCulling) return INTERSECTS_NONE;
        sign = 1;
    }
    else if (DdN < 0) {
        sign = -1;
        DdN = -DdN;
    }
    else {
        return INTERSECTS_NONE;
    }

    Vector3 _diff = _origin - a;
    Vector3 _diffCrossEdge2; Vector3::cross(_diff, _edge2, &_diffCrossEdge2);
    double DdQxE2 = sign * _direction.dot(_diffCrossEdge2);

    // b1 < 0, no intersection
    if (DdQxE2 < 0) {
        return INTERSECTS_NONE;
    }

    Vector3 _diffCrossEdge1; Vector3::cross(_edge1, _diff, &_diffCrossEdge1);
    double DdE1xQ = sign * _direction.dot(_diffCrossEdge1);

    // b2 < 0, no intersection
    if (DdE1xQ < 0) {
        return INTERSECTS_NONE;
    }

    // b1+b2 > 1, no intersection
    if (DdQxE2 + DdE1xQ > DdN) {
        return INTERSECTS_NONE;
    }

    // Line intersects triangle, check if ray does.
    double QdN = -sign * _diff.dot(_normal);
    // t < 0, no intersection
    if (QdN < 0) {
        return INTERSECTS_NONE;
    }

    // Ray intersects triangle.
    double t = QdN / DdN;
    if (target) {
        *target = (_direction * t) + _origin;
    }
    return t;
}

}
