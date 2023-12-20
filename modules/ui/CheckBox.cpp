#include "base/Base.h"
#include "CheckBox.h"
#include "platform/Toolkit.h"

namespace mgp
{

CheckBox::CheckBox() : _checked(false), _image(NULL)
{
    setPadding(0, 0, 0, 0);
    _className = "CheckBox";
}

CheckBox::~CheckBox()
{

}

void CheckBox::onSerialize(Serializer* serializer) {
    Button::onSerialize(serializer);
}

void CheckBox::onDeserialize(Serializer* serializer) {
    Button::onDeserialize(serializer);
    _checked = serializer->readBool("checked", false);
}

bool CheckBox::isChecked()
{
    return _checked;
}

void CheckBox::setChecked(bool checked)
{
    if (_checked != checked)
    {
        _checked = checked;
        setDirty(DIRTY_STATE);
        notifyListeners(Control::Listener::VALUE_CHANGED);
    }
}

void CheckBox::addListener(Control::Listener* listener, int eventFlags)
{
    if ((eventFlags & Control::Listener::TEXT_CHANGED) == Control::Listener::TEXT_CHANGED)
    {
        GP_ERROR("TEXT_CHANGED event is not applicable to CheckBox.");
        eventFlags &= ~Control::Listener::TEXT_CHANGED;
    }

    Control::addListener(listener, eventFlags);
}

bool CheckBox::keyEvent(Keyboard::KeyEvent evt, int key)
{
    if (getState() == ACTIVE && evt == Keyboard::KEY_RELEASE && key == Keyboard::KEY_RETURN)
    {
        setChecked( !_checked );
    }

    return Button::keyEvent(evt, key);
}

void CheckBox::controlEvent(Control::Listener::EventType evt)
{
    Button::controlEvent(evt);

    switch (evt)
    {
    case Control::Listener::CLICK:
        setChecked( !_checked );
        break;
    }
}

void CheckBox::updateState(State state)
{
    Label::updateState(state);

    _image = getTheme()->getImage(_checked ? "checked" : "unchecked");
}

void CheckBox::measureSize()
{
    Label::measureSize();

    Vector2 size;
    const Rectangle& selectedRegion = _image->getRegion();
    size.set(selectedRegion.width, selectedRegion.height);

    if (_autoSizeH == AUTO_WRAP_CONTENT)
    {
        // Text-only width was already measured in Label::update - append image
        //const Border& border = getBorder(NORMAL);
        //const Border& padding = getPadding();
        //setHeightInternal(std::max(_bounds.height, size.y + padding.top + padding.bottom));
    }

    if (_autoSizeW == AUTO_WRAP_CONTENT)
    {
        // Text-only width was already measured in Label::update - append image
        setWidthInternal(_localBounds.height + 5 + _localBounds.width);
    }
}

void CheckBox::updateAbsoluteBounds(const Vector2& offset)
{
    Label::updateAbsoluteBounds(offset);

    _textBounds.x += _localBounds.height + 5;
}

unsigned int CheckBox::drawImages(Form* form, const Rectangle& clip, RenderInfo* view)
{
    if (!_image)
        return 0;

    // Left, v-center.
    // TODO: Set an alignment for icons.

    const Rectangle& region = _image->getRegion();
    //const Vector4& uvs = _image->getUVs();
    Vector4 color = getStyle()->getColor((Style::OverlayType)getState());
    color.w *= _opacity;

    //Vector2 pos(_viewportBounds.x, _viewportBounds.y);

    SpriteBatch* batch = getStyle()->getTheme()->getSpriteBatch();
    startBatch(form, batch);
    batch->drawImage(Rectangle(_viewportBounds.x, _viewportBounds.y, _viewportBounds.height, _viewportBounds.height), 
        region, color, &_viewportClipBounds);
    finishBatch(form, batch, view);

    return 1;
}

}
