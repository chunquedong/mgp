#include "base/Base.h"
#include "Label.h"
#include "Form.h"
#include "ModalLayer.h"

namespace mgp
{

Label::Label() : _text(""), _font(NULL)
{
    _className = "Label";
}

Label::~Label()
{
}

void Label::onSerialize(Serializer* serializer) {
    Control::onSerialize(serializer);
    serializer->writeString("text", _text.c_str(), "");
}

void Label::onDeserialize(Serializer* serializer) {
    Control::onDeserialize(serializer);
    serializer->readString("text", _text, "");
}


void Label::addListener(Control::Listener* listener, Listener::EventType eventFlags)
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

void Label::setText(const char* text, bool fireEvent)
{
    if ((text == NULL && _text.length() > 0) || strcmp(text, _text.c_str()) != 0)
    {
        _text = text ? text : "";
        if (isWrapContentSize())
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

void Label::measureSize()
{
    Control::measureSize();

    if (isWrapContentSize() && _font)
    {
        updateFontLayout();
        // Measure bounds based only on normal state so that bounds updates are not always required on state changes.
        // This is a trade-off for functionality vs performance, but changing the size of UI controls on hover/focus/etc
        // is a pretty bad practice so we'll prioritize performance here.
        unsigned int w, h;
        fontLayout.measureText(&w, &h);
        if (h == 0) {
            h = getStyle()->getFontSize();
#ifndef __EMSCRIPTEN__
            h *= 1.4;
#endif
        }
        w += 2;
        h += 2;
        if (_autoSizeW == AUTO_WRAP_CONTENT)
        {
            setMeasureContentWidth(w);
        }
        if (_autoSizeH == AUTO_WRAP_CONTENT)
        {
            setMeasureContentHeight(h);
        }
    }
}

void Label::updateAbsoluteBounds(const Vector2& offset)
{
    Control::updateAbsoluteBounds(offset);

    _textBounds.set((int)_viewportBounds.x+1, (int)_viewportBounds.y+1, _viewportBounds.width-2, _viewportBounds.height-2);

    if (!isWrapContentSize()) {
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
        startBatch(form, _font, 2);
        //_font->drawText(_text.c_str(), _textBounds.x, _textBounds.y, _textColor, fontSize, _text.size(), &_viewportClipBounds);
        fontLayout.drawText(_textBounds, _textColor, getStyle()->getTextAlignment(), &_viewportClipBounds);
        finishBatch(form, _font, view);

        return 1;
    }

    return 0;
}

Toast::Toast()
{
    _className = "Toast";
    setAlignment(Control::ALIGN_VCENTER_HCENTER);
    setPadding(10);
}

void Toast::animationEvent(AnimationClip* clip, EventType type)
{
    if (type == AnimationClip::Listener::END) {
        this->getTopLevelForm()->getOverlay()->remove(this);
    }
}

void Toast::show(Control* any)
{
    GP_ASSERT(any);
    any->getTopLevelForm()->getOverlay()->add(this, 0);

    unsigned int keyTimes[] = { 0, 1000, 3000, 4000 };
    float keyValues[] = { 0.0, 1.0, 1.0, 0.0 };
    auto anim = this->createAnimation("toast", Control::ANIMATE_OPACITY, 4, keyTimes, keyValues, Curve::LINEAR);
    anim->getClip()->addEndListener(this);
    anim->getClip()->play();
}

void Toast::showToast(Control* any, const char* message)
{
    auto toast = Control::create<Toast>("toast");
    toast->setText(message);
    toast->show(any);
}

}
