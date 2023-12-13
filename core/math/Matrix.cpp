#include "base/Base.h"
#include "math/Matrix.h"
#include "Plane.h"
#include "Quaternion.h"
#include "MathUtil.h"

namespace mgp
{

#define GP_MATH_MATRIX_SIZE             (sizeof(Float) * 16)

static const Float MATRIX_IDENTITY[16] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

Matrix4::Matrix4()
{
    *this = Matrix4::identity();
}

Matrix4::Matrix4(Float m11, Float m12, Float m13, Float m14, Float m21, Float m22, Float m23, Float m24,
               Float m31, Float m32, Float m33, Float m34, Float m41, Float m42, Float m43, Float m44)
{
    set(m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44);
}

Matrix4::Matrix4(const float* m)
{
    set(m);
}

Matrix4::Matrix4(const Matrix4& copy)
{
    memcpy(m, copy.m, MATRIX_SIZE);
}

Matrix4::~Matrix4()
{
}

const Matrix4& Matrix4::identity()
{
    static Matrix4 m(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 );
    return m;
}

const Matrix4& Matrix4::zero()
{
    static Matrix4 m(
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0 );
    return m;
}

void Matrix4::createLookAt(const Vector3& eyePosition, const Vector3& targetPosition, const Vector3& up, Matrix4* dst, bool isView)
{
    createLookAt(eyePosition.x, eyePosition.y, eyePosition.z, targetPosition.x, targetPosition.y, targetPosition.z,
                 up.x, up.y, up.z, dst, isView);
}

void Matrix4::createLookAt(Float eyePositionX, Float eyePositionY, Float eyePositionZ,
                          Float targetPositionX, Float targetPositionY, Float targetPositionZ,
                          Float upX, Float upY, Float upZ, Matrix4* dst, bool isView)
{
    GP_ASSERT(dst);

    Vector3 eye(eyePositionX, eyePositionY, eyePositionZ);
    Vector3 target(targetPositionX, targetPositionY, targetPositionZ);
    Vector3 up(upX, upY, upZ);
    up.normalize();

    Vector3 zaxis;
    Vector3::subtract(eye, target, &zaxis);
    zaxis.normalize();

    Vector3 xaxis;
    Vector3::cross(up, zaxis, &xaxis);
    xaxis.normalize();

    Vector3 yaxis;
    Vector3::cross(zaxis, xaxis, &yaxis);
    yaxis.normalize();

    if (isView) {
        dst->m[0] = xaxis.x;
        dst->m[1] = yaxis.x;
        dst->m[2] = zaxis.x;
        dst->m[3] = 0.0f;

        dst->m[4] = xaxis.y;
        dst->m[5] = yaxis.y;
        dst->m[6] = zaxis.y;
        dst->m[7] = 0.0f;

        dst->m[8] = xaxis.z;
        dst->m[9] = yaxis.z;
        dst->m[10] = zaxis.z;
        dst->m[11] = 0.0f;

        dst->m[12] = -Vector3::dot(xaxis, eye);
        dst->m[13] = -Vector3::dot(yaxis, eye);
        dst->m[14] = -Vector3::dot(zaxis, eye);
        dst->m[15] = 1.0f;
    }
    else {
        dst->m[0] = xaxis.x;
        dst->m[1] = xaxis.y;
        dst->m[2] = xaxis.z;
        dst->m[3] = 0.0f;

        dst->m[4] = yaxis.x;
        dst->m[5] = yaxis.y;
        dst->m[6] = yaxis.z;
        dst->m[7] = 0.0f;

        dst->m[8] = zaxis.x;
        dst->m[9] = zaxis.y;
        dst->m[10] = zaxis.z;
        dst->m[11] = 0.0f;

        dst->m[12] = eye.x;
        dst->m[13] = eye.y;
        dst->m[14] = eye.z;
        dst->m[15] = 1.0f;
    }
}

void Matrix4::createPerspective(Float fieldOfView, Float aspectRatio,
                                     Float zNearPlane, Float zFarPlane, Matrix4* dst)
{
    GP_ASSERT(dst);
    GP_ASSERT(zFarPlane != zNearPlane);

    Float f_n = 1.0f / (zFarPlane - zNearPlane);
    Float theta = MATH_DEG_TO_RAD(fieldOfView) * 0.5f;
    if (fabs(fmod(theta, MATH_PIOVER2)) < MATH_EPSILON)
    {
        GP_ERROR("Invalid field of view value (%d) causes attempted calculation tan(%d), which is undefined.", fieldOfView, theta);
        return;
    }
    Float divisor = tan(theta);
    GP_ASSERT(divisor);
    Float factor = 1.0f / divisor;

    memset(dst, 0, MATRIX_SIZE);

    GP_ASSERT(aspectRatio);
    dst->m[0] = (1.0f / aspectRatio) * factor;
    dst->m[5] = factor;
    dst->m[10] = (-(zFarPlane + zNearPlane)) * f_n;
    dst->m[11] = -1.0f;
    dst->m[14] = -2.0f * zFarPlane * zNearPlane * f_n;
}

void Matrix4::createOrthographic(Float width, Float height, Float zNearPlane, Float zFarPlane, Matrix4* dst)
{
    Float halfWidth = width / 2.0f;
    Float halfHeight = height / 2.0f;
    createOrthographicOffCenter(-halfWidth, halfWidth, -halfHeight, halfHeight, zNearPlane, zFarPlane, dst);
}

void Matrix4::createOrthographicOffCenter(Float left, Float right, Float bottom, Float top,
                                         Float zNearPlane, Float zFarPlane, Matrix4* dst)
{
    GP_ASSERT(dst);
    GP_ASSERT(right != left);
    GP_ASSERT(top != bottom);
    GP_ASSERT(zFarPlane != zNearPlane);

    memset(dst, 0, MATRIX_SIZE);
    dst->m[0] = 2 / (right - left);
    dst->m[5] = 2 / (top - bottom);
    dst->m[12] = (left + right) / (left - right);
    dst->m[10] = -2 / (zFarPlane-zNearPlane);
    dst->m[13] = (top + bottom) / (bottom - top);
    dst->m[14] = -(zNearPlane + zFarPlane) / (zFarPlane-zNearPlane);
    dst->m[15] = 1;
}
    
void Matrix4::createBillboard(const Vector3& objectPosition, const Vector3& cameraPosition,
                             const Vector3& cameraUpVector, Matrix4* dst)
{
    createBillboardHelper(objectPosition, cameraPosition, cameraUpVector, NULL, dst);
}

void Matrix4::createBillboard(const Vector3& objectPosition, const Vector3& cameraPosition,
                             const Vector3& cameraUpVector, const Vector3& cameraForwardVector,
                             Matrix4* dst)
{
    createBillboardHelper(objectPosition, cameraPosition, cameraUpVector, &cameraForwardVector, dst);
}

void Matrix4::createBillboardHelper(const Vector3& objectPosition, const Vector3& cameraPosition,
                                   const Vector3& cameraUpVector, const Vector3* cameraForwardVector,
                                   Matrix4* dst)
{
    Vector3 delta(objectPosition, cameraPosition);
    bool isSufficientDelta = delta.lengthSquared() > MATH_EPSILON;

    dst->setIdentity();
    dst->m[3] = objectPosition.x;
    dst->m[7] = objectPosition.y;
    dst->m[11] = objectPosition.z;

    // As per the contracts for the 2 variants of createBillboard, we need
    // either a safe default or a sufficient distance between object and camera.
    if (cameraForwardVector || isSufficientDelta)
    {
        Vector3 target = isSufficientDelta ? cameraPosition : (objectPosition - *cameraForwardVector);

        // A billboard is the inverse of a lookAt rotation
        Matrix4 lookAt;
        createLookAt(objectPosition, target, cameraUpVector, &lookAt, true);
        dst->m[0] = lookAt.m[0];
        dst->m[1] = lookAt.m[4];
        dst->m[2] = lookAt.m[8];
        dst->m[4] = lookAt.m[1];
        dst->m[5] = lookAt.m[5];
        dst->m[6] = lookAt.m[9];
        dst->m[8] = lookAt.m[2];
        dst->m[9] = lookAt.m[6];
        dst->m[10] = lookAt.m[10];
    }
}
    
void Matrix4::createReflection(const Plane& plane, Matrix4* dst)
{
    Vector3 normal(plane.getNormal());
    Float k = -2.0f * plane.getNegDistance();

    dst->setIdentity();

    dst->m[0] -= 2.0f * normal.x * normal.x;
    dst->m[5] -= 2.0f * normal.y * normal.y;
    dst->m[10] -= 2.0f * normal.z * normal.z;
    dst->m[1] = dst->m[4] = -2.0f * normal.x * normal.y;
    dst->m[2] = dst->m[8] = -2.0f * normal.x * normal.z;
    dst->m[6] = dst->m[9] = -2.0f * normal.y * normal.z;
    
    dst->m[3] = k * normal.x;
    dst->m[7] = k * normal.y;
    dst->m[11] = k * normal.z;
}

void Matrix4::createScale(const Vector3& scale, Matrix4* dst)
{
    GP_ASSERT(dst);

    memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

    dst->m[0] = scale.x;
    dst->m[5] = scale.y;
    dst->m[10] = scale.z;
}

void Matrix4::createScale(Float xScale, Float yScale, Float zScale, Matrix4* dst)
{
    GP_ASSERT(dst);

    memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

    dst->m[0] = xScale;
    dst->m[5] = yScale;
    dst->m[10] = zScale;
}


void Matrix4::createRotation(const Quaternion& q, Matrix4* dst)
{
    GP_ASSERT(dst);

    Float x2 = q.x + q.x;
    Float y2 = q.y + q.y;
    Float z2 = q.z + q.z;

    Float xx2 = q.x * x2;
    Float yy2 = q.y * y2;
    Float zz2 = q.z * z2;
    Float xy2 = q.x * y2;
    Float xz2 = q.x * z2;
    Float yz2 = q.y * z2;
    Float wx2 = q.w * x2;
    Float wy2 = q.w * y2;
    Float wz2 = q.w * z2;

    dst->m[0] = 1.0f - yy2 - zz2;
    dst->m[1] = xy2 + wz2;
    dst->m[2] = xz2 - wy2;
    dst->m[3] = 0.0f;

    dst->m[4] = xy2 - wz2;
    dst->m[5] = 1.0f - xx2 - zz2;
    dst->m[6] = yz2 + wx2;
    dst->m[7] = 0.0f;

    dst->m[8] = xz2 + wy2;
    dst->m[9] = yz2 - wx2;
    dst->m[10] = 1.0f - xx2 - yy2;
    dst->m[11] = 0.0f;

    dst->m[12] = 0.0f;
    dst->m[13] = 0.0f;
    dst->m[14] = 0.0f;
    dst->m[15] = 1.0f;
}

void Matrix4::createRotation(const Vector3& axis, Float angle, Matrix4* dst)
{
    GP_ASSERT(dst);

    Float x = axis.x;
    Float y = axis.y;
    Float z = axis.z;

    // Make sure the input axis is normalized.
    Float n = x*x + y*y + z*z;
    if (n != 1.0f)
    {
        // Not normalized.
        n = sqrt(n);
        // Prevent divide too close to zero.
        if (n > 0.000001f)
        {
            n = 1.0f / n;
            x *= n;
            y *= n;
            z *= n;
        }
    }

    Float c = cos(angle);
    Float s = sin(angle);

    Float t = 1.0f - c;
    Float tx = t * x;
    Float ty = t * y;
    Float tz = t * z;
    Float txy = tx * y;
    Float txz = tx * z;
    Float tyz = ty * z;
    Float sx = s * x;
    Float sy = s * y;
    Float sz = s * z;

    dst->m[0] = c + tx*x;
    dst->m[1] = txy + sz;
    dst->m[2] = txz - sy;
    dst->m[3] = 0.0f;

    dst->m[4] = txy - sz;
    dst->m[5] = c + ty*y;
    dst->m[6] = tyz + sx;
    dst->m[7] = 0.0f;

    dst->m[8] = txz + sy;
    dst->m[9] = tyz - sx;
    dst->m[10] = c + tz*z;
    dst->m[11] = 0.0f;

    dst->m[12] = 0.0f;
    dst->m[13] = 0.0f;
    dst->m[14] = 0.0f;
    dst->m[15] = 1.0f;
}

void Matrix4::createRotationX(Float angle, Matrix4* dst)
{
    GP_ASSERT(dst);

    memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

    Float c = cos(angle);
    Float s = sin(angle);

    dst->m[5]  = c;
    dst->m[6]  = s;
    dst->m[9]  = -s;
    dst->m[10] = c;
}

void Matrix4::createRotationY(Float angle, Matrix4* dst)
{
    GP_ASSERT(dst);

    memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

    Float c = cos(angle);
    Float s = sin(angle);

    dst->m[0]  = c;
    dst->m[2]  = -s;
    dst->m[8]  = s;
    dst->m[10] = c;
}

void Matrix4::createRotationZ(Float angle, Matrix4* dst)
{
    GP_ASSERT(dst);

    memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

    Float c = cos(angle);
    Float s = sin(angle);

    dst->m[0] = c;
    dst->m[1] = s;
    dst->m[4] = -s;
    dst->m[5] = c;
}

void Matrix4::createFromEuler(Float yaw, Float pitch, Float roll, Matrix4* dst)
{
	GP_ASSERT(dst);

	memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);
	
	dst->rotateY(yaw);
	dst->rotateX(pitch);
	dst->rotateZ(roll);
}

void Matrix4::createTranslation(const Vector3& translation, Matrix4* dst)
{
    GP_ASSERT(dst);

    memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

    dst->m[12] = translation.x;
    dst->m[13] = translation.y;
    dst->m[14] = translation.z;
}

void Matrix4::createTranslation(Float xTranslation, Float yTranslation, Float zTranslation, Matrix4* dst)
{
    GP_ASSERT(dst);

    memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

    dst->m[12] = xTranslation;
    dst->m[13] = yTranslation;
    dst->m[14] = zTranslation;
}

void Matrix4::add(Float scalar)
{
    add(scalar, this);
}

void Matrix4::add(Float scalar, Matrix4* dst)
{
    GP_ASSERT(dst);

    MathUtil::addMatrix(m, scalar, dst->m);
}

void Matrix4::add(const Matrix4& m)
{
    add(*this, m, this);
}

void Matrix4::add(const Matrix4& m1, const Matrix4& m2, Matrix4* dst)
{
    GP_ASSERT(dst);

    MathUtil::addMatrix(m1.m, m2.m, dst->m);
}

bool Matrix4::decompose(Vector3* scale, Quaternion* rotation, Vector3* translation) const
{
    if (translation)
    {
        // Extract the translation.
        translation->x = m[12];
        translation->y = m[13];
        translation->z = m[14];
    }

    // Nothing left to do.
    if (scale == NULL && rotation == NULL)
        return true;

    // Extract the scale.
    // This is simply the length of each axis (row/column) in the matrix.
    Vector3 xaxis(m[0], m[1], m[2]);
    Float scaleX = xaxis.length();

    Vector3 yaxis(m[4], m[5], m[6]);
    Float scaleY = yaxis.length();

    Vector3 zaxis(m[8], m[9], m[10]);
    Float scaleZ = zaxis.length();

    // Determine if we have a negative scale (true if determinant is less than zero).
    // In this case, we simply negate a single axis of the scale.
    Float det = determinant();
    if (det < 0)
        scaleZ = -scaleZ;

    if (scale)
    {
        scale->x = scaleX;
        scale->y = scaleY;
        scale->z = scaleZ;
    }

    // Nothing left to do.
    if (rotation == NULL)
        return true;

    // Scale too close to zero, can't decompose rotation.
    if (scaleX < MATH_TOLERANCE || scaleY < MATH_TOLERANCE || fabs(scaleZ) < MATH_TOLERANCE)
        return false;

    Float rn;

    // Factor the scale out of the matrix axes.
    rn = 1.0f / scaleX;
    xaxis.x *= rn;
    xaxis.y *= rn;
    xaxis.z *= rn;

    rn = 1.0f / scaleY;
    yaxis.x *= rn;
    yaxis.y *= rn;
    yaxis.z *= rn;

    rn = 1.0f / scaleZ;
    zaxis.x *= rn;
    zaxis.y *= rn;
    zaxis.z *= rn;

    // Now calculate the rotation from the resulting matrix (axes).
    Float trace = xaxis.x + yaxis.y + zaxis.z + 1.0f;

    if (trace > 1.0f)
    {
        Float s = 0.5f / sqrt(trace);
        rotation->w = 0.25f / s;
        rotation->x = (yaxis.z - zaxis.y) * s;
        rotation->y = (zaxis.x - xaxis.z) * s;
        rotation->z = (xaxis.y - yaxis.x) * s;
    }
    else
    {
        // Note: since xaxis, yaxis, and zaxis are normalized, 
        // we will never divide by zero in the code below.
        if (xaxis.x > yaxis.y && xaxis.x > zaxis.z)
        {
            Float s = 0.5f / sqrt(1.0f + xaxis.x - yaxis.y - zaxis.z);
            rotation->w = (yaxis.z - zaxis.y) * s;
            rotation->x = 0.25f / s;
            rotation->y = (yaxis.x + xaxis.y) * s;
            rotation->z = (zaxis.x + xaxis.z) * s;
        }
        else if (yaxis.y > zaxis.z)
        {
            Float s = 0.5f / sqrt(1.0f + yaxis.y - xaxis.x - zaxis.z);
            rotation->w = (zaxis.x - xaxis.z) * s;
            rotation->x = (yaxis.x + xaxis.y) * s;
            rotation->y = 0.25f / s;
            rotation->z = (zaxis.y + yaxis.z) * s;
        }
        else
        {
            Float s = 0.5f / sqrt(1.0f + zaxis.z - xaxis.x - yaxis.y );
            rotation->w = (xaxis.y - yaxis.x ) * s;
            rotation->x = (zaxis.x + xaxis.z ) * s;
            rotation->y = (zaxis.y + yaxis.z ) * s;
            rotation->z = 0.25f / s;
        }
    }

    return true;
}

Float Matrix4::determinant() const
{
    Float a0 = m[0] * m[5] - m[1] * m[4];
    Float a1 = m[0] * m[6] - m[2] * m[4];
    Float a2 = m[0] * m[7] - m[3] * m[4];
    Float a3 = m[1] * m[6] - m[2] * m[5];
    Float a4 = m[1] * m[7] - m[3] * m[5];
    Float a5 = m[2] * m[7] - m[3] * m[6];
    Float b0 = m[8] * m[13] - m[9] * m[12];
    Float b1 = m[8] * m[14] - m[10] * m[12];
    Float b2 = m[8] * m[15] - m[11] * m[12];
    Float b3 = m[9] * m[14] - m[10] * m[13];
    Float b4 = m[9] * m[15] - m[11] * m[13];
    Float b5 = m[10] * m[15] - m[11] * m[14];

    // Calculate the determinant.
    return (a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0);
}

void Matrix4::getScale(Vector3* scale) const
{
    decompose(scale, NULL, NULL);
}

bool Matrix4::getRotation(Quaternion* rotation) const
{
    return decompose(NULL, rotation, NULL);
}

void Matrix4::getTranslation(Vector3* translation) const
{
    decompose(NULL, NULL, translation);
}

void Matrix4::getUpVector(Vector3* dst) const
{
    GP_ASSERT(dst);

    dst->x = m[4];
    dst->y = m[5];
    dst->z = m[6];
}

void Matrix4::getDownVector(Vector3* dst) const
{
    GP_ASSERT(dst);
    
    dst->x = -m[4];
    dst->y = -m[5];
    dst->z = -m[6];
}

void Matrix4::getLeftVector(Vector3* dst) const
{
    GP_ASSERT(dst);

    dst->x = -m[0];
    dst->y = -m[1];
    dst->z = -m[2];
}

void Matrix4::getRightVector(Vector3* dst) const
{
    GP_ASSERT(dst);

    dst->x = m[0];
    dst->y = m[1];
    dst->z = m[2];
}

void Matrix4::getForwardVector(Vector3* dst) const
{
    GP_ASSERT(dst);

    dst->x = -m[8];
    dst->y = -m[9];
    dst->z = -m[10];
}

void Matrix4::getBackVector(Vector3* dst) const
{
    GP_ASSERT(dst);

    dst->x = m[8];
    dst->y = m[9];
    dst->z = m[10];
}

bool Matrix4::invert()
{
    return invert(this);
}

bool Matrix4::invert(Matrix4* dst) const
{
    Float a0 = m[0] * m[5] - m[1] * m[4];
    Float a1 = m[0] * m[6] - m[2] * m[4];
    Float a2 = m[0] * m[7] - m[3] * m[4];
    Float a3 = m[1] * m[6] - m[2] * m[5];
    Float a4 = m[1] * m[7] - m[3] * m[5];
    Float a5 = m[2] * m[7] - m[3] * m[6];
    Float b0 = m[8] * m[13] - m[9] * m[12];
    Float b1 = m[8] * m[14] - m[10] * m[12];
    Float b2 = m[8] * m[15] - m[11] * m[12];
    Float b3 = m[9] * m[14] - m[10] * m[13];
    Float b4 = m[9] * m[15] - m[11] * m[13];
    Float b5 = m[10] * m[15] - m[11] * m[14];

    // Calculate the determinant.
    Float det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

    // Close to zero, can't invert.
    if (fabs(det) <= MATH_TOLERANCE)
        return false;

    // Support the case where m == dst.
    Matrix4 inverse;
    inverse.m[0]  = m[5] * b5 - m[6] * b4 + m[7] * b3;
    inverse.m[1]  = -m[1] * b5 + m[2] * b4 - m[3] * b3;
    inverse.m[2]  = m[13] * a5 - m[14] * a4 + m[15] * a3;
    inverse.m[3]  = -m[9] * a5 + m[10] * a4 - m[11] * a3;

    inverse.m[4]  = -m[4] * b5 + m[6] * b2 - m[7] * b1;
    inverse.m[5]  = m[0] * b5 - m[2] * b2 + m[3] * b1;
    inverse.m[6]  = -m[12] * a5 + m[14] * a2 - m[15] * a1;
    inverse.m[7]  = m[8] * a5 - m[10] * a2 + m[11] * a1;

    inverse.m[8]  = m[4] * b4 - m[5] * b2 + m[7] * b0;
    inverse.m[9]  = -m[0] * b4 + m[1] * b2 - m[3] * b0;
    inverse.m[10] = m[12] * a4 - m[13] * a2 + m[15] * a0;
    inverse.m[11] = -m[8] * a4 + m[9] * a2 - m[11] * a0;

    inverse.m[12] = -m[4] * b3 + m[5] * b1 - m[6] * b0;
    inverse.m[13] = m[0] * b3 - m[1] * b1 + m[2] * b0;
    inverse.m[14] = -m[12] * a3 + m[13] * a1 - m[14] * a0;
    inverse.m[15] = m[8] * a3 - m[9] * a1 + m[10] * a0;

    multiply(inverse, 1.0f / det, dst);

    return true;
}

bool Matrix4::isIdentity() const
{
    return (memcmp(m, MATRIX_IDENTITY, MATRIX_SIZE) == 0);
}

void Matrix4::multiply(Float scalar)
{
    multiply(scalar, this);
}

void Matrix4::multiply(Float scalar, Matrix4* dst) const
{
    multiply(*this, scalar, dst);
}

void Matrix4::multiply(const Matrix4& m, Float scalar, Matrix4* dst)
{
    GP_ASSERT(dst);

    MathUtil::multiplyMatrix(m.m, scalar, dst->m);
}

void Matrix4::multiply(const Matrix4& m)
{
    multiply(*this, m, this);
}

void Matrix4::multiply(const Matrix4& m1, const Matrix4& m2, Matrix4* dst)
{
    GP_ASSERT(dst);

    MathUtil::multiplyMatrix(m1.m, m2.m, dst->m);
}

void Matrix4::negate()
{
    negate(this);
}

void Matrix4::negate(Matrix4* dst) const
{
    GP_ASSERT(dst);

    MathUtil::negateMatrix(m, dst->m);
}

void Matrix4::rotate(const Quaternion& q)
{
    rotate(q, this);
}

void Matrix4::rotate(const Quaternion& q, Matrix4* dst) const
{
    Matrix4 r;
    createRotation(q, &r);
    multiply(*this, r, dst);
}

void Matrix4::rotate(const Vector3& axis, Float angle)
{
    rotate(axis, angle, this);
}

void Matrix4::rotate(const Vector3& axis, Float angle, Matrix4* dst) const
{
    Matrix4 r;
    createRotation(axis, angle, &r);
    multiply(*this, r, dst);
}

void Matrix4::rotateX(Float angle)
{
    rotateX(angle, this);
}

void Matrix4::rotateX(Float angle, Matrix4* dst) const
{
    Matrix4 r;
    createRotationX(angle, &r);
    multiply(*this, r, dst);
}

void Matrix4::rotateY(Float angle)
{
    rotateY(angle, this);
}

void Matrix4::rotateY(Float angle, Matrix4* dst) const
{
    Matrix4 r;
    createRotationY(angle, &r);
    multiply(*this, r, dst);
}

void Matrix4::rotateZ(Float angle)
{
    rotateZ(angle, this);
}

void Matrix4::rotateZ(Float angle, Matrix4* dst) const
{
    Matrix4 r;
    createRotationZ(angle, &r);
    multiply(*this, r, dst);
}

void Matrix4::scale(Float value)
{
    scale(value, this);
}

void Matrix4::scale(Float value, Matrix4* dst) const
{
    scale(value, value, value, dst);
}

void Matrix4::scale(Float xScale, Float yScale, Float zScale)
{
    scale(xScale, yScale, zScale, this);
}

void Matrix4::scale(Float xScale, Float yScale, Float zScale, Matrix4* dst) const
{
    Matrix4 s;
    createScale(xScale, yScale, zScale, &s);
    multiply(*this, s, dst);
}

void Matrix4::scale(const Vector3& s)
{
    scale(s.x, s.y, s.z, this);
}

void Matrix4::scale(const Vector3& s, Matrix4* dst) const
{
    scale(s.x, s.y, s.z, dst);
}

void Matrix4::set(Float m11, Float m12, Float m13, Float m14, Float m21, Float m22, Float m23, Float m24,
                 Float m31, Float m32, Float m33, Float m34, Float m41, Float m42, Float m43, Float m44)
{
    m[0]  = m11;
    m[1]  = m21;
    m[2]  = m31;
    m[3]  = m41;
    m[4]  = m12;
    m[5]  = m22;
    m[6]  = m32;
    m[7]  = m42;
    m[8]  = m13;
    m[9]  = m23;
    m[10] = m33;
    m[11] = m43;
    m[12] = m14;
    m[13] = m24;
    m[14] = m34;
    m[15] = m44;
}

void Matrix4::set(const float* m)
{
    GP_ASSERT(m);
    //memcpy(this->m, m, MATRIX_SIZE);
    for (int i = 0; i < 16; ++i) {
        this->m[i] = m[i];
    }
}

void Matrix4::toArray(float* m) const {
    GP_ASSERT(m);
    for (int i = 0; i < 16; ++i) {
        m[i] = this->m[i];
    }
}

void Matrix4::set(const Matrix4& m)
{
    memcpy(this->m, m.m, MATRIX_SIZE);
}

void Matrix4::setIdentity()
{
    memcpy(m, MATRIX_IDENTITY, MATRIX_SIZE);
}

void Matrix4::setZero()
{
    memset(m, 0, MATRIX_SIZE);
}

void Matrix4::subtract(const Matrix4& m)
{
    subtract(*this, m, this);
}

void Matrix4::subtract(const Matrix4& m1, const Matrix4& m2, Matrix4* dst)
{
    GP_ASSERT(dst);

    MathUtil::subtractMatrix(m1.m, m2.m, dst->m);
}

void Matrix4::transformPoint(Vector3* point) const
{
    GP_ASSERT(point);
    transformVector(point->x, point->y, point->z, 1.0f, point);
}

void Matrix4::transformPoint(const Vector3& point, Vector3* dst) const
{
    transformVector(point.x, point.y, point.z, 1.0f, dst);
}

void Matrix4::transformVector(Vector3* vector) const
{
    GP_ASSERT(vector);
    transformVector(vector->x, vector->y, vector->z, 0.0f, vector);
}

void Matrix4::transformVector(const Vector3& vector, Vector3* dst) const
{
    transformVector(vector.x, vector.y, vector.z, 0.0f, dst);
}

void Matrix4::transformVector(Float x, Float y, Float z, Float w, Vector3* dst) const
{
    GP_ASSERT(dst);

    MathUtil::transformVector4(m, x, y, z, w, (Float*)dst);
}

void Matrix4::transformVector(Vector4* vector) const
{
    GP_ASSERT(vector);
    transformVector(*vector, vector);
}

void Matrix4::transformVector(const Vector4& vector, Vector4* dst) const
{
    GP_ASSERT(dst);

    MathUtil::transformVector4(m, (const Float*) &vector, (Float*)dst);
}

void Matrix4::translate(Float x, Float y, Float z)
{
    translate(x, y, z, this);
}

void Matrix4::translate(Float x, Float y, Float z, Matrix4* dst) const
{
    Matrix4 t;
    createTranslation(x, y, z, &t);
    multiply(*this, t, dst);
}

void Matrix4::translate(const Vector3& t)
{
    translate(t.x, t.y, t.z, this);
}

void Matrix4::translate(const Vector3& t, Matrix4* dst) const
{
    translate(t.x, t.y, t.z, dst);
}

void Matrix4::transpose()
{
    transpose(this);
}

void Matrix4::transpose(Matrix4* dst) const
{
    GP_ASSERT(dst);

    MathUtil::transposeMatrix(m, dst->m);
}

Matrix4& Matrix4::operator=(const Matrix4& m)
{
    if(&m == this)
        return *this;

    std::memcpy(this->m, m.m, GP_MATH_MATRIX_SIZE);

    return *this;
}

bool Matrix4::operator==(const Matrix4& m) const
{
    return memcmp(this->m, m.m, GP_MATH_MATRIX_SIZE) == 0;
}

bool Matrix4::operator!=(const Matrix4& m) const
{
    return memcmp(this->m, m.m, GP_MATH_MATRIX_SIZE) != 0;
}

}
