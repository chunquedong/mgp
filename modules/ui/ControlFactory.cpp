#include "base/Base.h"
#include "ControlFactory.h"
#include "Label.h"
#include "Button.h"
#include "CheckBox.h"
#include "RadioButton.h"
#include "ScrollContainer.h"
#include "Slider.h"
#include "TextBox.h"
#include "JoystickControl.h"
#include "ImageControl.h"

#include <algorithm>

namespace mgp
{

static ControlFactory* __controlFactory = NULL;

ControlFactory::ControlFactory() 
{
	registerStandardControls();
}

ControlFactory::ControlFactory(const ControlFactory& copy)
{
}

ControlFactory::~ControlFactory() 
{
}

void ControlFactory::finalize()
{
    SAFE_DELETE(__controlFactory);
}

ControlFactory* ControlFactory::getInstance() 
{
	if (__controlFactory == NULL)
		__controlFactory = new ControlFactory();
	return __controlFactory;
}

bool ControlFactory::registerCustomControl(const char* typeName, ControlActivator activator)
{
    std::string upper(typeName);
    std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);

	if (_registeredControls.find(upper) != _registeredControls.end())
		return false;

	_registeredControls[upper] = activator;
	return true;
}

void ControlFactory::unregisterCustomControl(const char* typeName)
{
    std::string upper(typeName);
    std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);

	std::map<std::string, ControlActivator>::iterator it;
	if ((it = _registeredControls.find(upper)) != _registeredControls.end())
	{
		_registeredControls.erase(it);
	}
}

UPtr<Control> ControlFactory::createControl(const char* typeName, Style* style, Properties* properties)
{
    std::string upper(typeName);
    std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);

    std::map<std::string, ControlActivator>::iterator it = _registeredControls.find(upper);
    if (it == _registeredControls.end())
		return UPtr<Control>(NULL);

    Control* ctrl = (*it->second)(NULL, style, properties, typeName);
    return UPtr<Control>(ctrl);
}

template<typename T>
static Control* templateCreateControl(const char* id, Style* style = NULL, Properties* properties = NULL, const char* typeName = NULL) {
    UPtr<T> t = Control::create<T>(id, style, properties, typeName);
    return t.take();
}

void ControlFactory::registerStandardControls() 
{
    registerCustomControl("LABEL", templateCreateControl<Label>);
    registerCustomControl("BUTTON", templateCreateControl<Button>);
    registerCustomControl("CHECKBOX", templateCreateControl<CheckBox>);
    registerCustomControl("RADIOBUTTON", templateCreateControl<RadioButton>);
    registerCustomControl("CONTAINER", templateCreateControl<Container>);
    registerCustomControl("SCORLLCONTAINER", templateCreateControl<ScrollContainer>);
    registerCustomControl("SLIDER", templateCreateControl<Slider>);
    registerCustomControl("TEXTBOX", templateCreateControl<TextBox>);
    registerCustomControl("JOYSTICK", templateCreateControl<JoystickControl>); // convenience alias
    registerCustomControl("JOYSTICKCONTROL", templateCreateControl<JoystickControl>);
    registerCustomControl("IMAGE", templateCreateControl<ImageControl>);  // convenience alias
    registerCustomControl("IMAGECONTROL", templateCreateControl<ImageControl>);
}

}
