#include "base/Base.h"
#include "AnimationValue.h"

namespace mgp
{

AnimationValue::AnimationValue(unsigned int componentCount)
  : _componentCount(componentCount), _componentSize(componentCount * sizeof(Float))
{
    GP_ASSERT(_componentCount > 0);
    _value = new Float[_componentCount];
}

AnimationValue::AnimationValue(const AnimationValue& copy)
{
    _value = new Float[copy._componentCount];
    _componentSize = copy._componentSize;
    _componentCount = copy._componentCount;
    memcpy(_value, copy._value, _componentSize);
}

AnimationValue::~AnimationValue()
{
    SAFE_DELETE_ARRAY(_value);
}

AnimationValue& AnimationValue::operator=(const AnimationValue& v)
{
    if (this != &v)
    {
        if (_value == NULL || _componentSize != v._componentSize || _componentCount != v._componentCount)
        {
            _componentSize = v._componentSize;
            _componentCount = v._componentCount;
            SAFE_DELETE_ARRAY(_value);
            _value = new Float[v._componentCount];
        }
        memcpy(_value, v._value, _componentSize);
    }
    return *this;
}

Float AnimationValue::getFloat(unsigned int index) const
{
    GP_ASSERT(index < _componentCount);
    GP_ASSERT(_value);

    return _value[index];
}

void AnimationValue::setFloat(unsigned int index, Float value)
{
    GP_ASSERT(index < _componentCount);
    GP_ASSERT(_value);

    _value[index] = value;
}

void AnimationValue::getFloats(unsigned int index, Float* values, unsigned int count) const
{
    GP_ASSERT(_value && values && index < _componentCount && (index + count) <= _componentCount);

    memcpy(values, &_value[index], count * sizeof(Float));
}

void AnimationValue::setFloats(unsigned int index, Float* values, unsigned int count)
{
    GP_ASSERT(_value && values && index < _componentCount && (index + count) <= _componentCount);

    memcpy(&_value[index], values, count * sizeof(Float));
}

}
