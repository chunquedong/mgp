// Implementation of base platform-agnostic platform functionality.
#include "base/Base.h"
#include "Platform.h"
#include "app/Game.h"
//#include "script/ScriptController.h"

namespace mgp
{

void Platform::keyEventInternal(Keyboard evt)
{
    Game::getInstance()->keyEventInternal(evt);
}

bool Platform::mouseEventInternal(Mouse evt)
{
    return Game::getInstance()->mouseEventInternal(evt);
}

void Platform::resizeEventInternal(unsigned int width, unsigned int height)
{
    Game::getInstance()->resizeEventInternal(width, height);
}

// void Platform::gamepadEventConnectedInternal(GamepadHandle handle,  unsigned int buttonCount, unsigned int joystickCount, unsigned int triggerCount, const char* name)
// {
//     Gamepad::add(handle, buttonCount, joystickCount, triggerCount, name);
// }

// void Platform::gamepadEventDisconnectedInternal(GamepadHandle handle)
// {
//     Gamepad::remove(handle);
// }

// void Platform::gamepadButtonPressedEventInternal(GamepadHandle handle, Gamepad::ButtonMapping mapping)
// {
//     Gamepad* gamepad = Gamepad::getGamepad(handle);
//     if (gamepad)
//     {
//         unsigned int newButtons = gamepad->_buttons | (1 << mapping);
//         gamepad->setButtons(newButtons);
//         //Form::gamepadButtonEventInternal(gamepad);
//     }
// }

// void Platform::gamepadButtonReleasedEventInternal(GamepadHandle handle, Gamepad::ButtonMapping mapping)
// {
//     Gamepad* gamepad = Gamepad::getGamepad(handle);
//     if (gamepad)
//     {
//         unsigned int newButtons = gamepad->_buttons & ~(1 << mapping);
//         gamepad->setButtons(newButtons);
//         //Form::gamepadButtonEventInternal(gamepad);
//     }
// }

// void Platform::gamepadTriggerChangedEventInternal(GamepadHandle handle, unsigned int index, float value)
// {
//     Gamepad* gamepad = Gamepad::getGamepad(handle);
//     if (gamepad)
//     {
//         gamepad->setTriggerValue(index, value);
//         //Form::gamepadTriggerEventInternal(gamepad, index);
//     }
// }

// void Platform::gamepadJoystickChangedEventInternal(GamepadHandle handle, unsigned int index, float x, float y)
// {
//     Gamepad* gamepad = Gamepad::getGamepad(handle);
//     if (gamepad)
//     {
//         gamepad->setJoystickValue(index, x, y);
//         //Form::gamepadJoystickEventInternal(gamepad, index);
//     }
// }

}
