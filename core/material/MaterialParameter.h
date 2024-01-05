#ifndef MATERIALPARAMETER_H_
#define MATERIALPARAMETER_H_

#include "animation/AnimationTarget.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "math/Matrix.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include "base/Serializable.h"

namespace mgp
{
    class Node;

/**
 * Defines a material parameter.
 *
 * This class represents a parameter that can be set for a material.
 * The methods in this class provide a mechanism to set parameters
 * of all supported types. Some types support setting by value,
 * while others only support setting by reference/pointer.
 *
 * Setting a parameter by reference/pointer provides the ability to
 * pass an array of values as well as a convenient way to support
 * auto-binding of values to a material parameter. For example, by
 * setting the parameter value to a pointer to a Matrix, any changes
 * to the Matrix will automatically be reflected in the technique the
 * next time the parameter is applied to the render state.
 *
 * Note that for parameter values to arrays or pointers, the 
 * MaterialParameter will keep a long-lived reference to the passed
 * in array/pointer. Therefore, you must ensure that the pointers
 * you pass in are valid for the lifetime of the MaterialParameter
 * object.
 */
class MaterialParameter : public AnimationTarget, public Refable, public Serializable
{
    friend class Material;

public:

    /**
     * Animates the uniform.
     */
    static const int ANIMATE_UNIFORM = 1;

    /**
     * Returns the name of this material parameter.
     */
    const char* getName() const;

    /**
     * Returns the texture sampler or NULL if this MaterialParameter is not a sampler type.
     * 
     * @param index Index of the sampler (if the parameter is a sampler array),
     *      or zero if it is a single sampler value.
     *
     * @return The texture sampler or NULL if this MaterialParameter is not a sampler type.
     */
    Texture* getSampler(unsigned int index = 0) const;

    /**
     * Sets the value of this parameter to a float value.
     */
    void setValue(float value);
    void setValue(double value);

    /**
     * Sets the value of this parameter to an integer value.
     */
    void setValue(int value);

    /**
     * Stores a pointer/array of float values in this parameter.
     */
    void setValue(const float* values, unsigned int count = 1);

    /**
     * Stores a pointer/array of integer values in this parameter.
     */
    void setValue(const int* values, unsigned int count = 1);

    /**
     * Stores a copy of the specified Vector2 value in this parameter.
     */
    void setValue(const Vector2& value);

    /**
     * Stores a pointer/array of Vector2 values in this parameter.
     */
    void setValue(const Vector2* values, unsigned int count = 1);

    /**
     * Stores a copy of the specified Vector3 value in this parameter.
     */
    void setValue(const Vector3& value);

    /**
     * Stores a pointer/array of Vector3 values in this parameter.
     */
    void setValue(const Vector3* values, unsigned int count = 1);

    /**
     * Stores a copy of the specified Vector4 value in this parameter.
     */
    void setValue(const Vector4& value);

    /**
     * Stores a pointer/array of Vector4 values in this parameter.
     */
    void setValue(const Vector4* values, unsigned int count = 1);

    /**
     * Stores a copy of the specified Matrix value in this parameter.
     */
    void setValue(const Matrix& value);

    /**
     * Stores a pointer/array of Matrix values in this parameter.
     */
    void setValue(const Matrix* values, unsigned int count = 1);

    /**
     * Sets the value of this parameter to the specified texture sampler.
     */
    void setValue(const Texture* sampler);

    /**
     * Sets the value of this parameter to the specified texture sampler array.
     *
     * @script{ignore}
     */
    void setValue(const Texture** samplers, unsigned int count);

    /**
     * Loads a texture sampler from the specified path and sets it as the value of this parameter.
     *
     * @param texturePath The path to the texture to set.
     * @param generateMipmaps True to generate a full mipmap chain for the texture, false otherwise.
     *
     * @return The texture sampler that was set for this material parameter.
     */
    Texture* setValue(const char* texturePath, bool generateMipmaps);

    /**
     * Stores a float value in this parameter.
     *
     * @param value The value to set.
     */
    void setFloat(float value);

    /**
     * Stores an array of float values in this parameter.
     *
     * @param values The array of values.
     * @param count The number of values in the array.
     * @param copy True to make a copy of the array in the material parameter, or false
     *      to point to the passed in array/pointer (which must be valid for the lifetime
     *      of the MaterialParameter).
     */
    void setFloatArray(const float* values, unsigned int count, bool copy = false);

    /**
     * Stores an integer value in this parameter.
     *
     * @param value The value to set.
     */
    void setInt(int value);

    /**
     * Stores an array of integer values in this parameter.
     */
    void setIntArray(const int* values, unsigned int count, bool copy = false);

    /**
     * Stores a Vector2 value in this parameter.
     *
     * @param value The value to set.
     */
    void setVector2(const Vector2& value);

    /**
     * Stores an array of Vector2 values in this parameter.
     *
     * @param values The array of values.
     * @param count The number of values in the array.
     * @param copy True to make a copy of the array in the material parameter, or false
     *      to point to the passed in array/pointer (which must be valid for the lifetime
     *      of the MaterialParameter).
     */
    void setVector2Array(const Vector2* values, unsigned int count, bool copy = false);

    /**
     * Stores a Vector3 value in this parameter.
     *
     * @param value The value to set.
     */
    void setVector3(const Vector3& value);

    /**
     * Stores an array of Vector3 values in this parameter.
     */
    void setVector3Array(const Vector3* values, unsigned int count, bool copy = false);

    /**
     * Stores a Vector4 value in this parameter.
     *
     * @param value The value to set.
     */
    void setVector4(const Vector4& value);

    /**
     * Stores an array of Vector4 values in this parameter.
     *
     * @param values The array of values.
     * @param count The number of values in the array.
     * @param copy True to make a copy of the array in the material parameter, or false
     *      to point to the passed in array/pointer (which must be valid for the lifetime
     *      of the MaterialParameter).
     */
    void setVector4Array(const Vector4* values, unsigned int count, bool copy = false);

    /**
     * Stores a Matrix value in this parameter.
     *
     * @param value The value to set.
     */
    void setMatrix(const Matrix& value);

    /**
     * Stores an array of Matrix values in this parameter.
     *
     * @param values The array of values.
     * @param count The number of values in the array.
     * @param copy True to make a copy of the array in the material parameter, or false
     *      to point to the passed in array/pointer (which must be valid for the lifetime
     *      of the MaterialParameter).
     */
    void setMatrixArray(const Matrix* values, unsigned int count, bool copy = false);

    /**
     * Loads a texture sampler from the specified path and sets it as the value of this parameter.
     *
     * @param texturePath The path to the texture to set.
     * @param generateMipmaps True to generate a full mipmap chain for the texture, false otherwise.
     *
     * @return The texture sampler that was set for this material parameter.
     */
    Texture* setSampler(const char* texturePath, bool generateMipmaps);

    /**
     * Stores a Sampler value in this parameter.
     *
     * @param value The value to set.
     */
    void setSampler(const Texture* value);

    /**
     * Stores an array of Sampler values in this parameter.
     *
     * @param values The array of values.
     * @param count The number of values in the array.
     * @param copy True to make a copy of the array in the material parameter, or false
     *      to point to the passed in array/pointer (which must be valid for the lifetime
     *      of the MaterialParameter).
     * @script{ignore}
     */
    void setSamplerArray(const Texture** values, unsigned int count, bool copy = false);

    /**
     * Binds the return value of a class method to this material parameter.
     *
     * This method enables binding of arbitrary class methods to a material
     * parameter. This is useful when you want to set a material parameter
     * to a variable that is frequently changing (such as a world matrix).
     *
     * By binding a method pointer, the method will be called automatically
     * to retrieve the updated parameter value each time the material is bound
     * for rendering.
     *
     * @param classInstance The instance of the class containing the member method to bind.
     * @param valueMethod A pointer to the class method to bind (in the format '&class::method').
     */
    template <class ClassType, class ParameterType>
    void bindValue(ClassType* classInstance, ParameterType (ClassType::*valueMethod)() const);

    /**
     * Binds the return value of a class method to this material parameter.
     *
     * This overloads the setBinding method to provide support for array parameters.
     * The valueMethod parameter should return an array (pointer) of a supported
     * material parameter type, such as Matrix* for an array of matrices. The
     * countMethod should point to a method that returns the number of entries in
     * the value returned from valueMethod.
     *
     * @param classInstance The instance of the class containing the member method to bind.
     * @param valueMethod A pointer to the class method to bind (in the format '&class::method').
     * @param countMethod A pointer to a method that returns the number of entries in the array returned by valueMethod.
     */
    template <class ClassType, class ParameterType>
    void bindValue(ClassType* classInstance, ParameterType (ClassType::*valueMethod)() const, unsigned int (ClassType::*countMethod)() const);

    /**
     * Binds the return value of the supported class method for the given node to this material parameter.
     * 
     * Note: intended for use from Lua scripts.
     * 
     * @param node The node containing the the member method to bind.
     * @param binding The name of the class method to bind (in the format '&class::method').
     *      Note: this name must be one of the following supported methods:
     *      - "&Node::getBackVector"
     *      - "&Node::getDownVector"
     *      - "&Node::getTranslationWorld"
     *      - "&Node::getTranslationView"
     *      - "&Node::getForwardVector"
     *      - "&Node::getForwardVectorWorld"
     *      - "&Node::getForwardVectorView"
     *      - "&Node::getLeftVector"
     *      - "&Node::getRightVector"
     *      - "&Node::getRightVectorWorld"
     *      - "&Node::getUpVector"
     *      - "&Node::getUpVectorWorld"
     *      - "&Node::getActiveCameraTranslationWorld"
     *      - "&Node::getActiveCameraTranslationView"
     *      - "&Node::getScaleX"
     *      - "&Node::getScaleY"
     *      - "&Node::getScaleZ"
     *      - "&Node::getTranslationX"
     *      - "&Node::getTranslationY"
     *      - "&Node::getTranslationZ"
     */
    void bindValue(Node* node, const char* binding);

    /**
     * @see AnimationTarget::getAnimationPropertyComponentCount
     */
    unsigned int getAnimationPropertyComponentCount(int propertyId) const;

    /**
     * @see AnimationTarget::getAnimationProperty
     */
    void getAnimationPropertyValue(int propertyId, AnimationValue* value);

    /**
     * @see AnimationTarget::setAnimationProperty
     */
    void setAnimationPropertyValue(int propertyId, AnimationValue* value, float blendWeight = 1.0f);


    /**
     * @see Activator::createObject
     */
    static Serializable* createObject();

    /**
     * @see Activator::enumToString
     */
    static std::string enumToString(const std::string& enumName, int value);

    /**
     * @see Activator::enumParse
     */
    static int enumParse(const std::string& enumName, const std::string& str);

    /**
     * @see Serializable::getClassName
     */
    std::string getClassName();

    /**
     * @see Serializable::onSerialize
     */
    void onSerialize(Serializer* serializer);

    /**
     * @see Serializable::onDeserialize
     */
    void onDeserialize(Serializer* serializer);

private:
   
    /**
     * Constructor.
     */
    MaterialParameter(const char* name);
    
    /**
     * Destructor.
     */
    ~MaterialParameter();

    /**
     * Hidden copy assignment operator.
     */
    MaterialParameter& operator=(const MaterialParameter&);
    
    /**
     * Interface implemented by templated method bindings for simple storage and iteration.
     */
    class MethodBinding : public Refable
    {
        friend class Material;
        friend class MaterialParamBinding;

    public:

        virtual void setValue(ShaderProgram* effect) = 0;

    protected:

        /**
         * Constructor.
         */
        MethodBinding(MaterialParameter* param);

        /**
         * Destructor.
         */
        virtual ~MethodBinding() { }

        /**
         * Hidden copy assignment operator.
         */
        MethodBinding& operator=(const MethodBinding&);

        MaterialParameter* _parameter;
        bool _autoBinding;
    };

    /**
     * Defines a method parameter binding for a single value.
     */
    template <class ClassType, class ParameterType>
    class MethodValueBinding : public MethodBinding
    {
        typedef ParameterType (ClassType::*ValueMethod)() const;
    public:
        MethodValueBinding(MaterialParameter* param, ClassType* instance, ValueMethod valueMethod);
        void setValue(ShaderProgram* effect);
    private:
        ClassType* _instance;
        ValueMethod _valueMethod;

    };

    /**
     * Defines a method parameter binding for an array of values.
     */
    template <class ClassType, class ParameterType>
    class MethodArrayBinding : public MethodBinding
    {
        typedef ParameterType (ClassType::*ValueMethod)() const;
        typedef unsigned int (ClassType::*CountMethod)() const;
    public:
        MethodArrayBinding(MaterialParameter* param, ClassType* instance, ValueMethod valueMethod, CountMethod countMethod);
        void setValue(ShaderProgram* effect);
    private:
        ClassType* _instance;
        ValueMethod _valueMethod;
        CountMethod _countMethod;
    };

    void clearValue();

    void bind(ShaderProgram* effect);

    void applyAnimationValue(AnimationValue* value, float blendWeight, int components);

    void cloneInto(MaterialParameter* materialParameter) const;

public:
    enum LOGGER_DIRTYBITS
    {
        UNIFORM_NOT_FOUND = 0x01,
        PARAMETER_VALUE_NOT_SET = 0x02
    };
    
    union
    {
        /** @script{ignore} */
        float floatValue;
        /** @script{ignore} */
        int intValue;
        /** @script{ignore} */
        float* floatPtrValue;
        /** @script{ignore} */
        int32_t* intPtrValue;
        /** @script{ignore} */
        const Texture* samplerValue;
        /** @script{ignore} */
        const Texture** samplerArrayValue;
        /** @script{ignore} */
        //MethodBinding* method;
        float floats[16];
    } _value;
    
    enum Type
    {
        NONE,
        FLOAT,
        INT,
        VECTOR2,
        VECTOR3,
        VECTOR4,
        MATRIX,
        SAMPLER,
        //METHOD
    } _type;
    
    unsigned int _count;
    bool _dynamicAlloc;
    bool _isArray;
    std::string _name;
    Uniform* _uniform;
    char _loggerDirtyBits;
    MethodBinding* _methodBinding;
    bool _temporary;
    //index in array of uniform
    int arrrayOffset;
};

template <class ClassType, class ParameterType>
void MaterialParameter::bindValue(ClassType* classInstance, ParameterType (ClassType::*valueMethod)() const)
{
    clearValue();

    if (_methodBinding) {
        SAFE_RELEASE(_methodBinding);
    }
    _methodBinding = new MethodValueBinding<ClassType, ParameterType>(this, classInstance, valueMethod);
    _dynamicAlloc = true;
    _type = MaterialParameter::NONE;
    _value.intValue = 0;
}

template <class ClassType, class ParameterType>
void MaterialParameter::bindValue(ClassType* classInstance, ParameterType (ClassType::*valueMethod)() const, unsigned int (ClassType::*countMethod)() const)
{
    clearValue();

    if (_methodBinding) {
        SAFE_RELEASE(_methodBinding);
    }
    _methodBinding = new MethodArrayBinding<ClassType, ParameterType>(this, classInstance, valueMethod, countMethod);
    _dynamicAlloc = true;
    _type = MaterialParameter::NONE;
    _value.intValue = 0;
}

template <class ClassType, class ParameterType>
MaterialParameter::MethodValueBinding<ClassType, ParameterType>::MethodValueBinding(MaterialParameter* param, ClassType* instance, ValueMethod valueMethod) :
    MethodBinding(param), _instance(instance), _valueMethod(valueMethod)
{
}

template <class ClassType, class ParameterType>
void MaterialParameter::MethodValueBinding<ClassType, ParameterType>::setValue(ShaderProgram* effect)
{
    //effect->setValue(_parameter->_uniform, (_instance->*_valueMethod)());
    _parameter->setValue((_instance->*_valueMethod)());
}

template <class ClassType, class ParameterType>
MaterialParameter::MethodArrayBinding<ClassType, ParameterType>::MethodArrayBinding(MaterialParameter* param, ClassType* instance, ValueMethod valueMethod, CountMethod countMethod) :
    MethodBinding(param), _instance(instance), _valueMethod(valueMethod), _countMethod(countMethod)
{
}

template <class ClassType, class ParameterType>
void MaterialParameter::MethodArrayBinding<ClassType, ParameterType>::setValue(ShaderProgram* effect)
{
    //effect->setValue(_parameter->_uniform, (_instance->*_valueMethod)(), (_instance->*_countMethod)());
    _parameter->setValue((_instance->*_valueMethod)(), (_instance->*_countMethod)());
}

}

#endif
