#include "math/Matrix.h"
#include "math/Vector4.h"

namespace mgp
{

inline const Vector4 Vector4::operator+(const Vector4& v) const
{
    Vector4 result(*this);
    result.add(v);
    return result;
}

inline Vector4& Vector4::operator+=(const Vector4& v)
{
    add(v);
    return *this;
}

inline const Vector4 Vector4::operator-(const Vector4& v) const
{
    Vector4 result(*this);
    result.subtract(v);
    return result;
}

inline Vector4& Vector4::operator-=(const Vector4& v)
{
    subtract(v);
    return *this;
}

inline const Vector4 Vector4::operator-() const
{
    Vector4 result(*this);
    result.negate();
    return result;
}

inline const Vector4 Vector4::operator*(Float x) const
{
    Vector4 result(*this);
    result.scale(x);
    return result;
}

inline Vector4& Vector4::operator*=(Float x)
{
    scale(x);
    return *this;
}

inline const Vector4 Vector4::operator/(const Float x) const
{
    return Vector4(this->x / x, this->y / x, this->z / x, this->w / x);
}

inline bool Vector4::operator<(const Vector4& v) const
{
    if (x == v.x)
    {
        if (y == v.y)
        {
            if (z == v.z)
            {
                return w < v.w;
            }
            return z < v.z;
        }
        return y < v.y;
    }
    return x < v.x;
}

inline bool Vector4::operator==(const Vector4& v) const
{
    return x==v.x && y==v.y && z==v.z && w==v.w;
}

inline bool Vector4::operator!=(const Vector4& v) const
{
    return x!=v.x || y!=v.y || z!=v.z || w!=v.w;
}

inline const Vector4 operator*(Float x, const Vector4& v)
{
    Vector4 result(v);
    result.scale(x);
    return result;
}

}
