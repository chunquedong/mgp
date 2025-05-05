#ifndef CURVE_H_
#define CURVE_H_

#include "base/Ref.h"
#include "Math.h"
#include "base/Ptr.h"
#include "Vector3.h"

namespace mgp
{

/**
 * Defines an n-dimensional curve.
 */
class Curve : public Refable
{
    friend class AnimationTarget;
    friend class Animation;
    friend class AnimationClip;
    friend class AnimationController;
    friend class MeshSkin;

public:

    /**
     * Types of interpolation.
     *
     * Defines how the points in the curve are connected.
     *
     * Note: InterpolationType::BEZIER requires control points and InterpolationType::HERMITE requires tangents.
     */
    enum InterpolationType
    {
        /**
         * Bezier Interpolation.
         *
         * Requires that two control points are set for each segment.
         */
        BEZIER,

        /**
         * B-Spline Interpolation.
         *
         * Uses the points as control points, and the curve is guaranteed to only pass through the
         * first and last point.
         */
        BSPLINE,

        /**
         * Flat Interpolation.
         *
         * A form of Hermite interpolation that generates flat tangents for you. The tangents have a value equal to 0.
         */
        FLAT,

        /**
         * Hermite Interpolation.
         *
         * Requires that two tangents for each segment.
         */
        HERMITE,

        /**
         * Linear Interpolation.
         */
        LINEAR,

        /**
         * Smooth Interpolation.
         *
         * A form of Hermite interpolation that generates tangents for each segment based on the points prior to and after the segment.
         */
        SMOOTH,

        /**
         * Discrete Interpolation.
         */
        STEP,

        /**
         * Quadratic-In Interpolation.
         */
        QUADRATIC_IN,

        /**
         * Quadratic-Out Interpolation.
         */
        QUADRATIC_OUT,

        /**
         * Quadratic-In-Out Interpolation.
         */
        QUADRATIC_IN_OUT,

        /**
         * Quadratic-Out-In Interpolation.
         */
        QUADRATIC_OUT_IN,

        /**
         * Cubic-In Interpolation.
         */
        CUBIC_IN,

        /**
         * Cubic-Out Interpolation.
         */
        CUBIC_OUT,

        /**
         * Cubic-In-Out Interpolation.
         */
        CUBIC_IN_OUT,

        /**
         * Cubic-Out-In Interpolation.
         */
        CUBIC_OUT_IN,

        /**
         * Quartic-In Interpolation.
         */
        QUARTIC_IN,

        /**
         * Quartic-Out Interpolation.
         */
        QUARTIC_OUT,

        /**
         * Quartic-In-Out Interpolation.
         */
        QUARTIC_IN_OUT,

        /**
         * Quartic-Out-In Interpolation.
         */
        QUARTIC_OUT_IN,

        /**
         * Quintic-In Interpolation.
         */
        QUINTIC_IN,

        /**
         * Quintic-Out Interpolation.
         */
        QUINTIC_OUT,

        /**
         * Quintic-In-Out Interpolation.
         */
        QUINTIC_IN_OUT,

        /**
         * Quintic-Out-In Interpolation.
         */
        QUINTIC_OUT_IN,

        /**
         * Sine-In Interpolation.
         */
        SINE_IN,

        /**
         * Sine-Out Interpolation.
         */
        SINE_OUT,

        /**
         * Sine-In-Out Interpolation.
         */
        SINE_IN_OUT,

        /**
         * Sine-Out-In Interpolation.
         */
        SINE_OUT_IN,

        /**
         * Exponential-In Interpolation.
         */
        EXPONENTIAL_IN,

        /**
         * Exponential-Out Interpolation.
         */
        EXPONENTIAL_OUT,

        /**
         * Exponential-In-Out Interpolation.
         */
        EXPONENTIAL_IN_OUT,

        /**
         * Exponential-Out-In Interpolation.
         */
        EXPONENTIAL_OUT_IN,

        /**
         * Circular-In Interpolation.
         */
        CIRCULAR_IN,

        /**
         * Circular-Out Interpolation.
         */
        CIRCULAR_OUT,

        /**
         * Circular-In-Out Interpolation.
         */
        CIRCULAR_IN_OUT,

        /**
         * Circular-Out-In Interpolation.
         */
        CIRCULAR_OUT_IN,

        /**
         * Elastic-In Interpolation.
         */
        ELASTIC_IN,

        /**
         * Elastic-Out Interpolation.
         */
        ELASTIC_OUT,

        /**
         * Elastic-In-Out Interpolation.
         */
        ELASTIC_IN_OUT,

        /**
         * Elastic-Out-In Interpolation.
         */
        ELASTIC_OUT_IN,

        /**
         * Overshoot-In Interpolation.
         */
        OVERSHOOT_IN,

        /**
         * Overshoot-Out Interpolation.
         */
        OVERSHOOT_OUT,

        /**
         * Overshoot-In-Out Interpolation.
         */
        OVERSHOOT_IN_OUT,

        /**
         * Overshoot-Out-In Interpolation.
         */
        OVERSHOOT_OUT_IN,

        /**
         * Bounce-In Interpolation.
         */
        BOUNCE_IN,

        /**
         * Bounce-Out Interpolation.
         */
        BOUNCE_OUT,

        /**
         * Bounce-In-Out Interpolation.
         */
        BOUNCE_IN_OUT,

        /**
         * Bounce-Out-In Interpolation.
         */
        BOUNCE_OUT_IN
    };

    /**
     * Creates a new curve.
     *
     * @param pointCount The number of points in the curve.
     * @param componentCount The number of Float component values per key value.
     * @script{create}
     */
    static UPtr<Curve> create(unsigned int pointCount, unsigned int componentCount);

    /**
     * Gets the number of points in the curve.
     *
     * @return The number of points in the curve.
     */
    unsigned int getPointCount() const;

    /**
     * Gets the number of Float component values per points.
     *
     * @return The number of Float component values per point.
     */
    unsigned int getComponentCount() const;

    /**
     * Returns the start time for the curve.
     *
     * @return The curve's start time.
     */
    Float getStartTime() const;

    /**
     * Returns the end time for the curve.
     *
     * @return The curve's end time.
     */
    Float getEndTime() const;

    /**
     * Sets the given point values on the curve.
     *
     * @param index The index of the point.
     * @param time The time for the key.
     * @param value The point to add.
     * @param type The curve interpolation type.
     */
    void setPoint(unsigned int index, Float time, float* value, InterpolationType type);

    /**
     * Sets the given point on the curve for the specified index and the specified parameters.
     *
     * @param index The index of the point.
     * @param time The time of the point within the curve.
     * @param value The value of the point to copy the data from.
     * @param type The curve interpolation type.
     * @param inValue The tangent approaching the point.
     * @param outValue The tangent leaving the point.
     */
    void setPoint(unsigned int index, Float time, float* value, InterpolationType type,
                  float* inValue, float* outValue);

    /**
     * Sets the tangents for a point on the curve specified by the index.
     *
     * @param index The index of the point.
     * @param type The interpolation type.
     * @param type The curve interpolation type.
     * @param inValue The tangent approaching the point.
     * @param outValue The tangent leaving the point.
     */
    void setTangent(unsigned int index, InterpolationType type, float* inValue, float* outValue);
    
    /**
     * Gets the time at a specified point.
     *
     * @param index The index of the point.
     *
     * @return The time for a key point.
     */
    Float getPointTime(unsigned int index) const;
    
    /**
     * Gets the interpolation type at the specified point
     *
     * @param index The index of the point.
     * 
     * @return The interpolation type at the specified index.
     */
    InterpolationType getPointInterpolation(unsigned int index) const;
    
    /**
     * Gets the values and in/out tangent value at a spedified point.
     *
     * @param index The index of the point.
     * @param value The value at the specified index. Ignored if NULL.
     * @param inValue The tangent inValue at the specified index. Ignored if NULL.
     * @param outValue The tangent outValue at the specified index. Ignored if NULL.
     */
    void getPointValues(unsigned int index, float* value, float* inValue, float* outValue) const;

    /**
     * Evaluates the curve at the given position value.
     *
     * Time should generally be specified as a value between 0.0 - 1.0, inclusive.
     * A value outside this range can also be specified to perform an interpolation
     * between the two end points of the curve. This can be useful for smoothly
     * interpolating a repeat of the curve.
     *
     * @param time The position to evaluate the curve at.
     * @param dst The evaluated value of the curve at the given time.
     */
    void evaluate(Float time, Float* dst) const;

    /**
     * Evaluates the curve at the given position value (between 0.0 and 1.0 inclusive)
     * within the specified subregion of the curve.
     *
     * This method is useful for evaluating sub sections of the curve. A common use for
     * this is when evaluating individual animation clips that are positioned within a
     * larger animation curve. This method also allows looping to occur between the
     * end points of curve sub regions, with optional blending/interpolation between
     * the end points (using the loopBlendTime parameter).
     *
     * Time should generally be specified as a value between 0.0 - 1.0, inclusive.
     * A value outside this range can also be specified to perform an interpolation
     * between the two end points of the curve. This can be useful for smoothly
     * interpolating a repeat of the curve.
     *
     * @param time The position within the subregion of the curve to evaluate the curve at.
     *      A time of zero represents the start of the subregion, with a time of one
     *      representing the end of the subregion.
     * @param startTime Start time for the subregion (between 0.0 - 1.0).
     * @param endTime End time for the subregion (between 0.0 - 1.0).
     * @param loopBlendTime Time (in milliseconds) to blend between the end points of the curve
     *      for looping purposes when time is outside the range 0-1. A value of zero here
     *      disables curve looping.
     * @param dst The evaluated value of the curve at the given time.
     */
    void evaluate(Float time, Float startTime, Float endTime, Float loopBlendTime, Float* dst) const;

    /**
     * Linear interpolation function.
     */
    static Float lerp(Float t, Float from, Float to);

private:

    /**
     * Defines a single point within a curve.
     */
    class Point
    {
    public:

        /** The time of the point within the curve. */
        float time;
        /** The value of the point. */
        float* value;
        /** The value of the tangent when approaching this point (from the previous point in the curve). */
        float* inValue;
        /** The value of the tangent when leaving this point (towards the next point in the curve). */
        float* outValue;
        /** The type of interpolation to use between this point and the next point. */
        InterpolationType type;

        /**
         * Constructor.
         */
        Point();

        /**
         * Destructor.
         */
        ~Point();

        /**
         * Hidden copy assignment operator.
         */
        Point& operator=(const Point&);
    };

    /**
     * Constructor.
     */
    Curve();

    /**
     * Constructs a new curve and the specified parameters.
     *
     * @param pointCount The number of points in the curve.
     * @param componentCount The number of Float component values per key value.
     */
    Curve(unsigned int pointCount, unsigned int componentCount);

    /**
     * Constructor.
     */
    Curve(const Curve& copy);

    /**
     * Destructor.
     */
    ~Curve();

    /**
     * Hidden copy assignment operator.
     */
    Curve& operator=(const Curve&);

    /**
     * Bezier interpolation function.
     */
    void interpolateBezier(Float s, Point* from, Point* to, Float* dst) const;

    /**
     * Bspline interpolation function.
     */
    void interpolateBSpline(Float s, Point* c0, Point* c1, Point* c2, Point* c3, Float* dst) const;

    /**
     * Hermite interpolation function.
     */
    void interpolateHermite(Float s, Point* from, Point* to, Float* dst) const;

    /**
     * Hermite interpolation function.
     */
    void interpolateHermiteFlat(Float s, Point* from, Point* to, Float* dst) const;

    /**
     * Hermite interpolation function.
     */
    void interpolateHermiteSmooth(Float s, unsigned int index, Point* from, Point* to, Float* dst) const;

    /**
     * Linear interpolation function.
     */
    void interpolateLinear(Float s, Point* from, Point* to, Float* dst) const;

    /**
     * Quaternion interpolation function.
     */
    void interpolateQuaternion(Float s, float* from, float* to, Float* dst) const;

    /**
     * Determines the current keyframe to interpolate from based on the specified time.
     */
    int determineIndex(Float time, unsigned int min, unsigned int max) const;

    /**
     * Sets the offset for the beginning of a Quaternion piece of data within the curve's value span at the specified
     * index. The next four components of data starting at the given index will be interpolated as a Quaternion.
     * This function will assert an error if the given index is greater than the component size subtracted by the four components required
     * to store a quaternion.
     *
     * @param index The index of the Quaternion rotation data.
     */
    void setQuaternionOffset(unsigned int index);

    /**
     * Gets the InterpolationType value for the given string ID
     *
     * @param interpolationId The string representation of the InterpolationType
     * @return the InterpolationType value; -1 if the string does not represent an InterpolationType.
     */
    static int getInterpolationType(const char* interpolationId);

    unsigned int _pointCount;           // Number of points on the curve.
    unsigned int _componentCount;       // Number of components on the curve.
    unsigned int _componentSize;        // The component size (in bytes).
    unsigned int* _quaternionOffset;    // Offset for the rotation component.
    Point* _points;                     // The points on the curve.
};


Vector3 catmullRomSpline(const Vector3& p0, const Vector3& p1,
    const Vector3& p2, const Vector3& p3,
    double t, double tau = 0.5f);

Vector3 bezierCurve(const Vector3& p0, const Vector3& p1,
    const Vector3& p2, const Vector3& p3,
    double t, double tau = 0.5f);

}
#endif
