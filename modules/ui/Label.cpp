#include "base/Base.h"
#include "Label.h"

namespace mgp
{

Label::Label() : _text(""), _font(NULL)
{
}

Label::~Label()
{
}

UPtr<Label> Label::create(const char* id, Style* style)
{
    Label* label = new Label();
    label->_id = id ? id : "";
    label->initialize("Label", style, NULL);
    return UPtr<Label>(label);
}

Control* Label::create(Style* style, Properties* properties)
{
    Label* label = new Label();
	label->initialize("Label", style, properties);
    return label;
}

void Label::initialize(const char* typeName, Style* style, Properties* properties)
{
    Control::initialize(typeName, style, properties);

	if (properties)
	{
		const char* text = properties->getString("text");
		if (text)
		{
			_text = text;
		}
	}
}

const char* Label::getTypeName() const
{
    return "Label";
}


void Label::addListener(Control::Listener* listener, int eventFlags)
{
    if ((eventFlags & Control::Listener::TEXT_CHANGED) == Control::Listener::TEXT_CHANGED)
    {
        GP_ERROR("TEXT_CHANGED event is not applicable to this control.");
    }
    if ((eventFlags & Control::Listener::VALUE_CHANGED) == Control::Listener::VALUE_CHANGED)
    {
        GP_ERROR("VALUE_CHANGED event is not applicable to this control.");
    }

    Control::addListener(listener, eventFlags);
}

void Label::setText(const char* text)
{
    if ((text == NULL && _text.length() > 0) || strcmp(text, _text.c_str()) != 0)
    {
        _text = text ? text : "";
        if (_autoSize != AUTO_SIZE_NONE)
            setDirty(DIRTY_BOUNDS);
        else {
            updateFontLayout();
        }
    }
}

const char* Label::getText()
{
    return _text.c_str();
}

std::string& Label::getDisplayedText() {
    return _text;
}

void Label::updateFontLayout() {
    if (!_font) return;
    auto text = getDisplayedText();
    fontLayout.update(_font, getStyle()->getFontSize(), text.c_str(), text.size());
}

void Label::update(float elapsedTime)
{
    Control::update(elapsedTime);

    // Update text opacity each frame since opacity is updated in Control::update.
    _textColor = getStyle()->getTextColor();
    _textColor.w *= _opacity;
}

void Label::updateState(State state)
{
    Control::updateState(state);

    _font = getStyle()->getFont();
}

void Label::updateBounds()
{
    Control::updateBounds();

    if (_autoSize != AUTO_SIZE_NONE && _font)
    {
        updateFontLayout();
        // Measure bounds based only on normal state so that bounds updates are not always required on state changes.
        // This is a trade-off for functionality vs performance, but changing the size of UI controls on hover/focus/etc
        // is a pretty bad practice so we'll prioritize performance here.
        unsigned int w, h;
        fontLayout.measureText(&w, &h);
        if (_autoSize & AUTO_SIZE_WIDTH)
        {
            setWidthInternal(w + getPadding().left + getPadding().right);
        }
        if (_autoSize & AUTO_SIZE_HEIGHT)
        {
            setHeightInternal(h + getPadding().top + getPadding().bottom);
        }
    }
}

void Label::updateAbsoluteBounds(const Vector2& offset)
{
    Control::updateAbsoluteBounds(offset);

    _textBounds.set((int)_viewportBounds.x, (int)_viewportBounds.y, _viewportBounds.width, _viewportBounds.height);

    if (_autoSize == AUTO_SIZE_NONE) {
        updateFontLayout();
    }
}

unsigned int Label::drawText(Form* form, const Rectangle& clip, RenderInfo* view)
{
    // Draw the text.
    if (_text.size() > 0 && _font)
    {
        Control::State state = getState();
        unsigned int fontSize = getStyle()->getFontSize();

        //SpriteBatch* batch = _font->getSpriteBatch(fontSize);
        startBatch(form, _font);
        //_font->drawText(_text.c_str(), _textBounds.x, _textBounds.y, _textColor, fontSize, _text.size(), &_viewportClipBounds);
        fontLayout.drawText(_textBounds, _textColor, getStyle()->getTextAlignment(), &_viewportClipBounds);
        finishBatch(form, _font, view);

        return 1;
    }

    return 0;
}

}
