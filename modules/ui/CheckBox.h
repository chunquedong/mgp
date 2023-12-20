#ifndef CHECKBOX_H_
#define CHECKBOX_H_

#include "Theme.h"
#include "base/Properties.h"
#include "Button.h"

namespace mgp
{

/**
 * Defines a checkbox control.  
 *
 * This is a button that can be enabled or disabled.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-UI_Forms
 */
class CheckBox : public Button
{
    friend class Control;

public:

    /**
     * Gets whether this checkbox is checked.
     *
     * @return Whether this checkbox is checked.
     */
    bool isChecked();

    /**
     * Sets whether the checkbox is checked.
     *
     * @param checked TRUE if the checkbox is checked; FALSE if the checkbox is not checked.
     */
    void setChecked(bool checked);

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
    virtual void addListener(Control::Listener* listener, int eventFlags);

protected:

    /**
     * Constructor.
     */
    CheckBox();

    /**
     * Destructor.
     */
    ~CheckBox();

    /**
     * Event handled when an object is asked to serialize itself.
     * 
     * @param serializer The serializer to write properties to.
     */
    virtual void onSerialize(Serializer* serializer);

    /**
     * Event handled when an object properties are being deserialized.
     *
     * @param serializer The serializer to read properties from.
     */
    virtual void onDeserialize(Serializer* serializer);

    /**
     * Keyboard callback on key events.
     *
     * @see Keyboard::KeyEvent
     * @see Keyboard::Key
     */
    bool keyEvent(Keyboard::KeyEvent evt, int key);

    /**
     * @see Control#controlEvent
     */
    void controlEvent(Control::Listener::EventType evt);

    /**
     * @see Control::updateState
     */
    void updateState(State state);

    /**
     * @see Control::updateBounds
     */
    void measureSize();

    /**
     * @see Control::updateAbsoluteBounds
     */
    void updateAbsoluteBounds(const Vector2& offset);

    /**
     * @see Control::drawImages
     */
    unsigned int drawImages(Form* form, const Rectangle& clip, RenderInfo* view);

    /**
     * Whether this checkbox is currently checked.
     */
    bool _checked;

    /**
     * The ThemeImage to display for the checkbox.
     */
    ThemeImage* _image;

private:

    /*
     * Constructor.
     */
    CheckBox(const CheckBox& copy);
};

}

#endif
