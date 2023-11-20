#ifndef INPUTLISTENER_H_
#define INPUTLISTENER_H_

#include "platform/Keyboard.h"
#include "platform/Mouse.h"

namespace mgp {
class InputListener {
public:
    /**
     * Keyboard callback on keyPress events.
     *
     * @param evt The key event that occurred.
     * @param key If evt is KEY_PRESS or KEY_RELEASE then key is the key code from Keyboard::Key.
     *            If evt is KEY_CHAR then key is the unicode value of the character.
     * 
     * @see Keyboard::KeyEvent
     * @see Keyboard::Key
     */
    virtual void keyEvent(Keyboard key) {}

    /**
     * Mouse callback on mouse events. If the game does not consume the mouse move event or left mouse click event
     * then it is interpreted as a touch event instead.
     *
     * @param evt The mouse event that occurred.
     * @param x The x position of the mouse in pixels. Left edge is zero.
     * @param y The y position of the mouse in pixels. Top edge is zero.
     * @param wheelDelta The number of mouse wheel ticks. Positive is up (forward), negative is down (backward).
     *
     * @return True if the mouse event is consumed or false if it is not consumed.
     *
     * @see MotionEvent::MotionType
     */
    virtual bool mouseEvent(Mouse evt) { return false; }
    
    /**
     * Called when the game window has been resized.
     *
     * This method is called once the game window is created with its initial size
     * and then again any time the game window changes size.
     *
     * @param width The new game window width.
     * @param height The new game window height.
     */
    virtual void resizeEvent(unsigned int width, unsigned int height) {}


    /**
     * Gamepad callback on gamepad events.  Override to receive Gamepad::CONNECTED_EVENT 
     * and Gamepad::DISCONNECTED_EVENT, and store the Gamepad* in order to poll it from update().
     *
     * @param evt The gamepad event that occurred.
     * @param gamepad The gamepad that generated the event.
     */
    //virtual void gamepadEvent(Gamepad::GamepadEvent evt, Gamepad* gamepad);
};
}


#endif