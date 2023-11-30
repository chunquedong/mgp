#include "base/Base.h"
#include "Button.h"
//#include "platform/Gamepad.h"

namespace mgp
{

Button::Button() : _dataBinding(0)
{
    _canFocus = true;
    setPadding(12, 12, 12, 12);
}

Button::~Button()
{
}

void Button::initialize(const char* typeName, Style* style, Properties* properties)
{
    Label::initialize(typeName, style, properties);

    if (properties)
    {
        // Different types of data bindings can be named differently in a button namespace.
        // Gamepad button mappings have the name "mapping" and correspond to Gamepad::ButtonMapping enums.
        // const char* mapping = properties->getString("mapping");
        // if (mapping)
        // {
        //     _dataBinding = Gamepad::getButtonMappingFromString(mapping);
        // }
    }
}

const char* Button::getTypeName() const
{
    return "Button";
}

const unsigned int Button::getDataBinding() const
{
    return _dataBinding;
}

void Button::setDataBinding(unsigned int dataBinding)
{
    _dataBinding = dataBinding;
}

}
