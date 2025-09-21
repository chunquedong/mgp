#ifndef RAY_H_
#define RAY_H_

#include "math/Vector3.h"

namespace mgp
{

class Frustum;
class Plane;
class BoundingSphere;
class BoundingBox;

/**
 * Defines a 3-dimensional ray.
 *
 * Rays direction vector are always normalized.
 */
class Ray
{
public:

    /**
     * Represents when a 3D entity does not intersect a ray.
     */
    static const Float INTERSECTS_NONE;

    /**
     * Constructs a new ray initialized to origin(0,0,0) and direction(0,0,1).
     */
    Ray();

    /**
     * Constructs a new ray initialized to the specified values.
     *
     * @param origin The ray's origin.
     * @param direction The ray's direction.
     */
    Ray(const Vector3& origin, const Vector3& direction);

    /**
     * Constructs a new ray initialized to the specified values.
     * 
     * @param originX The x coordinate of the origin.
     * @param originY The y coordinate of the origin.
     * @param originZ The z coordinate of the origin.
     * @param dirX The x coordinate of the direction.
     * @param dirY The y coordinate of the direction.
     * @param dirZ The z coordinate of the direction.
     */
    Ray(Float originX, Float originY, Float originZ, Float dirX, Float dirY, Float dirZ);

    /**
     * Constructs a new ray from the given ray.
     *
     * @param copy The ray to copy.
     */
    Ray(const Ray& copy);

    /**
     * Destructor.
     */
    ~Ray();

    /**
     * Gets the ray's origin.
     *
     * @return The ray's origin.
     */
    const Vector3& getOrigin() const;

    /**
     * Sets the ray's origin to the given point.
     *
     * @param origin The new origin.
     */
    void setOrigin(const Vector3& origin);

    /**
     * Sets the ray's origin.
     * 
     * @param x The x coordinate of the origin.
     * @param y The y coordinate of the origin.
     * @param z The z coordinate of the origin.
     */
    void setOrigin(Float x, Float y, Float z);

    /**
     * Gets the ray's direction.
     *
     * @return The ray's direction.
     */
    const Vector3& getDirection() const;

    /**
     * Sets the ray's direction to the given vector.
     *
     * @param direction The new direction vector.
     */
    void setDirection(const Vector3& direction);

    /**
     * Sets the ray's direction.
     * 
     * @param x The x coordinate of the direction.
     * @param y The y coordinate of the direction.
     * @param z The z coordinate of the direction.
     */
    void setDirection(Float x, Float y, Float z);

    /**
     * Tests whether this ray intersects the specified bounding sphere.
     *
     * @param sphere The bounding sphere to test intersection with.
     * 
     * @return The distance from the origin of this ray to the bounding object or
     *     INTERSECTS_NONE if this ray does not intersect the bounding object.
     */
    Float intersectsQuery(const BoundingSphere& sphere) const;

    /**
     * Tests whether this ray intersects the specified bounding box.
     *
     * @param box The bounding box to test intersection with.
     * 
     * @return The distance from the origin of this ray to the bounding object or
     *     INTERSECTS_NONE if this ray does not intersect the bounding object.
     */
    Float intersectsQuery(const BoundingBox& box) const;

    /**
     * Tests whether this ray intersects the specified frustum.
     *
     * @param frustum The frustum to test intersection with.
     * 
     * @return The distance from the origin of this ray to the frustum or
     *     INTERSECTS_NONE if this ray does not intersect the frustum.
     */
    Float intersectsQuery(const Frustum& frustum) const;

    /**
     * Tests whether this ray intersects the specified plane and returns the distance
     * from the origin of the ray to the plane.
     *
     * @param plane The plane to test intersection with.
     * 
     * @return The distance from the origin of this ray to the plane or
     *     INTERSECTS_NONE if this ray does not intersect the plane.
     */
    Float intersectsQuery(const Plane& plane) const;

    /**
     * Sets this ray to the specified values.
     *
     * @param origin The ray's origin.
     * @param direction The ray's direction.
     */
    void set(const Vector3& origin, const Vector3& direction);

    /**
     * Sets this ray to the given ray.
     *
     * @param ray The ray to copy.
     */
    void set(const Ray& ray);

    /**
     * Transforms this ray by the given transformation matrix.
     *
     * @param matrix The transformation matrix to transform by.
     */
    void transform(const Matrix& matrix);

    /**
     * Transforms this ray by the given matrix.
     * 
     * @param matrix The matrix to transform by.
     * @return This ray, after the transformation occurs.
     */
    inline Ray& operator*=(const Matrix& matrix);

    Vector3 closestPointToPoint(Vector3& point);
    Float distanceToPoint(Vector3& point);
    Float distanceSqToPoint(Vector3& point);
    Float distanceSqToSegment(Vector3& v0, Vector3& v1, Vector3* optionalPointOnRay, Vector3* optionalPointOnSegment);
    Float intersectTriangle(Vector3& a, Vector3& b, Vector3& c, bool backfaceCulling, Vector3* target);
    Float intersectPlane(const Plane& plane, Vector3* target);
private:

    /**
     * Normalizes the ray.
     */
    void normalize();

    Vector3 _origin;        // The ray origin position.
    Vector3 _direction;     // The ray direction vector.
};

/**
 * Transforms the given ray by the given matrix.
 * 
 * @param matrix The matrix to transform by.
 * @param ray The ray to transform.
 * @return The resulting transformed ray.
 */
inline const Ray operator*(const Matrix& matrix, const Ray& ray);

}

#include "Ray.inl"

#endif
