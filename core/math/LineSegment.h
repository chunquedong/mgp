#ifndef LINESEGMENT_H_
#define LINESEGMENT_H_

#include "Vector3.h"

namespace mgp
{
class LineSegment {
public:
    Vector3 p1;
    Vector3 p2;

    LineSegment(const Vector3& f, const Vector3& t) : p1(f), p2(t) {}

    bool intersection(const LineSegment& that, Vector3* intersectPoint, bool strict = true) const;
    Float distanceToPoint(const Vector3& point) const;
};

bool triangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3, Vector3* normal);

}

#endif //MATH_BASE_H_