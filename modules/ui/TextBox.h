#ifndef TEXTBOX_H_
#define TEXTBOX_H_

#include <string>
#include "Label.h"

namespace mgp
{

/**
 * Defines a text control. 
 *
 * Listeners can listen for a TEXT_CHANGED event, and then query the text box
 * for the last keypress it received.
 * On mobile device you can tap or click within the text box to
 * bring up the virtual keyboard.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-UI_Forms
 */
class TextBox : public Label
{
    friend class Control;
public:

    /**
     * Input modes. Default is Text.
     */
    enum InputMode
    {
        /**
         * Text: Text is displayed directly.
         */
        TEXT = 0x01,

        /**
         * Password: Text is replaced by _passwordChar, which is '*' by default.
         */
        PASSWORD = 0x02
    };

    /**
     * Returns the current location of the caret with the text of this TextBox.
     *
     * @return The current caret location.
     */
    unsigned int getCaretLocation() const;

    /**
     * Sets the location of the caret within this text box.
     *
     * @param index The new location of the caret within the text of this TextBox.
     */
    void setCaretLocation(unsigned int index);

    /**
     * Get the last key pressed within this text box.
     *
     * @return The last key pressed within this text box.
     */
    int getLastKeypress();

    /**
     * Set the character displayed in password mode.
     *
     * @param character Character to display in password mode.
     */
    void setPasswordChar(char character);

    /**
     * Get the character displayed in password mode.
     *
     * @return The character displayed in password mode.
     */
    char getPasswordChar() const;

    /**
     * Set the input mode.
     *
     * @param inputMode Input mode to set.
     */
    void setInputMode(InputMode inputMode);

    /**
     * Get the input mode.
     *
     * @return The input mode.
     */
    InputMode getInputMode() const;

    virtual void addListener(Control::Listener* listener, int eventFlags);

    /**
     * Update the text being edited.
     */
    void setText(char const *text) override;

protected:

    /**
     * Constructor.
     */
    TextBox();

    /**
     * Destructor.
     */
    ~TextBox();


    virtual void onSerialize(Serializer* serializer);

    virtual void onDeserialize(Serializer* serializer);

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
     * Keyboard callback on key events.
     *
     * @param evt The key event that occurred.
     * @param key If evt is KEY_PRESS or KEY_RELEASE then key is the key code from Keyboard::Key.
     *            If evt is KEY_CHAR then key is the unicode value of the character.
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
     * @see Control::drawImages
     */
    unsigned int drawImages(Form* form, const Rectangle& clip, RenderInfo* view);

    /**
     * @see Control::drawText
     */
    unsigned int drawText(Form* form, const Rectangle& clip, RenderInfo* view);

    /**
     * Gets an InputMode by string.
     *
     * @param inputMode The string representation of the InputMode type.
     * @return The InputMode enum value corresponding to the given string.
     */
    static InputMode getInputMode(const char* inputMode);

    /**
     * Get the text which should be displayed, depending on
     * _inputMode.
     *
     * @return The text to be displayed.
     */
    std::string& getDisplayedText();

    /**
     * The current location of the TextBox's caret.
     */
    unsigned int _caretLocation;

    /**
     * The previous position of the TextBox's caret.
     */
    Vector2 _prevCaretLocation;
    
    /**
     * The last character that was entered into the TextBox.
     */
    int _lastKeypress;

    /**
     * The font size to be used in the TextBox.
     */
    unsigned int _fontSize;
    
    /**
     * The Theme::Image for the TextBox's caret.
     */
    ThemeImage* _caretImage;

    /**
     * The character displayed in password mode.
     */
    char _passwordChar;

    /**
     * The mode used to display the typed text.
     */
    InputMode _inputMode;

    /**
     * Indicate if the CTRL key is currently pressed.
     */
    bool _ctrlPressed;

    /**
     * Indicate if the SHIFT key is currently pressed.
     */
    bool _shiftPressed = false;

private:
    std::string _displayedText;

    /**
     * Constructor.
     */
    TextBox(const TextBox& copy);

    void setCaretLocation(int x, int y);

    void getCaretLocation(Vector2* p);
};

}

#endif
