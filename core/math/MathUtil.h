#ifndef MATHUTIL_H_
#define MATHUTIL_H_

#include "Math.h"

namespace mgp
{
/**
 * Defines a math utility class.
 *
 * This is primarily used for optimized internal math operations.
 */
class MathUtil
{
    friend class Matrix4;
    friend class Vector3;

public:

    /**
     * Updates the given scalar towards the given target using a smoothing function.
     * The given response time determines the amount of smoothing (lag). A longer
     * response time yields a smoother result and more lag. To force the scalar to
     * follow the target closely, provide a response time that is very small relative
     * to the given elapsed time.
     *
     * @param x the scalar to update.
     * @param target target value.
     * @param elapsedTime elapsed time between calls.
     * @param responseTime response time (in the same units as elapsedTime).
     */
    static void smooth(Float* x, Float target, Float elapsedTime, Float responseTime);

    /**
     * Updates the given scalar towards the given target using a smoothing function.
     * The given rise and fall times determine the amount of smoothing (lag). Longer
     * rise and fall times yield a smoother result and more lag. To force the scalar to
     * follow the target closely, provide rise and fall times that are very small relative
     * to the given elapsed time.
     *
     * @param x the scalar to update.
     * @param target target value.
     * @param elapsedTime elapsed time between calls.
     * @param riseTime response time for rising slope (in the same units as elapsedTime).
     * @param fallTime response time for falling slope (in the same units as elapsedTime).
     */
    static void smooth(Float* x, Float target, Float elapsedTime, Float riseTime, Float fallTime);

private:

    inline static void addMatrix(const Float* m, Float scalar, Float* dst);

    inline static void addMatrix(const Float* m1, const Float* m2, Float* dst);

    inline static void subtractMatrix(const Float* m1, const Float* m2, Float* dst);

    inline static void multiplyMatrix(const Float* m, Float scalar, Float* dst);

    inline static void multiplyMatrix(const Float* m1, const Float* m2, Float* dst);

    inline static void negateMatrix(const Float* m, Float* dst);

    inline static void transposeMatrix(const Float* m, Float* dst);

    inline static void transformVector4(const Float* m, Float x, Float y, Float z, Float w, Float* dst);

    inline static void transformVector4(const Float* m, const Float* v, Float* dst);

    inline static void crossVector3(const Float* v1, const Float* v2, Float* dst);

    MathUtil();
};

}

#define MATRIX_SIZE ( sizeof(Float) * 16)

#ifdef GP_USE_NEON
#include "MathUtilNeon.inl"
#else
#include "MathUtil.inl"
#endif

#endif
