#ifndef MATH_BASE_H_
#define MATH_BASE_H_

// Math
#define MATH_DEG_TO_RAD(x)          ((x) * 0.0174532925)
#define MATH_RAD_TO_DEG(x)          ((x)* 57.29577951)
#define MATH_RANDOM_MINUS1_1()      ((2.0*((Float)rand()/RAND_MAX))-1.0)      // Returns a random Float between -1 and 1.
#define MATH_RANDOM_0_1()           ((Float)rand()/RAND_MAX)                    // Returns a random Float between 0 and 1.
#define MATH_FLOAT_SMALL            1.0e-37
#define MATH_TOLERANCE              2e-37
#define MATH_E                      2.71828182845904523536
#define MATH_LOG10E                 0.4342944819032518
#define MATH_LOG2E                  1.442695040888963387
#define MATH_PI                     3.14159265358979323846
#define MATH_PIOVER2                1.57079632679489661923
#define MATH_PIOVER4                0.785398163397448309616
#define MATH_PIX2                   6.28318530717958647693
#define MATH_EPSILON                0.000001
#define MATH_CLAMP(x, lo, hi)       ((x < lo) ? lo : ((x > hi) ? hi : x))
#ifndef M_1_PI
    #define M_1_PI                      0.31830988618379067154
#endif

typedef double Float;

namespace mgp
{
  /**
   * power of 2 meaning 2^n.
   * e.g. 2,4,8,16,...
   */
  inline bool isPowerOf2(unsigned int x) {
    return !(x & (x-1));
  }

  inline unsigned int nextPowerOf2(unsigned int x) {
    if ( isPowerOf2(x) ) return x;
    x |= x>>1;
    x |= x>>2;
    x |= x>>4;
    x |= x>>8;
    x |= x>>16;
    return x+1;
  }
}

#endif //MATH_BASE_H_