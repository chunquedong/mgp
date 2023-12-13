#include "base/Base.h"
#include "Plane.h"
#include "Frustum.h"
#include "Ray.h"
#include "BoundingSphere.h"
#include "BoundingBox.h"

namespace mgp
{

Plane::Plane()
    : _normal(0, 1, 0), _negDistance(0)
{
}

Plane::Plane(const Vector3& normal, const Vector3& origin) {
    double dis = normal.dot(origin);
    set(normal, -dis);
}

Plane::Plane(const Vector3& normal, Float negDistance)
{
    set(normal, negDistance);
}

Plane::Plane(Float normalX, Float normalY, Float normalZ, Float negDistance)
{
    set(Vector3(normalX, normalY, normalZ), negDistance);
}

Plane::Plane(const Plane& copy)
{
    set(copy);
}

Plane::~Plane()
{
}

const Vector3& Plane::getNormal() const
{
    return _normal;
}

void Plane::setNormal(const Vector3& normal)
{
    _normal = normal;
    normalize();
}

void Plane::setNormal(Float x, Float y, Float z)
{
    _normal.set(x, y, z);
    normalize();
}

Float Plane::getNegDistance() const
{
    return _negDistance;
}

void Plane::setNegDistance(Float distance)
{
    _negDistance = distance;
}

Float Plane::distance(const Vector3& point) const
{
    return _normal.x * point.x + _normal.y * point.y + _normal.z * point.z + _negDistance;
}

void Plane::intersection(const Plane& p1, const Plane& p2, const Plane& p3, Vector3* point)
{
    GP_ASSERT(point);

    // The planes' normals must be all normalized (which we guarantee in the Plane class).
    // Calculate the determinant of the matrix (i.e | n1 n2 n3 |).
    Float det = p1._normal.x * (p2._normal.y * p3._normal.z -
                p2._normal.z * p3._normal.y) - p2._normal.x *(p1._normal.y * p3._normal.z -
                p1._normal.z * p3._normal.y) + p3._normal.x * (p1._normal.y * p2._normal.z - p1._normal.z * p2._normal.y);

    // If the determinant is zero, then the planes do not all intersect.
    if (fabs(det) <= MATH_EPSILON)
        return;

    // Create 3 points, one on each plane.
    // (We just pick the point on the plane directly along its normal from the origin).
    Float p1x = -p1._normal.x * p1._negDistance;
    Float p1y = -p1._normal.y * p1._negDistance;
    Float p1z = -p1._normal.z * p1._negDistance;
    Float p2x = -p2._normal.x * p2._negDistance;
    Float p2y = -p2._normal.y * p2._negDistance;
    Float p2z = -p2._normal.z * p2._negDistance;
    Float p3x = -p3._normal.x * p3._negDistance;
    Float p3y = -p3._normal.y * p3._negDistance;
    Float p3z = -p3._normal.z * p3._negDistance;

    // Calculate the cross products of the normals.
    Float c1x = (p2._normal.y * p3._normal.z) - (p2._normal.z * p3._normal.y);
    Float c1y = (p2._normal.z * p3._normal.x) - (p2._normal.x * p3._normal.z);
    Float c1z = (p2._normal.x * p3._normal.y) - (p2._normal.y * p3._normal.x);
    Float c2x = (p3._normal.y * p1._normal.z) - (p3._normal.z * p1._normal.y);
    Float c2y = (p3._normal.z * p1._normal.x) - (p3._normal.x * p1._normal.z);
    Float c2z = (p3._normal.x * p1._normal.y) - (p3._normal.y * p1._normal.x);
    Float c3x = (p1._normal.y * p2._normal.z) - (p1._normal.z * p2._normal.y);
    Float c3y = (p1._normal.z * p2._normal.x) - (p1._normal.x * p2._normal.z);
    Float c3z = (p1._normal.x * p2._normal.y) - (p1._normal.y * p2._normal.x);

    // Calculate the point of intersection using the formula:
    // x = (| n1 n2 n3 |)^-1 * [(x1 * n1)(n2 x n3) + (x2 * n2)(n3 x n1) + (x3 * n3)(n1 x n2)]
    Float s1 = p1x * p1._normal.x + p1y * p1._normal.y + p1z * p1._normal.z;
    Float s2 = p2x * p2._normal.x + p2y * p2._normal.y + p2z * p2._normal.z;
    Float s3 = p3x * p3._normal.x + p3y * p3._normal.y + p3z * p3._normal.z;
    Float detI = 1.0f / det;
    point->x = (s1 * c1x + s2 * c2x + s3 * c3x) * detI;
    point->y = (s1 * c1y + s2 * c2y + s3 * c3y) * detI;
    point->z = (s1 * c1z + s2 * c2z + s3 * c3z) * detI;
}

Float Plane::intersectsQuery(const BoundingSphere& sphere) const
{
    return sphere.intersectsQuery(*this);
}

Float Plane::intersectsQuery(const BoundingBox& box) const
{
    return box.intersectsQuery(*this);
}

Float Plane::intersectsQuery(const Frustum& frustum) const
{
    // Get the corners of the frustum.
    Vector3 corners[8];
    frustum.getCorners(corners);

    // Calculate the distances from all of the corners to the plane.
    // If all of the distances are positive, then the frustum is in the
    // positive half-space of this plane; if all the distances are negative,
    // then the frustum is in the negative half-space of this plane; if some of
    // the distances are positive and some are negative, the frustum intersects.
    Float d = distance(corners[0]);
    if (d > 0.0f)
    {
        if (distance(corners[1]) <= 0.0f ||
            distance(corners[2]) <= 0.0f ||
            distance(corners[3]) <= 0.0f ||
            distance(corners[4]) <= 0.0f ||
            distance(corners[5]) <= 0.0f ||
            distance(corners[6]) <= 0.0f ||
            distance(corners[7]) <= 0.0f)
        {
            return Plane::INTERSECTS_INTERSECTING;
        }

        return Plane::INTERSECTS_FRONT;
    }
    else if (d < 0.0f)
    {
        if (distance(corners[1]) >= 0.0f ||
            distance(corners[2]) >= 0.0f ||
            distance(corners[3]) >= 0.0f ||
            distance(corners[4]) >= 0.0f ||
            distance(corners[5]) >= 0.0f ||
            distance(corners[6]) >= 0.0f ||
            distance(corners[7]) >= 0.0f)
        {
            return Plane::INTERSECTS_INTERSECTING;
        }

        return Plane::INTERSECTS_BACK;
    }
    else
    {
        return Plane::INTERSECTS_INTERSECTING;
    }
}

Float Plane::intersectsQuery(const Plane& plane) const
{
    // Check if the planes intersect.
    if ((_normal.x == plane._normal.x && _normal.y == plane._normal.y && _normal.z == plane._normal.z) || !isParallel(plane))
    {
        return Plane::INTERSECTS_INTERSECTING;
    }

    // Calculate the point where the given plane's normal vector intersects the given plane.
    Vector3 point(plane._normal.x * -plane._negDistance, plane._normal.y * -plane._negDistance, plane._normal.z * -plane._negDistance);

    // Calculate whether the given plane is in the positive or negative half-space of this plane
    // (corresponds directly to the sign of the distance from the point calculated above to this plane).
    if (distance(point) > 0.0f)
    {
        return Plane::INTERSECTS_FRONT;
    }
    else
    {
        return Plane::INTERSECTS_BACK;
    }
}

Float Plane::intersectsQuery(const Ray& ray) const
{
    // Calculate the distance from the ray's origin to the plane.
    Float d = distance(ray.getOrigin());

    // If the origin of the ray lies in the plane, then it intersects.
    if (d == 0.0f)
    {
        return Plane::INTERSECTS_INTERSECTING;
    }
    else
    {
        Vector3 rayDirection = ray.getDirection();
        // If the dot product of this plane's normal and the ray's direction is positive, and
        // if the distance from this plane to the ray's origin is negative -> intersection, OR
        // if the dot product of this plane's normal and the ray's direction is negative, and
        // if the distance from this plane to the ray's origin is positive -> intersection.
        if (_normal.x * rayDirection.x + _normal.y * rayDirection.y + _normal.z * rayDirection.z > 0.0f)
        {
            if (d < 0.0f)
            {
                return Plane::INTERSECTS_INTERSECTING;
            }
            else
            {
                return Plane::INTERSECTS_FRONT;
            }
        }
        else
        {
            if (d > 0.0f)
            {
                return Plane::INTERSECTS_INTERSECTING;
            }
            else
            {
                return Plane::INTERSECTS_BACK;
            }
        }
    }
}

bool Plane::intersectsLineSegment(const Vector3& p1, const Vector3& p2, Vector3* point, double* _t) {
    Vector3 direction = p2 - p1;

    double denominator = this->_normal.dot(direction);
    if (denominator == 0) {
        if (_t) *_t = NAN;
        if (this->_normal.dot(p1) + this->_negDistance == 0) {
            *point = p1;
            return true;
        }
        return false;
    }
    double t = -(p1.dot(this->_normal) + this->_negDistance) / denominator;
    if (_t) *_t = t;
    if (t < 0 || t > 1) {
        return false;
    }
    *point = p1 + direction * t;
    return true;
}

bool Plane::isParallel(const Plane& plane) const
{
    return (_normal.y * plane._normal.z) - (_normal.z * plane._normal.y) == 0.0f &&
           (_normal.z * plane._normal.x) - (_normal.x * plane._normal.z) == 0.0f &&
           (_normal.x * plane._normal.y) - (_normal.y * plane._normal.x) == 0.0f;
}

void Plane::set(const Vector3& normal, Float negDistance)
{
    _normal = normal;
    _negDistance = negDistance;
    normalize();
}

void Plane::set(const Plane& plane)
{
    _normal = plane._normal;
    _negDistance = plane._negDistance;
}

void Plane::transform(const Matrix& matrix)
{
    Matrix inverted;
    if (matrix.invert(&inverted))
    {
        // Treat the plane as a four-tuple and multiply by the inverse transpose of the matrix to get the transformed plane.
        // Then we normalize the plane by dividing both the normal and the distance by the length of the normal.
        Float nx = _normal.x * inverted.m[0] + _normal.y * inverted.m[1] + _normal.z * inverted.m[2] + _negDistance * inverted.m[3];
        Float ny = _normal.x * inverted.m[4] + _normal.y * inverted.m[5] + _normal.z * inverted.m[6] + _negDistance * inverted.m[7];
        Float nz = _normal.x * inverted.m[8] + _normal.y * inverted.m[9] + _normal.z * inverted.m[10] + _negDistance * inverted.m[11];
        Float d = _normal.x * inverted.m[12]+ _normal.y * inverted.m[13] + _normal.z * inverted.m[14] + _negDistance * inverted.m[15];
        Float divisor = sqrt(nx * nx + ny * ny + nz * nz);
        GP_ASSERT(divisor);
        Float factor = 1.0f / divisor;

        _normal.x = nx * factor;
        _normal.y = ny * factor;
        _normal.z = nz * factor;
        _negDistance = d * factor;
    }
}

void Plane::normalize()
{
    if (_normal.isZero())
        return;

    // Normalize the plane's normal.
    Float normalizeFactor = 1.0f / sqrt(_normal.x * _normal.x + _normal.y * _normal.y + _normal.z * _normal.z);

    if (normalizeFactor != 1.0f)
    {
        _normal.x *= normalizeFactor;
        _normal.y *= normalizeFactor;
        _normal.z *= normalizeFactor;
        _negDistance *= normalizeFactor;
    }
}

}
