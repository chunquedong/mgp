#include "base/Base.h"
#include "Button.h"
//#include "platform/Gamepad.h"

namespace mgp
{

Button::Button() : _dataBinding(0)
{
    _canFocus = true;
    setPadding(9, 12, 9, 12);
    _className = "Button";
}

Button::~Button()
{
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
