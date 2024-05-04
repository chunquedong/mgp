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
    virtual bool keyEvent(Keyboard key) { return false; }

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
    
    virtual void onSetup() {}
    virtual void onTeardown() {}
};
}


#endif