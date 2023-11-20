#include "Ray.h"

namespace mgp
{

inline Ray& Ray::operator*=(const Matrix& matrix)
{
    transform(matrix);
    return *this;
}

inline const Ray operator*(const Matrix& matrix, const Ray& ray)
{
    Ray r(ray);
    r.transform(matrix);
    return r;
}

}
