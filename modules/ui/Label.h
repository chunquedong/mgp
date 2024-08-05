#ifndef LABEL_H_
#define LABEL_H_

#include "Control.h"
#include "Theme.h"

namespace mgp
{

/**
 * Defines a label control.
 * 
 * This is capable of rendering text within its border.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-UI_Forms
 */
class Label : public Control
{
    friend class Control;
public:

    /**
     * Set the text for this label to display.
     *
     * @param text The text to display.
     */
    virtual void setText(const char* text, bool fireEvent = true);

    /**
     * Get the text displayed by this label.
     *
     * @return The text displayed by this label.
     */
    const char* getText();

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
    virtual void addListener(Control::Listener* listener, Listener::EventType eventFlags);

    void setMulitline(bool mul) { multiLine = mul; }
protected:

    /**
     * Constructor.
     */
    Label();

    /**
     * Destructor.
     */
    virtual ~Label();


    virtual void onSerialize(Serializer* serializer);

    virtual void onDeserialize(Serializer* serializer);

    /**
     * @see Control::update
     */
    void update(float elapsedTime);

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
     * @see Control::drawText
     */
    virtual unsigned int drawText(Form* form, const Rectangle& clip, RenderInfo* view);

    virtual std::string& getDisplayedText();
    void updateFontLayout();

    /**
     * The text displayed by this label.
     */
    std::string _text;

    /**
     * The font being used to display the label.
     */
    Font* _font;
    
    /**
     * The text color being used to display the label.
     */
    Vector4 _textColor;

    /**
     * The position and size of this control's text area, before clipping.  Used for text alignment.
     */
    Rectangle _textBounds;


    bool multiLine = false;
    FontLayout fontLayout;

private:

    /**
     * Constructor.
     */
    Label(const Label& copy);
};


class Toast : public Label, public AnimationClip::Listener {
    friend class Control;
protected:
    Toast();

    virtual void animationEvent(AnimationClip* clip, EventType type) override;
public:
    void show(Control* any);

    static void showToast(Control* any, const char* message);
};

}

#endif
