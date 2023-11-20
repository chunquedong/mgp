#include "math/Matrix.h"

namespace mgp
{

inline const Matrix4 Matrix4::operator+(const Matrix4& m) const
{
    Matrix4 result(*this);
    result.add(m);
    return result;
}

inline Matrix4& Matrix4::operator+=(const Matrix4& m)
{
    add(m);
    return *this;
}

inline const Matrix4 Matrix4::operator-(const Matrix4& m) const
{
    Matrix4 result(*this);
    result.subtract(m);
    return result;
}

inline Matrix4& Matrix4::operator-=(const Matrix4& m)
{
    subtract(m);
    return *this;
}

inline const Matrix4 Matrix4::operator-() const
{
    Matrix4 m(*this);
    m.negate();
    return m;
}

inline const Matrix4 Matrix4::operator*(const Matrix4& m) const
{
    Matrix4 result(*this);
    result.multiply(m);
    return result;
}

inline Matrix4& Matrix4::operator*=(const Matrix4& m)
{
    multiply(m);
    return *this;
}

inline Vector3& operator*=(Vector3& v, const Matrix4& m)
{
    m.transformVector(&v);
    return v;
}

inline const Vector3 operator*(const Matrix4& m, const Vector3& v)
{
    Vector3 x;
    m.transformVector(v, &x);
    return x;
}

inline Vector4& operator*=(Vector4& v, const Matrix4& m)
{
    m.transformVector(&v);
    return v;
}

inline const Vector4 operator*(const Matrix4& m, const Vector4& v)
{
    Vector4 x;
    m.transformVector(v, &x);
    return x;
}

}
