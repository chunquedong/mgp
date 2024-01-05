#ifndef ANIMATIONVALUE_H_
#define ANIMATIONVALUE_H_

#include "Animation.h"

namespace mgp
{

/**
 * Defines a running animation value which can have one or more floats.
 */
class AnimationValue
{
    friend class AnimationClip;
    friend class KeyframeChannel;

public:

    /**
     * Gets the value at the specified index.
     *
     * @param index The index of the component to get the value for.
     *
     * @return The Float value at the specified index.
     */
    Float getFloat(unsigned int index) const;

    /**
     * Sets the value at the specified index.
     *
     * @param index The index of the component to set the value for.
     * @param value The value to set the component to.
     */
    void setFloat(unsigned int index, Float value);

    /**
     * Copies one or more Float values from this AnimationValue into the specified array.
     *
     * @param index The index to start copying from.
     * @param values Pointer to Float array to copy values into.
     * @param count Number of values to copy.
     */
    void getFloats(unsigned int index, Float* values, unsigned int count) const;

    /**
     * Copies one or more Float values into the AnimationValue.
     *
     * @param index The index of the first component to set the value for.
     * @param values Array of values to copy into the AnimationValue.
     * @param count Number of values to in the array to copy in.
     */
    void setFloats(unsigned int index, Float* values, unsigned int count);


    Float* getData() { return _value; }

private:

    /**
     * Constructor.
     */
    AnimationValue();

    /**
     * Constructor.
     */
    AnimationValue(unsigned int componentCount);

    /**
     * Constructor.
     */
    AnimationValue(const AnimationValue& copy);

    /**
     * Destructor.
     */
    ~AnimationValue();

    /**
     * Hidden copy assignment operator.
     */
    AnimationValue& operator=(const AnimationValue& v);

    unsigned int _componentCount;   // The number of Float values for the property.
    unsigned int _componentSize;    // The number of bytes of memory the property is.
    Float* _value;                  // The current value of the property.

};

}

#endif
