#include "LineSegment.h"

using namespace mgp;

bool LineSegment::intersection(const LineSegment& that, Vector3* intersectPoint, bool strict) const {
    const Vector3& line1P1 = p1;
    const Vector3& line1P2 = p2;
    const Vector3& line2P1 = that.p1;
    const Vector3& line2P2 = that.p2;

    Vector3 v1 = line1P2 - line1P1;
    Vector3 v2 = line2P2 - line2P1;

    Vector3 startPointSeg = line2P1 - line1P1;
    Vector3 vecS1; Vector3::cross(v1, v2, &vecS1);
    Vector3 vecS2; Vector3::cross(startPointSeg, v2, &vecS2);

    double num = Vector3::dot(startPointSeg, vecS1);
    if (strict && (num >= 1E-05f || num <= -1E-05f))
    {
        return false;
    }

    double lengthSquared = vecS1.lengthSquared();
    if (lengthSquared < 1E-05f) {
        return false;
    }

    double num2 = Vector3::dot(vecS2, vecS1) / lengthSquared;
    if (num2 > 1 || num2 < 0) {
        return false;
    }

    *intersectPoint = line1P1 + v1 * num2;
    return true;
}

Float LineSegment::distanceToPoint(const Vector3& point) const {
    Vector3 v1 = p2 - p1;
    Vector3 v2 = point - p1;

    double dotP = Vector3::dot(v1, v2);
    if (dotP < 0) {
        return point.distance(p1);
    }
    if (dotP > v1.lengthSquared()) {
        return point.distance(p2);
    }

    Vector3 vecS1; Vector3::cross(v1, v2, &vecS1);
    return vecS1.length() / v1.length();
}

bool mgp::triangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3, Vector3* normal) {
    Vector3 d1 = p2 - p1;
    Vector3 d2 = p3 - p1;
    Vector3::cross(d1, d2, normal);
    return normal->normalize(normal);
}