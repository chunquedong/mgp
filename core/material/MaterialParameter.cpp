#include "base/Base.h"
#include "MaterialParameter.h"
#include "scene/Node.h"
#include "scene/Renderer.h"
#include "Texture.h"

namespace mgp
{

MaterialParameter::MaterialParameter(const char* name) :
    _type(MaterialParameter::NONE), _count(1), _dynamicAlloc(false), _name(name ? name : ""), _uniform(NULL), _loggerDirtyBits(0),
    _methodBinding(NULL), _temporary(false), arrrayOffset(0), _isArray(false)
{
    clearValue();
}

MaterialParameter::~MaterialParameter()
{
    clearValue();
    if (_methodBinding) {
        SAFE_RELEASE(_methodBinding);
    }
}

void MaterialParameter::clearValue()
{
    // Release parameters
    if (_type == MaterialParameter::SAMPLER)
    {
        if (!_isArray) {
            if (_value.samplerValue)
                const_cast<Texture*>(_value.samplerValue)->release();
        }
        else {
            if (_value.samplerArrayValue)
            {
                for (unsigned int i = 0; i < _count; ++i)
                {
                    const_cast<Texture*>(_value.samplerArrayValue[i])->release();
                }
            }
        }
    }

    // Free dynamic data
    if (_dynamicAlloc && _isArray)
    {
        switch (_type)
        {
        case MaterialParameter::FLOAT:
        case MaterialParameter::VECTOR2:
        case MaterialParameter::VECTOR3:
        case MaterialParameter::VECTOR4:
        case MaterialParameter::MATRIX:
            SAFE_DELETE_ARRAY(_value.floatPtrValue);
            break;
        case MaterialParameter::INT:
            SAFE_DELETE_ARRAY(_value.intPtrValue);
            break;
        //case MaterialParameter::METHOD:
        //    SAFE_RELEASE(_value.method);
            break;
        case MaterialParameter::SAMPLER:
            SAFE_DELETE_ARRAY(_value.samplerArrayValue);
            break;
        default:
            // Ignore all other cases.
            break;
        }
    }
    _dynamicAlloc = false;
    _count = 1;
    _isArray = false;
    _type = MaterialParameter::NONE;
}

const char* MaterialParameter::getName() const
{
    return _name.c_str();
}

Texture* MaterialParameter::getSampler(unsigned int index) const
{
    if (_type == MaterialParameter::SAMPLER) {
        if (!_isArray) {
            return const_cast<Texture*>(_value.samplerValue);
        }
        else {
            return const_cast<Texture*>(_value.samplerArrayValue[index]);
        }
    }
    return NULL;
}

void MaterialParameter::setValue(float value)
{
    setFloat(value);
}

void MaterialParameter::setValue(double value)
{
    setFloat(value);
}

void MaterialParameter::setValue(int value)
{
    setInt(value);
}

void MaterialParameter::setValue(const float* values, unsigned int count)
{
    setFloatArray(values, count);
}

void MaterialParameter::setValue(const int* values, unsigned int count)
{
    setIntArray(values, count);
}

void MaterialParameter::setValue(const Vector2& value)
{
    setVector2(value);
}

void MaterialParameter::setValue(const Vector2* values, unsigned int count)
{
    setVector2Array(values, count);
}

void MaterialParameter::setValue(const Vector3& value)
{
    setVector3(value);
}

void MaterialParameter::setValue(const Vector3* values, unsigned int count)
{
    setVector3Array(values, count);
}

void MaterialParameter::setValue(const Vector4& value)
{
    setVector4(value);
}

void MaterialParameter::setValue(const Vector4* values, unsigned int count)
{
    setVector4Array(values, count);
}

void MaterialParameter::setValue(const Matrix& value)
{
    setMatrix(value);
}

void MaterialParameter::setValue(const Matrix* values, unsigned int count)
{
    setMatrixArray(values, count);
}

void MaterialParameter::setValue(const Texture* sampler)
{
    setSampler(sampler);
}

void MaterialParameter::setValue(const Texture** samplers, unsigned int count)
{
    setSamplerArray(samplers, count);
}

Texture* MaterialParameter::setValue(const char* texturePath, bool generateMipmaps)
{
    return setSampler(texturePath, generateMipmaps);
}

void MaterialParameter::setFloat(float value)
{
    clearValue();

    _value.floatValue = value;
    _type = MaterialParameter::FLOAT;
}

void MaterialParameter::setFloatArray(const float* values, unsigned int count, bool copy)
{
    GP_ASSERT(values);
    if (copy)
    {
        if (_type == MaterialParameter::FLOAT && _isArray && _count == count) {
            //pass
        }
        else {
            clearValue();
            _value.floatPtrValue = new float[count];
            _dynamicAlloc = true;
        }
        memcpy(_value.floatPtrValue, values, sizeof(float) * count);
    }
    else
    {
        clearValue();
        _value.floatPtrValue = const_cast<float*> (values);
    }

    _count = count;
    _type = MaterialParameter::FLOAT;
    _isArray = true;
}

void MaterialParameter::setInt(int value)
{
    clearValue();

    _value.intValue = value;
    _type = MaterialParameter::INT;
}

void MaterialParameter::setIntArray(const int* values, unsigned int count, bool copy)
{
    GP_ASSERT(values);

    if (copy)
    {
        if (_type == MaterialParameter::INT && _isArray && _count == count) {
            //pass
        }
        else {
            clearValue();
            _value.intPtrValue = new int32_t[count];
            _dynamicAlloc = true;
        }
        memcpy(_value.intPtrValue, values, sizeof(int) * count);
    }
    else
    {
        clearValue();
        _value.intPtrValue = const_cast<int32_t*> (values);
    }

    _count = count;
    _type = MaterialParameter::INT;
    _isArray = true;
}

void MaterialParameter::setVector2(const Vector2& value)
{
    clearValue();

    _value.floats[0] = value.x;
    _value.floats[1] = value.y;
    _dynamicAlloc = false;
    _count = 1;
    _type = MaterialParameter::VECTOR2;
}

void MaterialParameter::setVector2Array(const Vector2* values, unsigned int count, bool copy)
{
    GP_ASSERT(values);
    if (_type == MaterialParameter::VECTOR2 && _isArray && _count == count) {
        //pass
    }
    else {
        clearValue();
        _value.floatPtrValue = new float[2 * count];
        _dynamicAlloc = true;

        _count = count;
        _type = MaterialParameter::VECTOR2;
        _isArray = true;
    }

    for (int i = 0; i < count; ++i) {
        int p = i * 2;
        _value.floatPtrValue[p] = values[i].x;
        _value.floatPtrValue[p + 1] = values[i].y;
    }

}

void MaterialParameter::setVector3(const Vector3& value)
{
    clearValue();

    _value.floats[0] = value.x;
    _value.floats[1] = value.y;
    _value.floats[2] = value.z;
    _dynamicAlloc = false;
    _count = 1;
    _type = MaterialParameter::VECTOR3;
}

void MaterialParameter::setVector3Array(const Vector3* values, unsigned int count, bool copy)
{
    GP_ASSERT(values);
    if (_type == MaterialParameter::VECTOR3 && _isArray && _count == count) {
        //pass
    }
    else {
        clearValue();
        _value.floatPtrValue = new float[3 * count];
        _dynamicAlloc = true;

        _count = count;
        _type = MaterialParameter::VECTOR3;
        _isArray = true;
    }

    for (int i = 0; i < count; ++i) {
        int p = i * 3;
        _value.floatPtrValue[p] = values[i].x;
        _value.floatPtrValue[p + 1] = values[i].y;
        _value.floatPtrValue[p + 2] = values[i].z;
    }

}

void MaterialParameter::setVector4(const Vector4& value)
{
    clearValue();

    _value.floats[0] = value.x;
    _value.floats[1] = value.y;
    _value.floats[2] = value.z;
    _value.floats[3] = value.w;
    _dynamicAlloc = false;
    _count = 1;
    _type = MaterialParameter::VECTOR4;
}

void MaterialParameter::setVector4Array(const Vector4* values, unsigned int count, bool copy)
{
    GP_ASSERT(values);
    if (_type == MaterialParameter::VECTOR4 && _isArray && _count == count) {
        //pass
    }
    else {
        clearValue();
        _value.floatPtrValue = new float[4 * count];
        _dynamicAlloc = true;
        _count = count;
        _type = MaterialParameter::VECTOR4;
        _isArray = true;
    }

    for (int i = 0; i < count; ++i) {
        int p = i * 4;
        _value.floatPtrValue[p] = values[i].x;
        _value.floatPtrValue[p + 1] = values[i].y;
        _value.floatPtrValue[p + 2] = values[i].z;
        _value.floatPtrValue[p + 3] = values[i].w;
    }

}

void MaterialParameter::setMatrix(const Matrix& value)
{
    clearValue();

    value.toArray(_value.floats);

    _dynamicAlloc = false;
    _count = 1;
    _type = MaterialParameter::MATRIX;
}

void MaterialParameter::setMatrixArray(const Matrix* values, unsigned int count, bool copy)
{
    GP_ASSERT(values);

    if (_type == MaterialParameter::MATRIX && _isArray && _count == count) {
        //pass
    }
    else {
        clearValue();
        _value.floatPtrValue = new float[16 * count];
        _dynamicAlloc = true;

        _count = count;
        _type = MaterialParameter::MATRIX;
        _isArray = true;
    }

    for (int i = 0; i < count; ++i) {
        int p = i * 16;
        values[i].toArray(_value.floatPtrValue+p);
    }

}

Texture* MaterialParameter::setSampler(const char* texturePath, bool generateMipmaps)
{
    GP_ASSERT(texturePath);
    clearValue();

    Texture* sampler = Texture::create(texturePath, generateMipmaps).take();
    if (sampler)
    {
        _value.samplerValue = sampler;
        _type = MaterialParameter::SAMPLER;
    }
    return sampler;
}

void MaterialParameter::setSampler(const Texture* sampler)
{
    GP_ASSERT(sampler);
    clearValue();

    const_cast<Texture*>(sampler)->addRef();
    _value.samplerValue = sampler;
    _type = MaterialParameter::SAMPLER;
}

void MaterialParameter::setSamplerArray(const Texture** values, unsigned int count, bool copy)
{
    GP_ASSERT(values);
    if (copy)
    {
        if (_type == MaterialParameter::SAMPLER && _isArray && _count == count) {
            //pass
        }
        else {
            clearValue();
            _value.samplerArrayValue = new const Texture * [count];
            _dynamicAlloc = true;
        }
        memcpy(_value.samplerArrayValue, values, sizeof(Texture*) * count);
    }
    else
    {
        clearValue();
        _value.samplerArrayValue = values;
    }

    for (unsigned int i = 0; i < count; ++i)
    {
        const_cast<Texture*>(_value.samplerArrayValue[i])->addRef();
    }

    _count = count;
    _type = MaterialParameter::SAMPLER;
    _isArray = true;
}

void MaterialParameter::bind(ShaderProgram* effect)
{
    GP_ASSERT(effect);

    // If we had a Uniform cached that is not from the passed in effect,
    // we need to update our uniform to point to the new effect's uniform.
    if (!_uniform || _uniform->getEffect() != effect)
    {
        _uniform = effect->getUniform(_name.c_str());

        if (!_uniform)
        {
            if ((_loggerDirtyBits & UNIFORM_NOT_FOUND) == 0 && _name != "u_viewport")
            {
                // This parameter was not found in the specified effect, so do nothing.
                GP_WARN("Material parameter for uniform '%s' not found in effect: '%s'.", _name.c_str(), effect->getId());
                _loggerDirtyBits |= UNIFORM_NOT_FOUND;
            }
            return;
        }

        //auto set arrayOffset
        if (_uniform->_size > 1 && arrrayOffset == 0 && _name.back() == ']') {
            arrrayOffset = _name[_name.size() - 2]-'0';
        }
    }

    Renderer::cur()->bindUniform(this, _uniform, effect);
}

void MaterialParameter::bindValue(Node* node, const char* binding)
{
    GP_ASSERT(binding);

    if (strcmp(binding, "&Node::getBackVector") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getBackVector);
    }
    else if (strcmp(binding, "&Node::getDownVector") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getDownVector);
    }
    else if (strcmp(binding, "&Node::getTranslationWorld") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getTranslationWorld);
    }
    /*else if (strcmp(binding, "&Node::getTranslationView") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getTranslationView);
    }*/
    else if (strcmp(binding, "&Node::getForwardVector") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getForwardVector);
    }
    else if (strcmp(binding, "&Node::getForwardVectorWorld") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getForwardVectorWorld);
    }
    /*else if (strcmp(binding, "&Node::getForwardVectorView") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getForwardVectorView);
    }*/
    else if (strcmp(binding, "&Node::getLeftVector") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getLeftVector);
    }
    else if (strcmp(binding, "&Node::getRightVector") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getRightVector);
    }
    else if (strcmp(binding, "&Node::getRightVectorWorld") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getRightVectorWorld);
    }
    else if (strcmp(binding, "&Node::getUpVector") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getUpVector);
    }
    else if (strcmp(binding, "&Node::getUpVectorWorld") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getUpVectorWorld);
    }
    /*else if (strcmp(binding, "&Node::getActiveCameraTranslationWorld") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getActiveCameraTranslationWorld);
    }
    else if (strcmp(binding, "&Node::getActiveCameraTranslationView") == 0)
    {
        bindValue<Node, Vector3>(node, &Node::getActiveCameraTranslationView);
    }*/
    else if (strcmp(binding, "&Node::getScaleX") == 0)
    {
        bindValue<Node, Float>(node, &Node::getScaleX);
    }
    else if (strcmp(binding, "&Node::getScaleY") == 0)
    {
        bindValue<Node, Float>(node, &Node::getScaleY);
    }
    else if (strcmp(binding, "&Node::getScaleZ") == 0)
    {
        bindValue<Node, Float>(node, &Node::getScaleZ);
    }
    else if (strcmp(binding, "&Node::getTranslationX") == 0)
    {
        bindValue<Node, Float>(node, &Node::getTranslationX);
    }
    else if (strcmp(binding, "&Node::getTranslationY") == 0)
    {
        bindValue<Node, Float>(node, &Node::getTranslationY);
    }
    else if (strcmp(binding, "&Node::getTranslationZ") == 0)
    {
        bindValue<Node, Float>(node, &Node::getTranslationZ);
    }
    else
    {
        GP_WARN("Unsupported material parameter binding '%s'.", binding);
    }
}

unsigned int MaterialParameter::getAnimationPropertyComponentCount(int propertyId) const
{
    switch (propertyId)
    {
        case ANIMATE_UNIFORM:
        {
            switch (_type)
            {
                // These types don't support animation.
                case NONE:
                case MATRIX:
                case SAMPLER:
                //case METHOD:
                    return 0;
                case FLOAT:
                case INT:
                    return _count;
                case VECTOR2:
                    return 2 * _count;
                case VECTOR3:
                    return 3 * _count;
                case VECTOR4:
                    return 4 * _count;
                default:
                    return 0;
            }
        }
        break;
    }

    return 0;
}

void MaterialParameter::getAnimationPropertyValue(int propertyId, AnimationValue* value)
{
    GP_ASSERT(value);
    switch (propertyId)
    {
        case ANIMATE_UNIFORM:
        {
            switch (_type)
            {
                case FLOAT:
                    if (!_isArray) {
                        value->setFloat(0, _value.floatValue);
                    }
                    else {
                        GP_ASSERT(_value.floatPtrValue);
                        for (unsigned int i = 0; i < _count; i++)
                        {
                            value->setFloat(i, _value.floatPtrValue[i]);
                        }
                    }
                    break;
                case INT:
                    if (!_isArray) {
                        value->setFloat(0, _value.intValue);
                    }
                    else {
                        GP_ASSERT(_value.intPtrValue);
                        for (unsigned int i = 0; i < _count; i++)
                        {
                            value->setFloat(i, _value.intPtrValue[i]);
                        }
                    }
                    break;
                case VECTOR2: {
                    if (!_isArray) {
                        Float* data = value->getData();
                        for (int i = 0; i < _count; ++i) {
                            int p = i * 2;
                            data[p] = _value.floats[p];
                            data[p + 1] = _value.floats[p + 1];
                        }
                        value->setFloats(0, data, _count * 2);
                    }
                    else {
                        Float* data = value->getData();
                        for (int i = 0; i < _count; ++i) {
                            int p = i * 2;
                            data[p] = _value.floatPtrValue[p];
                            data[p + 1] = _value.floatPtrValue[p + 1];
                        }
                        value->setFloats(0, data, _count * 2);
                    }
                }
                    break;
                case VECTOR3: {
                    if (!_isArray) {
                        Float* data = value->getData();
                        for (int i = 0; i < _count; ++i) {
                            int p = i * 3;
                            data[p] = _value.floats[p];
                            data[p + 1] = _value.floats[p + 1];
                            data[p + 2] = _value.floats[p + 2];
                        }
                        value->setFloats(0, data, _count * 3);
                    }
                    else {
                        Float* data = value->getData();
                        for (int i = 0; i < _count; ++i) {
                            int p = i * 3;
                            data[p] = _value.floatPtrValue[p];
                            data[p + 1] = _value.floatPtrValue[p + 1];
                            data[p + 2] = _value.floatPtrValue[p + 2];
                        }
                        value->setFloats(0, data, _count * 3);
                        //delete[] data;
                    }
                }
                    break;
                case VECTOR4: {
                    if (!_isArray) {
                        Float* data = value->getData();
                        for (int i = 0; i < _count; ++i) {
                            int p = i * 4;
                            data[p] = _value.floats[p];
                            data[p + 1] = _value.floats[p + 1];
                            data[p + 2] = _value.floats[p + 2];
                            data[p + 3] = _value.floats[p + 3];
                        }
                    }
                    else {
                        Float* data = value->getData();
                        for (int i = 0; i < _count; ++i) {
                            int p = i * 4;
                            data[p] = _value.floatPtrValue[p];
                            data[p + 1] = _value.floatPtrValue[p + 1];
                            data[p + 2] = _value.floatPtrValue[p + 2];
                            data[p + 3] = _value.floatPtrValue[p + 3];
                        }
                        value->setFloats(0, data, _count * 4);
                        //delete[] data;
                    }
                }
                    break;
                case NONE:
                case MATRIX:
                //case METHOD:
                case SAMPLER:
                    // Unsupported material parameter types for animation.
                    break;
                default:
                    break;
            }
        }
        break;
    }
}

void MaterialParameter::setAnimationPropertyValue(int propertyId, AnimationValue* value, float blendWeight)
{
    GP_ASSERT(value);
    GP_ASSERT(blendWeight >= 0.0f && blendWeight <= 1.0f);

    switch (propertyId)
    {
        case ANIMATE_UNIFORM:
        {
            switch (_type)
            {
                case FLOAT:
                    if (!_isArray) {
                        _value.floatValue = Curve::lerp(blendWeight, _value.floatValue, value->getFloat(0));
                    }
                    else {
                        applyAnimationValue(value, blendWeight, 1);
                    }
                    break;
                case INT:
                    if (!_isArray) {
                        _value.intValue = Curve::lerp(blendWeight, _value.intValue, value->getFloat(0));
                    }
                    else {
                        GP_ASSERT(_value.intPtrValue);
                        for (unsigned int i = 0; i < _count; i++)
                            _value.intPtrValue[i] = Curve::lerp(blendWeight, _value.intPtrValue[i], value->getFloat(i));
                    }
                    break;
                case VECTOR2:
                    applyAnimationValue(value, blendWeight, 2);
                    break;
                case VECTOR3:
                    applyAnimationValue(value, blendWeight, 3);
                    break;
                case VECTOR4:
                    applyAnimationValue(value, blendWeight, 4);
                    break;
                case NONE:
                case MATRIX:
                //case METHOD:
                case SAMPLER:
                    // Unsupported material parameter types for animation.
                    break;
                default:
                    break;
            }
        }
        break;
    }
}

void MaterialParameter::applyAnimationValue(AnimationValue* value, float blendWeight, int components)
{
    GP_ASSERT(value);

    float* floatValue = _isArray ? _value.floatPtrValue : _value.floats;
    GP_ASSERT(floatValue);

    unsigned int count = _count * components;
    for (unsigned int i = 0; i < count; i++)
        floatValue[i] = Curve::lerp(blendWeight, floatValue[i], value->getFloat(i));
}

void MaterialParameter::cloneInto(MaterialParameter* materialParameter) const
{
    GP_ASSERT(materialParameter);
    materialParameter->_type = _type;
    materialParameter->_count = _count;
    materialParameter->_dynamicAlloc = _dynamicAlloc;
    materialParameter->_uniform = _uniform;
    switch (_type)
    {
    case NONE:
        break;
    case FLOAT:
        if (!_isArray) {
            materialParameter->setFloat(_value.floatValue);
        }
        else {
            materialParameter->setFloatArray(_value.floatPtrValue, _count);
        }
        break;
    case INT:
        if (!_isArray) {
            materialParameter->setInt(_value.intValue);
        }
        else {
            materialParameter->setIntArray(_value.intPtrValue, _count);
        }
        break;
    case VECTOR2:
    {
        if (!_isArray) {
            materialParameter->setVector2(Vector2(_value.floats[0], _value.floats[1]));
        }
        else {
            Vector2* value = reinterpret_cast<Vector2*>(_value.floatPtrValue);
            GP_ASSERT(value);
            materialParameter->setVector2Array(value, _count);
        }
        break;
    }   
    case VECTOR3:
    {
        if (!_isArray) {
            materialParameter->setVector3(Vector3(_value.floats[0], _value.floats[1], _value.floats[2]));
        }
        else {
            Vector3* value = reinterpret_cast<Vector3*>(_value.floatPtrValue);
            GP_ASSERT(value);
            materialParameter->setVector3Array(value, _count);
        }
        break;
    }
    case VECTOR4:
    {
        if (!_isArray) {
            materialParameter->setVector4(Vector4(_value.floats[0], _value.floats[1], _value.floats[2], _value.floats[3]));
        }
        else {
            Vector4* value = reinterpret_cast<Vector4*>(_value.floatPtrValue);
            GP_ASSERT(value);
            materialParameter->setVector4Array(value, _count);
        }
        break;
    }
    case MATRIX:
    {
        if (!_isArray) {
            materialParameter->setMatrix(Matrix(_value.floats));
        }
        else {
            Matrix* value = reinterpret_cast<Matrix*>(_value.floatPtrValue);
            GP_ASSERT(value);
            materialParameter->setMatrixArray(value, _count);
        }
        break;
    }
    case SAMPLER:
        if (!_isArray) {
            materialParameter->setSampler(_value.samplerValue);
        }
        else {
            materialParameter->setSamplerArray(_value.samplerArrayValue, _count);
        }
        break;
    /*case METHOD:
        materialParameter->_value.method = _value.method;
        GP_ASSERT(materialParameter->_value.method);
        materialParameter->_value.method->addRef();
        break;*/
    default:
        GP_ERROR("Unsupported material parameter type(%d).", _type);
        break;
    }

    if (_methodBinding) {
        materialParameter->_methodBinding = _methodBinding;
        _methodBinding->addRef();
    }
    
    NodeCloneContext context;
    this->AnimationTarget::cloneInto(materialParameter, context);
}

MaterialParameter::MethodBinding::MethodBinding(MaterialParameter* param) :
    _parameter(param), _autoBinding(false)
{
}



Serializable* MaterialParameter::createObject() {
    return new MaterialParameter("");
}


std::string MaterialParameter::enumToString(const std::string& enumName, int value)
{
    if (enumName.compare("mgp::MaterialParameter::Type") == 0)
    {
        switch (value)
        {
            case static_cast<int>(FLOAT) :
                return "FLOAT";
            case static_cast<int>(INT) :
                return "INT";
            case static_cast<int>(VECTOR2) :
                return "VECTOR2";
            case static_cast<int>(VECTOR3) :
                return "VECTOR3";
            case static_cast<int>(VECTOR4) :
                return "VECTOR4";
            case static_cast<int>(MATRIX) :
                return "MATRIX";
            case static_cast<int>(SAMPLER) :
                return "SAMPLER";
            default:
                return "NONE";
        }
    }
    return "";
}

int MaterialParameter::enumParse(const std::string& enumName, const std::string& str)
{
    if (enumName.compare("mgp::MaterialParameter::Type") == 0)
    {
        if (str.compare("FLOAT") == 0)
            return static_cast<int>(FLOAT);
        else if (str.compare("NONE") == 0)
            return static_cast<int>(NONE);
        else if (str.compare("INT") == 0)
            return static_cast<int>(INT);
        else if (str.compare("VECTOR2") == 0)
            return static_cast<int>(VECTOR2);
        else if (str.compare("VECTOR3") == 0)
            return static_cast<int>(VECTOR3);
        else if (str.compare("VECTOR4") == 0)
            return static_cast<int>(VECTOR4);
        else if (str.compare("MATRIX") == 0)
            return static_cast<int>(MATRIX);
        else if (str.compare("SAMPLER") == 0)
            return static_cast<int>(SAMPLER);
    }

    return static_cast<int>(0);
}

/**
 * @see Serializable::getClassName
 */
std::string MaterialParameter::getClassName() {
    return "mgp::MaterialParameter";
}

/**
 * @see Serializable::onSerialize
 */
void MaterialParameter::onSerialize(Serializer* serializer) {
    serializer->writeString("name", getName(), "");
    serializer->writeEnum("type", "mgp::MaterialParameter::Type", static_cast<int>(_type), -1);
    serializer->writeInt("count", _count, 1);

    switch (_type) {
    case MaterialParameter::FLOAT:
        if (!_isArray) {
            serializer->writeFloat("value", _value.floatValue, 0);
        }
        else {
            serializer->writeFloatArray("value", _value.floatPtrValue, _count);
        }
        break;
    case MaterialParameter::INT:
        if (!_isArray) {
            serializer->writeInt("value", _value.intValue, 0);
        }
        else {
            serializer->writeIntArray("value", _value.intPtrValue, _count);
        }
        break;
    case MaterialParameter::VECTOR2:
        if (!_isArray) {
            serializer->writeFloatArray("value", _value.floats, 2);
        }
        else {
            serializer->writeFloatArray("value", _value.floatPtrValue, 2);
        }
        break;
    case MaterialParameter::VECTOR3:
        if (!_isArray) {
            serializer->writeFloatArray("value", _value.floats, 3);
        }
        else {
            serializer->writeFloatArray("value", _value.floatPtrValue, 3);
        }
        break;
    case MaterialParameter::VECTOR4:
        if (!_isArray) {
            serializer->writeFloatArray("value", _value.floats, 4);
        }
        else {
            serializer->writeFloatArray("value", _value.floatPtrValue, 4);
        }
        break;
    case MaterialParameter::MATRIX:
        if (!_isArray) {
            serializer->writeFloatArray("value", _value.floats, 16);
        }
        else {
            serializer->writeFloatArray("value", _value.floatPtrValue, 16);
        }
        break;
    case MaterialParameter::SAMPLER:
        if (!_isArray) {
            serializer->writeObject("value", (Texture*)_value.samplerValue);
        }
        else {
            serializer->writeList("value", _count);
            for (int i = 0; i < _count; ++i) {
                serializer->writeObject(NULL, (Texture*)_value.samplerArrayValue[i]);
            }
            serializer->finishColloction();
        }
        break;
    }

}

/**
 * @see Serializable::onDeserialize
 */
void MaterialParameter::onDeserialize(Serializer* serializer) {
    serializer->readString("name", _name, "");
    _type = static_cast<MaterialParameter::Type>(serializer->readEnum("minFilter", "mgp::MaterialParameter::Type", -1));
    _count = serializer->readInt("count", 1);

    switch (_type) {
    case MaterialParameter::FLOAT:
        if (!_isArray) {
            setFloat(serializer->readFloat("value", 0));
        }
        else {
            float data[1024];
            int size = serializer->readFloatArray("value", (float**)&data);
            setFloatArray(data, size, true);
        }
        break;
    case MaterialParameter::INT: {
        if (!_isArray) {
            setInt(serializer->readInt("value", 0));
        }
        else {
            int data[1024];
            int size = serializer->readIntArray("value", (int**)&data);
            setIntArray(data, size, true);
        }
        break;
    }
    case MaterialParameter::VECTOR2: {
        Vector2 v;
        serializer->readVector("value", v);
        setVector2(v);
        break;
    }
    case MaterialParameter::VECTOR3: {
        Vector3 v;
        serializer->readVector("value", v);
        setVector3(v);
        break;
    }
    case MaterialParameter::VECTOR4: {
        Vector4 v;
        serializer->readVector("value", v);
        setVector4(v);
        break;
    }
    case MaterialParameter::MATRIX: {
        Matrix v;
        serializer->readMatrix("value", v);
        setMatrix(v);
        break;
    }
    case MaterialParameter::SAMPLER: {
        if (!_isArray) {
            Texture* tex = dynamic_cast<Texture*>(serializer->readObject("value"));
            _value.samplerValue = tex;
        }
        else {
            int size = serializer->readList("value");
            std::vector<Texture*> samplaers;
            for (int i = 0; i < size; ++i) {
                Texture* tex = dynamic_cast<Texture*>(serializer->readObject("value"));
                samplaers.push_back(tex);
            }
            serializer->finishColloction();

            setSamplerArray((const Texture**)samplaers.data(), size, true);
        }
        break;
    }
    }
}


}
