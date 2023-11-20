#include "math/Vector3.h"
#include "math/Matrix.h"

namespace mgp
{

inline const Vector3 Vector3::operator+(const Vector3& v) const
{
    Vector3 result(*this);
    result.add(v);
    return result;
}

inline Vector3& Vector3::operator+=(const Vector3& v)
{
    add(v);
    return *this;
}

inline const Vector3 Vector3::operator-(const Vector3& v) const
{
    Vector3 result(*this);
    result.subtract(v);
    return result;
}

inline Vector3& Vector3::operator-=(const Vector3& v)
{
    subtract(v);
    return *this;
}

inline const Vector3 Vector3::operator-() const
{
    Vector3 result(*this);
    result.negate();
    return result;
}

inline const Vector3 Vector3::operator*(Float x) const
{
    Vector3 result(*this);
    result.scale(x);
    return result;
}

inline Vector3& Vector3::operator*=(Float x)
{
    scale(x);
    return *this;
}

inline const Vector3 Vector3::operator/(const Float x) const
{
    return Vector3(this->x / x, this->y / x, this->z / x);
}

inline bool Vector3::operator<(const Vector3& v) const
{
    if (x == v.x)
    {
        if (y == v.y)
        {
            return z < v.z;
        }
        return y < v.y;
    }
    return x < v.x;
}

inline bool Vector3::operator==(const Vector3& v) const
{
    return x==v.x && y==v.y && z==v.z;
}

inline bool Vector3::operator!=(const Vector3& v) const
{
    return x!=v.x || y!=v.y || z!=v.z;
}

inline const Vector3 operator*(Float x, const Vector3& v)
{
    Vector3 result(v);
    result.scale(x);
    return result;
}

}
