#include "TextBox.h"
#include "platform/Toolkit.h"

namespace mgp
{

TextBox::TextBox() : _caretLocation(0), _lastKeypress(0), _fontSize(0), _caretImage(NULL), _passwordChar('*'), _inputMode(TEXT), 
    _ctrlPressed(false), _shiftPressed(false)
{
    _canFocus = true;
    setPadding(8, 8, 8, 8);
    _className = "TextBox";
}

TextBox::~TextBox()
{
}

void TextBox::onSerialize(Serializer* serializer) {
    Label::onSerialize(serializer);
}

void TextBox::onDeserialize(Serializer* serializer) {
    Label::onDeserialize(serializer);
    std::string inputMode;
    serializer->readString("inputMode", inputMode, "");
    _inputMode = getInputMode(inputMode.c_str());
}

void TextBox::addListener(Control::Listener* listener, int eventFlags)
{
    if ((eventFlags & Control::Listener::VALUE_CHANGED) == Control::Listener::VALUE_CHANGED)
    {
        GP_ERROR("VALUE_CHANGED event is not applicable to this control.");
    }

    Control::addListener(listener, eventFlags);
}

int TextBox::getLastKeypress()
{
    return _lastKeypress;
}

unsigned int TextBox::getCaretLocation() const
{
    return _caretLocation;
}

void TextBox::setCaretLocation(unsigned int index)
{
    _caretLocation = index;
    if (_caretLocation > _text.length())
        _caretLocation = (unsigned int)_text.length();
}

bool TextBox::touchEvent(MotionEvent::MotionType evt, int x, int y, unsigned int contactIndex)
{
    if (getState() == ACTIVE) {
        switch (evt)
        {
        case MotionEvent::press:
            setCaretLocation(x, y);
            break;
        case MotionEvent::touchMove:
            setCaretLocation(x, y);
            break;
        default:
            break;
        }
    }

    return Label::touchEvent(evt, x, y, contactIndex);
}

static bool isWhitespace(char c)
{
    switch (c)
    {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        return true;

    default:
        return false;
    }
}

static unsigned int findNextWord(const std::string& text, unsigned int from, bool backwards)
{
    int pos = (int)from;
    if (backwards)
    {
        if (pos > 0)
        {
            // Moving backwards: skip all consecutive whitespace characters
            while (pos > 0 && isWhitespace(text.at(pos-1)))
                --pos;
            // Now search back to the first whitespace character
            while (pos > 0 && !isWhitespace(text.at(pos-1)))
                --pos;
        }
    }
    else
    {
        const int len = (const int)text.length();
        if (pos < len)
        {
            // Moving forward: skip all consecutive non-whitespace characters
            ++pos;
            while (pos < len && !isWhitespace(text.at(pos)))
                ++pos;
            // Now search for the first non-whitespace character
            while (pos < len && isWhitespace(text.at(pos)))
                ++pos;
        }
    }

    return (unsigned int)pos;
}

bool TextBox::keyEvent(Keyboard::KeyEvent evt, int key)
{
    switch (evt)
    {
        case Keyboard::KEY_PRESS:
        {
            switch (key)
            {
            	case Keyboard::KEY_SHIFT:
            	{
                    _shiftPressed = true;
                    break;
            	}
                case Keyboard::KEY_CTRL:
                {
                    _ctrlPressed = true;
                    break;
                }
                case Keyboard::KEY_HOME:
                {
                    _caretLocation = 0;
                    break;
                }
                case Keyboard::KEY_END:
                {
                    _caretLocation = _text.length();
                    break;
                }
                case Keyboard::KEY_DELETE:
                {
                    if (_caretLocation < _text.length())
                    {
                        int newCaretLocation;
                        if (_ctrlPressed)
                        {
                            newCaretLocation = findNextWord(getDisplayedText(), _caretLocation, false);
                        }
                        else
                        {
                            newCaretLocation = _caretLocation + 1;
                        }
                        _text.erase(_caretLocation, newCaretLocation - _caretLocation);
                        notifyListeners(Control::Listener::TEXT_CHANGED);
                        updateFontLayout();
                    }
                    break;
                }
                case Keyboard::KEY_TAB:
                {
                    // Allow tab to move the focus forward.
                    return false;
                }
                case Keyboard::KEY_LEFT_ARROW:
                {
                    if (_caretLocation > 0)
                    {
                        if (_ctrlPressed)
                        {
                            _caretLocation = findNextWord(getDisplayedText(), _caretLocation, true);
                        }
                        else
                        {
                            --_caretLocation;
                        }
                    }
                    break;
                }
                case Keyboard::KEY_RIGHT_ARROW:
                {
                    if (_caretLocation < _text.length())
                    {
                        if (_ctrlPressed)
                        {
                            _caretLocation = findNextWord(getDisplayedText(), _caretLocation, false);
                        }
                        else
                        {
                            ++_caretLocation;
                        }
                    }
                    break;
                }
                case Keyboard::KEY_UP_ARROW:
                {
                    // TODO: Support multiline
                    break;
                }
                case Keyboard::KEY_DOWN_ARROW:
                {
                    // TODO: Support multiline
                    break;
                }
                case Keyboard::KEY_BACKSPACE:
                {
                    if (_caretLocation > 0)
                    {
                        int newCaretLocation;
                        if (_ctrlPressed)
                        {
                            newCaretLocation = findNextWord(getDisplayedText(), _caretLocation, true);
                        }
                        else
                        {
                            newCaretLocation = _caretLocation - 1;
                        }
                        _text.erase(newCaretLocation, _caretLocation - newCaretLocation);
                        _caretLocation = newCaretLocation;
                        notifyListeners(Control::Listener::TEXT_CHANGED);
                        updateFontLayout();
                    }
                    break;
                }
            }
            break;
        }

        case Keyboard::KEY_CHAR:
        {
            switch (key)
            {
                case Keyboard::KEY_RETURN:
                    // TODO: Support multi-line
                    notifyListeners(Control::Listener::ACTIVATED);
                    break;
                case Keyboard::KEY_ESCAPE:
                    break;
                case Keyboard::KEY_BACKSPACE:
                    break;
                case Keyboard::KEY_TAB:
                    // Allow tab to move the focus forward.
                    return false;
                default:
                {
                    // Insert character into string, only if our font supports this character
                    if (_shiftPressed && islower(key))
                    {
                        key = toupper(key);
                    }
                    // Insert character into string, only if our font supports this character
                    if (_font && _font->isCharacterSupported(key))
                    {
                        if (_caretLocation <= _text.length())
                        {
                            _text.insert(_caretLocation, 1, (char)key);
                            ++_caretLocation;
                        }

                        notifyListeners(Control::Listener::TEXT_CHANGED);
                        updateFontLayout();
                    }
                    break;
                }
            
                break;
            }
            break;
        }
        case Keyboard::KEY_RELEASE:
            switch (key)
            {
            	case Keyboard::KEY_SHIFT:
            	{
                    _shiftPressed = false;
                    break;
             	 }
                case Keyboard::KEY_CTRL:
                {
                    _ctrlPressed = false;
                    break;
                }
            }
    }

    _lastKeypress = key;

    return Label::keyEvent(evt, key);
}

void TextBox::controlEvent(Control::Listener::EventType evt)
{
    Label::controlEvent(evt);

    switch (evt)
    {
    case Control::Listener::FOCUS_GAINED:
        Toolkit::cur()->displayKeyboard(true);
        break;

    case Control::Listener::FOCUS_LOST:
        Toolkit::cur()->displayKeyboard(false);
        break;
    default:
        break;
    }
}

void TextBox::updateState(State state)
{
    Label::updateState(state);

    _fontSize = getStyle()->getFontSize();
    _caretImage = getTheme()->getImage("empty");
}

unsigned int TextBox::drawImages(Form* form, const Rectangle& clip, RenderInfo* view)
{
    Control::State state = getState();

    if (_caretImage && (state == ACTIVE || hasFocus()))
    {
        // Draw the cursor at its current location.
        const Rectangle& region = _caretImage->getRegion();
        if (!region.isEmpty())
        {
            //const Vector4& uvs = _caretImage->getUVs();
            Vector4 color = _textColor;
            color.w *= _opacity;

            float caretWidth = 2;
            Rectangle caretRegion(region.x + 1, region.y + 1, region.width - 2, region.height - 2);

            Font* font = getStyle()->getFont();
            unsigned int fontSize = getStyle()->getFontSize();
            Vector2 point;
            getCaretLocation(&point);
            point.x += _absoluteBounds.x;
            point.y += _absoluteBounds.y;


            SpriteBatch* batch = getStyle()->getTheme()->getSpriteBatch();
            startBatch(form, batch);
            batch->drawImage(Rectangle(point.x - caretWidth * 0.5f, point.y, caretWidth, fontSize*1.5), caretRegion, color, &_viewportClipBounds);
            finishBatch(form, batch, view);

            return 1;
        }
    }

    return 0;
}

unsigned int TextBox::drawText(Form* form, const Rectangle& clip, RenderInfo* view)
{
    if (_text.size() <= 0)
        return 0;

    // Draw the text.
    if (_font)
    {
        //Control::State state = getState();
        //unsigned int fontSize = getStyle()->getFontSize();

        //SpriteBatch* batch = _font->getSpriteBatch(fontSize);
        startBatch(form, _font, 2);
        //_font->start();
        fontLayout.drawText(_textBounds, _textColor, getStyle()->getTextAlignment(), &_viewportClipBounds);
        //_font->finish();
        finishBatch(form, _font, view);

        return 1;
    }

    return 0;
}

void TextBox::setText(char const *text)
{
    Label::setText(text);
    if (_caretLocation > _text.length())
    {
        _caretLocation = _text.length();
    }
    notifyListeners(Control::Listener::TEXT_CHANGED);
}

void TextBox::setCaretLocation(int x, int y)
{
    Control::State state = getState();

    Vector2 point(x - (_textBounds.x - _absoluteBounds.x), y - (_textBounds.y - _absoluteBounds.y));

    int index = fontLayout.indexAtPosition(point);
    if (index != -1)
    {
        _caretLocation = index;
    }
    else
    {
        _caretLocation = _text.length();
    }
}

void TextBox::getCaretLocation(Vector2* p)
{
    GP_ASSERT(p);

    *p = fontLayout.positionAtIndex(_caretLocation);
    p->x += (_textBounds.x - _absoluteBounds.x);
    p->y += (_textBounds.y - _absoluteBounds.y);
}

void TextBox::setPasswordChar(char character)
{
    _passwordChar = character;
}

char TextBox::getPasswordChar() const
{
    return _passwordChar;
}

void TextBox::setInputMode(InputMode inputMode)
{
    _inputMode = inputMode;
}

TextBox::InputMode TextBox::getInputMode() const
{
    return _inputMode;
}

TextBox::InputMode TextBox::getInputMode(const char* inputMode)
{
    if (!inputMode)
    {
        return TextBox::TEXT;
    }

    if (strcmp(inputMode, "TEXT") == 0)
    {
        return TextBox::TEXT;
    }
    else if (strcmp(inputMode, "PASSWORD") == 0)
    {
        return TextBox::PASSWORD;
    }
    else
    {
        GP_ERROR("Failed to get corresponding textbox inputmode for unsupported value '%s'.", inputMode);
    }

    // Default.
    return TextBox::TEXT;
}

std::string& TextBox::getDisplayedText()
{
    switch (_inputMode) {
        case PASSWORD:
            _displayedText.insert((size_t)0, _text.length(), _passwordChar);
            break;

        case TEXT:
        default:
            return _text;
            break;
    }

    return _displayedText;
}

}
