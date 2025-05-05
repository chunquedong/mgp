// Purposely not including Base.h here, or any other gameplay dependencies, so it can be reused between gameplay and gameplay-encoder.
#include "Curve.h"
#include "Quaternion.h"
#include <cassert>
#include <cstring>
#include <cmath>
#include <memory>

using std::memcpy;
using std::fabs;
using std::sqrt;
using std::cos;
using std::sin;
using std::exp;
using std::strcmp;

#ifndef NULL
#define NULL 0
#endif

#ifndef MATH_PI
#define MATH_PI 3.14159265358979323846f
#endif

#ifndef MATH_PIOVER2 
#define MATH_PIOVER2 1.57079632679489661923f
#endif

#ifndef MATH_PIX2
#define MATH_PIX2 6.28318530717958647693f
#endif

// Object deletion macro
#ifndef SAFE_DELETE
#define SAFE_DELETE(x) \
    if (x) \
    { \
        delete x; \
        x = NULL; \
    }
#endif

// Array deletion macro
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) \
    if (x) \
    { \
        delete[] x; \
        x = NULL; \
    }
#endif

static inline Float bezier(Float eq0, Float eq1, Float eq2, Float eq3, Float from, Float out, Float to, Float in)
{
    return from * eq0 + out * eq1 + in * eq2 + to * eq3;
}

static inline Float bspline(Float eq0, Float eq1, Float eq2, Float eq3, Float c0, Float c1, Float c2, Float c3)
{
    return c0 * eq0 + c1 * eq1 + c2 * eq2 + c3 * eq3;
}

static inline Float hermite(Float h00, Float h01, Float h10, Float h11, Float from, Float out, Float to, Float in)
{
    return h00 * from + h01 * to + h10 * out + h11 * in;
}

static inline Float hermiteFlat(Float h00, Float h01, Float from, Float to)
{
    return h00 * from + h01 * to;
}

static inline Float hermiteSmooth(Float h00, Float h01, Float h10, Float h11, Float from, Float out, Float to, Float in)
{
    return h00 * from + h01 * to + h10 * out + h11 * in;
}

static inline Float lerpInl(Float s, Float from, Float to)
{
    return from + (to - from) * s;
}

namespace mgp
{

UPtr<Curve> Curve::create(unsigned int pointCount, unsigned int componentCount)
{
    return UPtr<Curve>(new Curve(pointCount, componentCount));
}

Curve::Curve(unsigned int pointCount, unsigned int componentCount)
    : _pointCount(pointCount), _componentCount(componentCount), _componentSize(sizeof(float)*componentCount), _quaternionOffset(NULL), _points(NULL)
{
    _points = new Point[_pointCount];
    for (unsigned int i = 0; i < _pointCount; i++)
    {
        _points[i].time = 0.0f;
        _points[i].value = new float[_componentCount];
        _points[i].inValue = new float[_componentCount];
        _points[i].outValue = new float[_componentCount];
        _points[i].type = LINEAR;
    }
    _points[_pointCount - 1].time = 1.0f;
}

Curve::~Curve()
{
    SAFE_DELETE_ARRAY(_points);
    SAFE_DELETE_ARRAY(_quaternionOffset);
}

Curve::Point::Point()
    : time(0.0f), value(NULL), inValue(NULL), outValue(NULL), type(LINEAR)
{
}

Curve::Point::~Point()
{
    SAFE_DELETE_ARRAY(value);
    SAFE_DELETE_ARRAY(inValue);
    SAFE_DELETE_ARRAY(outValue);
}

unsigned int Curve::getPointCount() const
{
    return _pointCount;
}

unsigned int Curve::getComponentCount() const
{
    return _componentCount;
}

Float Curve::getStartTime() const
{
    return _points[0].time;
}

Float Curve::getEndTime() const
{
    return _points[_pointCount-1].time;
}

Float Curve::getPointTime(unsigned int index) const
{
    assert(index < _pointCount);
    return _points[index].time;
}


Curve::InterpolationType Curve::getPointInterpolation(unsigned int index) const
{
    assert(index < _pointCount);
    return _points[index].type;;
}

void Curve::getPointValues(unsigned int index, float* value, float* inValue, float* outValue) const
{
    assert(index < _pointCount);
    
    if (value)
        memcpy(value, _points[index].value, _componentSize);
    
    if (inValue)
        memcpy(inValue, _points[index].inValue, _componentSize);
    
    if (outValue)
        memcpy(outValue, _points[index].outValue, _componentSize);
}

void Curve::setPoint(unsigned int index, Float time, float* value, InterpolationType type)
{
    setPoint(index, time, value, type, NULL, NULL);
}

void Curve::setPoint(unsigned int index, Float time, float* value, InterpolationType type, float* inValue, float* outValue)
{
    assert(index < _pointCount);
    assert(time >= 0.0f && time <= 1.0f);
    assert(!(_pointCount > 1 && index == 0 && time != 0.0f));
    assert(!(_pointCount != 1 && index == _pointCount - 1 && time != 1.0f));

    _points[index].time = time;
    _points[index].type = type;

    if (value)
        memcpy(_points[index].value, value, _componentSize);

    if (inValue)
        memcpy(_points[index].inValue, inValue, _componentSize);

    if (outValue)
        memcpy(_points[index].outValue, outValue, _componentSize);
}

void Curve::setTangent(unsigned int index, InterpolationType type, float* inValue, float* outValue)
{
    assert(index < _pointCount);

    _points[index].type = type;

    if (inValue)
        memcpy(_points[index].inValue, inValue, _componentSize);

    if (outValue)
        memcpy(_points[index].outValue, outValue, _componentSize);
}

void Curve::evaluate(Float time, Float* dst) const
{
    assert(dst);

    evaluate(time, 0.0f, 1.0f, 0.0f, dst);
}

static void copyFloatToDouble(Float* dst, float *value, int byteSize) {
    int componentSize = byteSize / 4;
    for (int i = 0; i < componentSize; ++i) {
        dst[i] = value[i];
    }
}

void Curve::evaluate(Float time, Float startTime, Float endTime, Float loopBlendTime, Float* dst) const
{
    assert(dst && startTime >= 0.0f && startTime <= endTime && endTime <= 1.0f && loopBlendTime >= 0.0f);

    // If there's only one point on the curve, return its value.
    if (_pointCount == 1)
    {
        copyFloatToDouble(dst, _points[0].value, _componentSize);
        return;
    }

    unsigned int min = 0;
    unsigned int max = _pointCount - 1;
    Float localTime = time;
    if (startTime > 0.0f || endTime < 1.0f)
    {
        // Evaluating a sub section of the curve
        min = determineIndex(startTime, 0, max);
        max = determineIndex(endTime, min, max);

        // Convert time to fall within the subregion
        localTime = _points[min].time + (_points[max].time - _points[min].time) * time;
    }

    if (loopBlendTime == 0.0f)
    {
        // If no loop blend time is specified, clamp time to end points
        if (localTime < _points[min].time)
            localTime = _points[min].time;
        else if (localTime > _points[max].time)
            localTime = _points[max].time;
    }

    // If an exact endpoint was specified, skip interpolation and return the value directly
    if (localTime == _points[min].time)
    {
        copyFloatToDouble(dst, _points[min].value, _componentSize);
        return;
    }
    if (localTime == _points[max].time)
    {
        copyFloatToDouble(dst, _points[max].value, _componentSize);
        return;
    }

    Point* from;
    Point* to;
    Float scale;
    Float t;
    unsigned int index;

    if (localTime > _points[max].time)
    {
        // Looping forward
        index = max;
        from = &_points[max];
        to = &_points[min];

        // Calculate the fractional time between the two points.
        t = (localTime - from->time) / loopBlendTime;
    }
    else if (localTime < _points[min].time)
    {
        // Looping in reverse
        index = min;
        from = &_points[min];
        to = &_points[max];

        // Calculate the fractional time between the two points.
        t = (from->time - localTime) / loopBlendTime;
    }
    else
    {
        // Locate the points we are interpolating between using a binary search.
        index = determineIndex(localTime, min, max);
        from = &_points[index];
        to = &_points[index == max ? index : index+1];

        // Calculate the fractional time between the two points.
        scale = (to->time - from->time);
        t = (localTime - from->time) / scale;
    }

    // Calculate the value of the curve discretely if appropriate.
    switch (from->type)
    {
        case BEZIER:
        {
            interpolateBezier(t, from, to, dst);
            return;
        }
        case BSPLINE:
        {
            Point* c0;
            Point* c1;
            if (index == 0)
            {
                c0 = from;
            }
            else
            {
                c0 = (_points + index - 1);
            }
            
            if (index == _pointCount - 2)
            {
                c1 = to;
            }
            else
            {
                c1 = (_points + index + 2);
            }
            interpolateBSpline(t, c0, from, to, c1, dst);
            return;
        }
        case FLAT:
        {
            interpolateHermiteFlat(t, from, to, dst);
            return;
        }
        case HERMITE:
        {
            interpolateHermite(t, from, to, dst);
            return;
        }
        case LINEAR:
        {
            // Can just break here because linear formula follows switch
            break;
        }
        case SMOOTH:
        {
            interpolateHermiteSmooth(t, index, from, to, dst);
            return;
        }
        case STEP:
        {
            copyFloatToDouble(dst, from->value, _componentSize);
            return;
        }
        case QUADRATIC_IN:
        {
            t *= t;
            break;
        }
        case QUADRATIC_OUT:
        {
            t *= -(t - 2.0f);
            break;
        }
        case QUADRATIC_IN_OUT:
        {
            Float tx2 = t * 2.0f;

            if (tx2 < 1.0f)
                t = 0.5f * (tx2 * tx2);
            else
            {
                Float temp = tx2 - 1.0f;
                t = 0.5f * (-( temp * (temp - 2.0f)) + 1.0f);
            }
            break;
        }
        case QUADRATIC_OUT_IN:
        {
            if (t < 0.5f)
            {
                t = 2.0f * t * (1.0f - t);
            }
            else
            {
                t = 1.0f + 2.0f * t * (t - 1.0f);
            }
            break;
        }
        case CUBIC_IN:
        {
            t *= t * t;
            break;
        }
        case CUBIC_OUT:
        {
            t--;
            t = t * t * t + 1;
            break;
        }
        case CUBIC_IN_OUT:
        {
            if ((t *= 2.0f) < 1.0f)
            {
                t = t * t * t * 0.5f;
            }
            else
            {
                t -= 2.0f;
                t = (t * t * t + 2.0f) * 0.5f;
            }
            break;
        }
        case CUBIC_OUT_IN:
        {
            t = (2.0f * t - 1.0f);
            t = (t * t * t + 1) * 0.5f;
            break;
        }
        case QUARTIC_IN:
        {
            t *= t * t * t;
            break;
        }
        case QUARTIC_OUT:
        {
            t--;
            t = -(t * t * t * t) + 1.0f;
            break;
        }
        case QUARTIC_IN_OUT:
        {
            t *= 2.0f;
            if (t < 1.0f)
            {
                t = 0.5f * t * t * t * t;
            }
            else
            {
                t -= 2.0f;
                t = -0.5f * (t * t * t * t - 2.0f);
            }
            break;
        }
        case QUARTIC_OUT_IN:
        {
            t = 2.0f * t - 1.0f;
            if (t < 0.0f)
            {
                t = 0.5f * (-(t * t) * t * t + 1.0f);
            }
            else
            {
                t = 0.5f * (t * t * t * t + 1.0f);
            }
            break;
        }
        case QUINTIC_IN:
        {
            t *= t * t * t * t;
            break;
        }
        case QUINTIC_OUT:
        {
            t--;
            t = t * t * t * t * t + 1.0f;
            break;
        }
        case QUINTIC_IN_OUT:
        {
            t *= 2.0f;
            if (t < 1.0f)
            {
                t = 0.5f * t * t * t * t * t;
            }
            else
            {
                t -= 2.0f;
                t = 0.5f * (t * t * t * t * t + 2.0f);
            }
            break;
        }
        case QUINTIC_OUT_IN:
        {
            t = 2.0f * t - 1.0f;
            t = 0.5f * (t * t * t * t * t + 1.0f);
            break;
        }
        case SINE_IN:
        {
            t = -(cos(t * MATH_PIOVER2) - 1.0f);
            break;
        }
        case SINE_OUT:
        {
            t = sin(t * MATH_PIOVER2);
            break;
        }
        case SINE_IN_OUT:
        {
            t = -0.5f * (cos(MATH_PI * t) - 1.0f);
            break;
        }
        case SINE_OUT_IN:
        {
            if (t < 0.5f)
            {
                t = 0.5f * sin(MATH_PI * t);
            }
            else
            {
                t = -0.5f * cos(MATH_PIOVER2 * (2.0f * t - 1.0f)) + 1.0f;
            }
            break;
        }
        case EXPONENTIAL_IN:
        {
            if (t != 0.0f)
            {
                t = exp(10.0f * (t - 1.0f));
            }
            break;
        }
        case EXPONENTIAL_OUT:
        {
            if (t != 1.0f)
            {
                t = -exp(-10.0f * t) + 1.0f;
            }
            break;
        }
        case EXPONENTIAL_IN_OUT:
        {
            if (t != 0.0f && t != 1.0f)
            {
                if (t < 0.5f)
                {
                    t = 0.5f * exp(10.0f * (2.0f * t - 1.0f));
                }
                else
                {
                    t = -0.5f * exp(10.0f * (-2.0f * t + 1.0f)) + 1.0f;
                }
            }
            break;
        }
        case EXPONENTIAL_OUT_IN:
        {
            if (t != 0.0f && t != 1.0f)
            {
                if (t < 0.5f)
                {
                    t = -0.5f * exp(-20.0f * t) + 0.5f;
                }
                else
                {
                    t = 0.5f * exp(20.0f * (t - 1.0f)) + 0.5f;
                }
            }
            break;
        }
        case CIRCULAR_IN:
        {
            t = -(sqrt(1.0f - t * t) - 1.0f);
            break;
        }
        case CIRCULAR_OUT:
        {
            t--;
            t = sqrt(1.0f - t * t);
            break;
        }
        case CIRCULAR_IN_OUT:
        {
            t *= 2.0f;
            if (t < 1.0f)
            {
                t = 0.5f * (-sqrt((1.0f - t * t)) + 1.0f);
            }
            else
            {
                t -= 2.0f;
                t = 0.5f * (sqrt((1.0f - t * t)) + 1.0f);
            }
            break;
        }
        case CIRCULAR_OUT_IN:
        {
            t = 2.0f * t - 1.0f;
            if (t < 0.0f)
            {
                t = 0.5f * sqrt(1.0f - t * t);
            }
            else
            {
                t = 0.5f * (2.0f - sqrt(1.0f - t * t));
            }
            break;
        }
        case ELASTIC_IN:
        {
            if (t != 0.0f && t != 1.0f)
            {
                t = t - 1.0f;
                t = -1.0f * ( exp(10.0f * t) * sin( (t - 0.075f) * MATH_PIX2 / 0.3f ) );
            }
            break;
        }
        case ELASTIC_OUT:
        {
            if (t != 0.0f && t != 1.0f)
            {
                t = exp(-10.0f * t) * sin((t - 0.075f) * MATH_PIX2 / 0.3f) + 1.0f;
            }
            break;
        }
        case ELASTIC_IN_OUT:
        {
            if (t != 0.0f && t != 1.0f)
            {
                t = 2.0f * t - 1.0f;
                if (t < 0.0f)
                {
                    t = -0.5f * (exp((10 * t)) * sin(((t - 0.1125f) * MATH_PIX2 / 0.45f)));
                }
                else
                {
                    t = 0.5f * exp((-10 * t)) * sin(((t - 0.1125f) * MATH_PIX2 / 0.45f)) + 1.0f;
                }
            }
            break;
        }
        case ELASTIC_OUT_IN:
        {
            if (t != 0.0f && t != 1.0f)
            {
                t *= 2.0f;
                if (t < 1.0f)
                {
                    t = 0.5f * (exp((-10 * t)) * sin(((t - 0.1125f) * (MATH_PIX2) / 0.45f))) + 0.5f;
                }
                else
                {
                    t = 0.5f * (exp((10 *(t - 2))) * sin(((t - 0.1125f) * (MATH_PIX2) / 0.45f))) + 0.5f;
                }
            }
            break;
        }
        case OVERSHOOT_IN:
        {
            t = t * t * (2.70158f * t - 1.70158f);
            break;
        }
        case OVERSHOOT_OUT:
        {
            t--;
            t = t * t * (2.70158f * t + 1.70158f) + 1;
            break;
        }
        case OVERSHOOT_IN_OUT:
        {
            t *= 2.0f;
            if (t < 1.0f)
            {
                t = 0.5f * t * t * (3.5949095f * t - 2.5949095f);
            }
            else
            {
                t -= 2.0f;
                t = 0.5f * (t * t * (3.5949095f * t + 2.5949095f) + 2.0f);
            }
            break;
        }
        case OVERSHOOT_OUT_IN:
        {
            t = 2.0f * t - 1.0f;
            if (t < 0.0f)
            {
                t = 0.5f * (t * t * (3.5949095f * t + 2.5949095f) + 1.0f);
            }
            else
            {
                t = 0.5f * (t * t * (3.5949095f * t - 2.5949095f) + 1.0f);
            }
            break;
        }
        case BOUNCE_IN:
        {
            t = 1.0f - t;

            if (t < 0.36363636363636365f)
            {
                t = 7.5625f * t * t;
            }
            else if (t < 0.7272727272727273f)
            {
                t -= 0.5454545454545454f;
                t = 7.5625f * t * t + 0.75f;
            }
            else if (t < 0.9090909090909091f)
            {
                t -= 0.8181818181818182f;
                t = 7.5625f * t * t + 0.9375f;
            }
            else
            {
                t -= 0.9545454545454546f;
                t = 7.5625f * t * t + 0.984375f;
            }

            t = 1.0f - t;
            break;
        }
        case BOUNCE_OUT:
        {
            if (t < 0.36363636363636365f)
            {
                t = 7.5625f * t * t;
            }
            else if (t < 0.7272727272727273f)
            {
                t -= 0.5454545454545454f;
                t = 7.5625f * t * t + 0.75f;
            }
            else if (t < 0.9090909090909091f)
            {
                t -= 0.8181818181818182f;
                t = 7.5625f * t * t + 0.9375f;
            }
            else
            {
                t -= 0.9545454545454546f;
                t = 7.5625f * t * t + 0.984375f;
            }
            break;
        }
        case BOUNCE_IN_OUT:
        {
            if (t < 0.5f)
            {
                t = 1.0f - t * 2.0f;

                if (t < 0.36363636363636365f)
                {
                    t = 7.5625f * t * t;
                }
                else if (t < 0.7272727272727273f)
                {
                    t -= 0.5454545454545454f;
                    t = 7.5625f * t * t + 0.75f;
                }
                else if (t < 0.9090909090909091f)
                {
                    t -= 0.8181818181818182f;
                    t = 7.5625f * t * t + 0.9375f;
                }
                else
                {
                    t -= 0.9545454545454546f;
                    t = 7.5625f * t * t + 0.984375f;
                }

                t = (1.0f - t) * 0.5f;
            }
            else
            {
                t = t * 2.0f - 1.0f;
                if (t < 0.36363636363636365f)
                {
                    t = 7.5625f * t * t;
                }
                else if (t < 0.7272727272727273f)
                {
                    t -= 0.5454545454545454f;
                    t = 7.5625f * t * t + 0.75f;
                }
                else if (t < 0.9090909090909091f)
                {
                    t -= 0.8181818181818182f;
                    t = 7.5625f * t * t + 0.9375f;
                }
                else
                {
                    t -= 0.9545454545454546f;
                    t = 7.5625f * t * t + 0.984375f;
                }

                t = 0.5f * t + 0.5f;
            }
            break;
        }
        case BOUNCE_OUT_IN:
        {
            if (t < 0.1818181818f)
            {
                t = 15.125f * t * t;
            }
            else if (t < 0.3636363636f)
            {
                t = 1.5f + (-8.250000001f + 15.125f * t) * t;
            }
            else if (t < 0.4545454546f)
            {
                t = 3.0f + (-12.375f + 15.125f * t) * t;
            }
            else if (t < 0.5f)
            {
                t = 3.9375f + (-14.4375f + 15.125f * t) * t;
            }
            else if (t <= 0.5454545455f)
            {
                t = -3.625000004f + (15.81250001f - 15.125f * t) * t;
            }
            else if (t <= 0.6363636365f)
            {
                t = -4.75f + (17.875f - 15.125f * t) * t;
            }
            else if (t <= 0.8181818180f)
            {
                t = -7.374999995f + (21.99999999f - 15.125f * t) * t;
            }
            else
            {
                t = -14.125f + (30.25f - 15.125f * t) * t;
            }
            break;
        }
    }

    interpolateLinear(t, from, to, dst);
}

Float Curve::lerp(Float t, Float from, Float to)
{
    return lerpInl(t, from, to);
}

void Curve::setQuaternionOffset(unsigned int offset)
{
    assert(offset <= (_componentCount - 4));

    if (!_quaternionOffset)
        _quaternionOffset = new unsigned int[1];
    
    *_quaternionOffset = offset;
}

void Curve::interpolateBezier(Float s, Point* from, Point* to, Float* dst) const
{
    Float s_2 = s * s;
    Float eq0 = 1 - s;
    Float eq0_2 = eq0 * eq0;
    Float eq1 = eq0_2 * eq0;
    Float eq2 = 3 * s * eq0_2;
    Float eq3 = 3 * s_2 * eq0;
    Float eq4 = s_2 * s;

    float* fromValue = from->value;
    float* toValue = to->value;
    float* outValue = from->outValue;
    float* inValue = to->inValue;


    if (!_quaternionOffset)
    {
        for (unsigned int i = 0; i < _componentCount; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = bezier(eq1, eq2, eq3, eq4, fromValue[i], outValue[i], toValue[i], inValue[i]);
        }
    }
    else
    {
        // Interpolate any values up to the quaternion offset as scalars.
        unsigned int quaternionOffset = *_quaternionOffset;
        unsigned int i = 0;
        for (i = 0; i < quaternionOffset; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = bezier(eq1, eq2, eq3, eq4, fromValue[i], outValue[i], toValue[i], inValue[i]);
        }

        // Handle quaternion component.
        Float interpTime = bezier(eq1, eq2, eq3, eq4, from->time, outValue[i], to->time, inValue[i]);
        interpolateQuaternion(interpTime, (fromValue + i), (toValue + i), (dst + i));
        
        // Handle remaining components (if any) as scalars
        for (i += 4; i < _componentCount; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = bezier(eq1, eq2, eq3, eq4, fromValue[i], outValue[i], toValue[i], inValue[i]);
        }
    }
}

void Curve::interpolateBSpline(Float s, Point* c0, Point* c1, Point* c2, Point* c3, Float* dst) const
{   
    Float s_2 = s * s;
    Float s_3 = s_2 * s;
    Float eq0 = (-s_3 + 3 * s_2 - 3 * s + 1) / 6.0f;
    Float eq1 = (3 * s_3 - 6 * s_2 + 4) / 6.0f;
    Float eq2 = (-3 * s_3 + 3 * s_2 + 3 * s + 1) / 6.0f;
    Float eq3 = s_3 / 6.0f;

    float* c0Value = c0->value;
    float* c1Value = c1->value;
    float* c2Value = c2->value;
    float* c3Value = c3->value;

    if (!_quaternionOffset)
    {
        for (unsigned int i = 0; i < _componentCount; i++)
        {
            if (c1Value[i] == c2Value[i])
                dst[i] = c1Value[i];
            else
                dst[i] = bspline(eq0, eq1, eq2, eq3, c0Value[i], c1Value[i], c2Value[i], c3Value[i]);
        }
    }
    else
    {
        // Interpolate any values up to the quaternion offset as scalars.
        unsigned int quaternionOffset = *_quaternionOffset;
        unsigned int i = 0;
        for (i = 0; i < quaternionOffset; i++)
        {
            if (c1Value[i] == c2Value[i])
                dst[i] = c1Value[i];
            else
                dst[i] = bspline(eq0, eq1, eq2, eq3, c0Value[i], c1Value[i], c2Value[i], c3Value[i]);
        }

        // Handle quaternion component.
        Float interpTime;
        if (c0->time == c1->time)
            interpTime = bspline(eq0, eq1, eq2, eq3, -c0->time, c1->time, c2->time, c3->time);
        else if (c2->time == c3->time)
            interpTime = bspline(eq0, eq1, eq2, eq3, c0->time, c1->time, c2->time, -c3->time); 
        else
            interpTime = bspline(eq0, eq1, eq2, eq3, c0->time, c1->time, c2->time, c3->time);
        interpolateQuaternion(s, (c1Value + i) , (c2Value + i), (dst + i));
            
        // Handle remaining components (if any) as scalars
        for (i += 4; i < _componentCount; i++)
        {
            if (c1Value[i] == c2Value[i])
                dst[i] = c1Value[i];
            else
                dst[i] = bspline(eq0, eq1, eq2, eq3, c0Value[i], c1Value[i], c2Value[i], c3Value[i]);
        }
    }
}

void Curve::interpolateHermite(Float s, Point* from, Point* to, Float* dst) const
{
    // Calculate the hermite basis functions.
    Float s_2 = s * s;                   // t^2
    Float s_3 = s_2 * s;                 // t^3
    Float h00 = 2 * s_3 - 3 * s_2 + 1;   // basis function 0
    Float h01 = -2 * s_3 + 3 * s_2;      // basis function 1
    Float h10 = s_3 - 2 * s_2 + s;       // basis function 2
    Float h11 = s_3 - s_2;               // basis function 3

    float* fromValue = from->value;
    float* toValue = to->value;
    float* outValue = from->outValue;
    float* inValue = to->inValue;

    if (!_quaternionOffset)
    {
        for (unsigned int i = 0; i < _componentCount; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = hermite(h00, h01, h10, h11, fromValue[i], outValue[i], toValue[i], inValue[i]);
        }
    }
    else
    {
        // Interpolate any values up to the quaternion offset as scalars.
        unsigned int quaternionOffset = *_quaternionOffset;
        unsigned int i = 0;
        for (i = 0; i < quaternionOffset; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = hermite(h00, h01, h10, h11, fromValue[i], outValue[i], toValue[i], inValue[i]);
        }

        // Handle quaternion component.
        Float interpTime = hermite(h00, h01, h10, h11, from->time, outValue[i], to->time, inValue[i]);
        interpolateQuaternion(interpTime, (fromValue + i), (toValue + i), (dst + i));
        
        // Handle remaining components (if any) as scalars
        for (i += 4; i < _componentCount; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = hermite(h00, h01, h10, h11, fromValue[i], outValue[i], toValue[i], inValue[i]);
        }
    }
}

void Curve::interpolateHermiteFlat(Float s, Point* from, Point* to, Float* dst) const
{
    // Calculate the hermite basis functions.
    Float s_2 = s * s;                   // t^2
    Float s_3 = s_2 * s;                 // t^3
    Float h00 = 2 * s_3 - 3 * s_2 + 1;   // basis function 0
    Float h01 = -2 * s_3 + 3 * s_2;      // basis function 1

    float* fromValue = from->value;
    float* toValue = to->value;

    if (!_quaternionOffset)
    {
        for (unsigned int i = 0; i < _componentCount; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = hermiteFlat(h00, h01, fromValue[i], toValue[i]);
        }
    }
    else
    {
        // Interpolate any values up to the quaternion offset as scalars.
        unsigned int quaternionOffset = *_quaternionOffset;
        unsigned int i = 0;
        for (i = 0; i < quaternionOffset; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = hermiteFlat(h00, h01, fromValue[i], toValue[i]);
        }

        // Handle quaternion component.
        Float interpTime = hermiteFlat(h00, h01, from->time, to->time);
        interpolateQuaternion(interpTime, (fromValue + i), (toValue + i), (dst + i));
        
        // Handle remaining components (if any) as scalars
        for (i += 4; i < _componentCount; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = hermiteFlat(h00, h01, fromValue[i], toValue[i]);
        }
    }
}

void Curve::interpolateHermiteSmooth(Float s, unsigned int index, Point* from, Point* to, Float* dst) const
{
    // Calculate the hermite basis functions.
    Float s_2 = s * s;                   // t^2
    Float s_3 = s_2 * s;                 // t^3
    Float h00 = 2 * s_3 - 3 * s_2 + 1;   // basis function 0
    Float h01 = -2 * s_3 + 3 * s_2;      // basis function 1
    Float h10 = s_3 - 2 * s_2 + s;       // basis function 2
    Float h11 = s_3 - s_2;               // basis function 3

    Float inValue;
    Float outValue;

    float* fromValue = from->value;
    float* toValue = to->value;

    if (!_quaternionOffset)
    {
        for (unsigned int i = 0; i < _componentCount; i++)
        {
            if (fromValue[i] == toValue[i])
            {
                dst[i] = fromValue[i];
            }
            else
            {
                if (index == 0)
                {
                    outValue = toValue[i] - fromValue[i];
                }
                else
                {
                    outValue = (toValue[i] - (from - 1)->value[i]) * ((from->time - (from - 1)->time) / (to->time - (from - 1)->time));
                }

                if (index == _pointCount - 2)
                {
                    inValue = toValue[i] - fromValue[i];
                }
                else
                {
                    inValue = ((to + 1)->value[i] - fromValue[i]) * ((to->time - from->time) / ((to + 1)->time - from->time));
                }

                dst[i] = hermiteSmooth(h00, h01, h10, h11, fromValue[i], outValue, toValue[i], inValue);
            }
        }
    }
    else
    {
        // Interpolate any values up to the quaternion offset as scalars.
        unsigned int quaternionOffset = *_quaternionOffset;
        unsigned int i = 0;
        for (i = 0; i < quaternionOffset; i++)
        {   
            if (fromValue[i] == toValue[i])
            {
                dst[i] = fromValue[i];
            }
            else
            {    
                if (index == 0)
                {
                    outValue = toValue[i] - fromValue[i];
                }
                else
                {
                    outValue = (toValue[i] - (from - 1)->value[i]) * ((from->time - (from - 1)->time) / (to->time - (from - 1)->time));
                }

                if (index == _pointCount - 2)
                {
                    inValue = toValue[i] - fromValue[i];
                }
                else
                {
                    inValue = ((to + 1)->value[i] - fromValue[i]) * ((to->time - from->time) / ((to + 1)->time - from->time));
                }

                dst[i] = hermiteSmooth(h00, h01, h10, h11, fromValue[i], outValue, toValue[i], inValue);
            }
        }

        // Handle quaternion component.
        if (index == 0)
        {
            outValue = to->time - from->time;
        }
        else
        {
            outValue = (to->time - (from - 1)->time) * ((from->time - (from - 1)->time) / (to->time - (from - 1)->time));
        }

        if (index == _pointCount - 2)
        {
            inValue = to->time - from->time;
        }
        else
        {
            inValue = ((to + 1)->time - from->time) * ((to->time - from->time) / ((to + 1)->time - from->time));
        }

        Float interpTime = hermiteSmooth(h00, h01, h10, h11, from->time, outValue, to->time, inValue);
        interpolateQuaternion(interpTime, (fromValue + i), (toValue + i), (dst + i));
        
        // Handle remaining components (if any) as scalars
        for (i += 4; i < _componentCount; i++)
        {
            if (fromValue[i] == toValue[i])
            {
                dst[i] = fromValue[i];
            }
            else
            {
                // Interpolate as scalar.
                if (index == 0)
                {
                    outValue = toValue[i] - fromValue[i];
                }
                else
                {
                    outValue = (toValue[i] - (from - 1)->value[i]) * ((from->time - (from - 1)->time) / (to->time - (from - 1)->time));
                }

                if (index == _pointCount - 2)
                {
                    inValue = toValue[i] - fromValue[i];
                }
                else
                {
                    inValue = ((to + 1)->value[i] - fromValue[i]) * ((to->time - from->time) / ((to + 1)->time - from->time));
                }

                dst[i] = hermiteSmooth(h00, h01, h10, h11, fromValue[i], outValue, toValue[i], inValue);
            }
        }
    }
}

void Curve::interpolateLinear(Float s, Point* from, Point* to, Float* dst) const
{
    float* fromValue = from->value;
    float* toValue = to->value;

    if (!_quaternionOffset)
    {
        for (unsigned int i = 0; i < _componentCount; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = lerpInl(s, fromValue[i], toValue[i]);
        }
    }
    else
    {
        // Interpolate any values up to the quaternion offset as scalars.
        unsigned int quaternionOffset = *_quaternionOffset;
        unsigned int i = 0;
        for (i = 0; i < quaternionOffset; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = lerpInl(s, fromValue[i], toValue[i]);
        }

        // Handle quaternion component.
        interpolateQuaternion(s, (fromValue + i), (toValue + i), (dst + i));
        
        // handle any remaining components as scalars
        for (i += 4; i < _componentCount; i++)
        {
            if (fromValue[i] == toValue[i])
                dst[i] = fromValue[i];
            else
                dst[i] = lerpInl(s, fromValue[i], toValue[i]);
        }
    }
}

void Curve::interpolateQuaternion(Float s, float* from, float* to, Float* dst) const
{
    // Evaluate.
    if (s >= 0)
        Quaternion::slerp(from[0], from[1], from[2], from[3], to[0], to[1], to[2], to[3], s, dst, dst + 1, dst + 2, dst + 3);
    else
        Quaternion::slerp(to[0], to[1], to[2], to[3], from[0], from[1], from[2], from[3], s, dst, dst + 1, dst + 2, dst + 3);
}

int Curve::determineIndex(Float time, unsigned int min, unsigned int max) const
{
    unsigned int mid;

    // Do a binary search to determine the index.
    do 
    {
        mid = (min + max) >> 1;

        if (time >= _points[mid].time && time < _points[mid + 1].time)
            return mid;
        else if (time < _points[mid].time)
            max = mid - 1;
        else
            min = mid + 1;
    } while (min <= max);
    
    return max;
}

int Curve::getInterpolationType(const char* curveId)
{
    if (strcmp(curveId, "BEZIER") == 0)
    {
        return Curve::BEZIER;
    }
    else if (strcmp(curveId, "BSPLINE") == 0)
    {
        return Curve::BSPLINE;
    }
    else if (strcmp(curveId, "FLAT") == 0)
    {
        return Curve::FLAT;
    }
    else if (strcmp(curveId, "HERMITE") == 0)
    {
        return Curve::HERMITE;
    }
    else if (strcmp(curveId, "LINEAR") == 0)
    {
        return Curve::LINEAR;
    }
    else if (strcmp(curveId, "SMOOTH") == 0)
    {
        return Curve::SMOOTH;
    }
    else if (strcmp(curveId, "STEP") == 0)
    {
        return Curve::STEP;
    }
    else if (strcmp(curveId, "QUADRATIC_IN") == 0)
    {
        return Curve::QUADRATIC_IN;
    }
    else if (strcmp(curveId, "QUADRATIC_OUT") == 0)
    {
        return Curve::QUADRATIC_OUT;
    }
    else if (strcmp(curveId, "QUADRATIC_IN_OUT") == 0)
    {
        return Curve::QUADRATIC_IN_OUT;
    }
    else if (strcmp(curveId, "QUADRATIC_OUT_IN") == 0)
    {
        return Curve::QUADRATIC_OUT_IN;
    }
    else if (strcmp(curveId, "CUBIC_IN") == 0)
    {
        return Curve::CUBIC_IN;
    }
    else if (strcmp(curveId, "CUBIC_OUT") == 0)
    {
        return Curve::CUBIC_OUT;
    }
    else if (strcmp(curveId, "CUBIC_IN_OUT") == 0)
    {
        return Curve::CUBIC_IN_OUT;
    }
    else if (strcmp(curveId, "CUBIC_OUT_IN") == 0)
    {
        return Curve::CUBIC_OUT_IN;
    }
    else if (strcmp(curveId, "QUARTIC_IN") == 0)
    {
        return Curve::QUARTIC_IN;
    }
    else if (strcmp(curveId, "QUARTIC_OUT") == 0)
    {
        return Curve::QUARTIC_OUT;
    }
    else if (strcmp(curveId, "QUARTIC_IN_OUT") == 0)
    {
        return Curve::QUARTIC_IN_OUT;
    }
    else if (strcmp(curveId, "QUARTIC_OUT_IN") == 0)
    {
        return Curve::QUARTIC_OUT_IN;
    }
    else if (strcmp(curveId, "QUINTIC_IN") == 0)
    {
        return Curve::QUINTIC_IN;
    }
    else if (strcmp(curveId, "QUINTIC_OUT") == 0)
    {
        return Curve::QUINTIC_OUT;
    }
    else if (strcmp(curveId, "QUINTIC_IN_OUT") == 0)
    {
        return Curve::QUINTIC_IN_OUT;
    }
    else if (strcmp(curveId, "QUINTIC_OUT_IN") == 0)
    {
        return Curve::QUINTIC_OUT_IN;
    }
    else if (strcmp(curveId, "SINE_IN") == 0)
    {
        return Curve::SINE_IN;
    }
    else if (strcmp(curveId, "SINE_OUT") == 0)
    {
        return Curve::SINE_OUT;
    }
    else if (strcmp(curveId, "SINE_IN_OUT") == 0)
    {
        return Curve::SINE_IN_OUT;
    }
    else if (strcmp(curveId, "SINE_OUT_IN") == 0)
    {
        return Curve::SINE_OUT_IN;
    }
    else if (strcmp(curveId, "EXPONENTIAL_IN") == 0)
    {
        return Curve::EXPONENTIAL_IN;
    }
    else if (strcmp(curveId, "EXPONENTIAL_OUT") == 0)
    {
        return Curve::EXPONENTIAL_OUT;
    }
    else if (strcmp(curveId, "EXPONENTIAL_IN_OUT") == 0)
    {
        return Curve::EXPONENTIAL_IN_OUT;
    }
    else if (strcmp(curveId, "EXPONENTIAL_OUT_IN") == 0)
    {
        return Curve::EXPONENTIAL_OUT_IN;
    }
    else if (strcmp(curveId, "CIRCULAR_IN") == 0)
    {
        return Curve::CIRCULAR_IN;
    }
    else if (strcmp(curveId, "CIRCULAR_OUT") == 0)
    {
        return Curve::CIRCULAR_OUT;
    }
    else if (strcmp(curveId, "CIRCULAR_IN_OUT") == 0)
    {
        return Curve::CIRCULAR_IN_OUT;
    }
    else if (strcmp(curveId, "CIRCULAR_OUT_IN") == 0)
    {
        return Curve::CIRCULAR_OUT_IN;
    }
    else if (strcmp(curveId, "ELASTIC_IN") == 0)
    {
        return Curve::ELASTIC_IN;
    }
    else if (strcmp(curveId, "ELASTIC_OUT") == 0)
    {
        return Curve::ELASTIC_OUT;
    }
    else if (strcmp(curveId, "ELASTIC_IN_OUT") == 0)
    {
        return Curve::ELASTIC_IN_OUT;
    }
    else if (strcmp(curveId, "ELASTIC_OUT_IN") == 0)
    {
        return Curve::ELASTIC_OUT_IN;
    }
    else if (strcmp(curveId, "OVERSHOOT_IN") == 0)
    {
        return Curve::OVERSHOOT_IN;
    }
    else if (strcmp(curveId, "OVERSHOOT_OUT") == 0)
    {
        return Curve::OVERSHOOT_OUT;
    }
    else if (strcmp(curveId, "OVERSHOOT_IN_OUT") == 0)
    {
        return Curve::OVERSHOOT_IN_OUT;
    }
    else if (strcmp(curveId, "OVERSHOOT_OUT_IN") == 0)
    {
        return Curve::OVERSHOOT_OUT_IN;
    }
    else if (strcmp(curveId, "BOUNCE_IN") == 0)
    {
        return Curve::BOUNCE_IN;
    }
    else if (strcmp(curveId, "BOUNCE_OUT") == 0)
    {
        return Curve::BOUNCE_OUT;
    }
    else if (strcmp(curveId, "BOUNCE_IN_OUT") == 0)
    {
        return Curve::BOUNCE_IN_OUT;
    }
    else if (strcmp(curveId, "BOUNCE_OUT_IN") == 0)
    {
        return Curve::BOUNCE_OUT_IN;
    }

    return -1;
}

Vector3 catmullRomSpline(const Vector3& p0, const Vector3& p1,
    const Vector3& p2, const Vector3& p3,
    double t, double tau) {
    double t2 = t * t;
    double t3 = t2 * t;

    Vector3 result;
    result.x = p1.x + tau * (-p0.x + p2.x) * t +
        tau * (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 +
        tau * (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3;

    result.y = p1.y + tau * (-p0.y + p2.y) * t +
        tau * (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 +
        tau * (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3;

    result.z = p1.z + tau * (-p0.z + p2.z) * t +
        tau * (2 * p0.z - 5 * p1.z + 4 * p2.z - p3.z) * t2 +
        tau * (-p0.z + 3 * p1.z - 3 * p2.z + p3.z) * t3;
    return result;
}

Vector3 bezierCurve(const Vector3& p0, const Vector3& p1,
    const Vector3& p2, const Vector3& p3,
    double t, double tau) {

    double st = t;
    double dt = (1 - st);

    double st2 = st * st;
    double dt2 = dt * dt;

    double t0 = dt * dt2;
    double t1 = dt2 * st * 3;
    double t2 = dt * st2 * 3;
    double t3 = st * st2;

    Vector3 p = p0 * t0 + p1 * t1 + p2 * t2 + p3 * t3;
    return p;
}

}
