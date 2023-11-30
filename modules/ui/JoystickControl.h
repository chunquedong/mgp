#ifndef JOYSTICKCONTROL_H_
#define JOYSTICKCONTROL_H_

#include "Control.h"

namespace mgp
{

/**
 * Defines a control representing a joystick (axis).
 *
 * This is used in virtual Gamepad instances.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-UI_Forms
 */
class JoystickControl : public Control
{
    friend class Container;
    friend class Gamepad;
	friend class ControlFactory;
    friend class Control;
public:

    /**
     * Extends ScriptTarget::getTypeName() to return the type name of this class.
     *
     * Child controls should override this function to return the correct type name.
     *
     * @return The type name of this class: "JoystickControl"
     * @see ScriptTarget::getTypeName()
     */
    const char* getTypeName() const;

    /**
     * Add a listener to be notified of specific events affecting
     * this control.  Event types can be OR'ed together.
     * E.g. To listen to touch-press and touch-release events,
     * pass <code>Control::Listener::TOUCH | Control::Listener::RELEASE</code>
     * as the second parameter.
     *
     * @param listener The listener to add.
     * @param eventFlags The events to listen for.
     */
    void addListener(Control::Listener* listener, int eventFlags);

    /**
     * Gets the value (2-dimensional direction) of the joystick.
     * 
     * @return The value of the joystick.
     */
    const Vector2& getValue() const;

    /**
     * Sets whether relative positioning is enabled or not.
     * 
     * Note: The default behavior is absolute positioning, and not relative.
     *
     * @param relative Whether relative positioning should be enabled or not.
     */
    void setRelative(bool relative);

    /**
     * Gets whether absolute positioning is enabled or not.
     * 
     * Note: The default behavior is absolute positioning, and not relative.
     *
     * @return <code>true</code> if relative positioning is enabled; <code>false</code> otherwise.
     */
    bool isRelative() const;

    /**
     * Gets the index of this joystick across all joysticks on a form.
     *
     * @return The index of this joystick on a form.
     */
    unsigned int getIndex() const;

    /**
     * Sets the radius of joystick motion
     *
     * @param radius The radius to be set.
     * @param isPercentage If the radius value is a percentage value of the relative size of this control
     */
    void setRadius(float radius, bool isPercentage = false);

    /**
     * Gets the radius of joystick motion
     *
     * @return The radius of joystick motion
     */
    float getRadius() const;

    /**
      * Determines if the radius of joystick motion is a percentage value of the relative size of this control
      *
     * @return True if the radius of joystick motion is a percentage value of the relative size of this control
     */
    bool isRadiusPercentage() const;

protected:
    
    /**
     * Constructor.
     */
    JoystickControl();

    /**
     * Destructor.
     */
    virtual ~JoystickControl();


    /**
     * @see Control::initialize
     */
    void initialize(const char* typeName, Style* style, Properties* properties);

    /**
     * Touch callback on touch events.  Controls return true if they consume the touch event.
     *
     * @param evt The touch event that occurred.
     * @param x The x position of the touch in pixels. Left edge is zero.
     * @param y The y position of the touch in pixels. Top edge is zero.
     * @param contactIndex The order of occurrence for multiple touch contacts starting at zero.
     *
     * @return Whether the touch event was consumed by the control.
     *
     * @see MotionEvent::MotionType
     */
    bool touchEvent(MotionEvent::MotionType evt, int x, int y, unsigned int contactIndex);

    /**
     * @see Control::updateAbsoluteBounds
     */
    void updateAbsoluteBounds(const Vector2& offset);
    void updateBounds();

    /**
     * @see Control::drawImages
     */
    unsigned int drawImages(Form* form, const Rectangle& clip, RenderInfo* view);

private:

    JoystickControl(const JoystickControl& copy);

    void updateAbsoluteSizes();

    void setBoundsBit(bool set, int& bitSetOut, int bit);

    float _radiusCoord;
    float _radiusPixels;

    Vector2 _pressOffset;
    bool _relative;
    Vector2 _value;
    Vector2 _displacement;
    unsigned int _index;

    ThemeImage* outer;
    ThemeImage* inner;
};

}

#endif
