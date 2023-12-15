#include "base/Base.h"
#include "RadioButton.h"

namespace mgp
{

RadioButton::RadioButton() : _selected(false), _image(NULL)
{
    setPadding(0, 0, 0, 0);
    _className = "RadioButton";
}

RadioButton::~RadioButton()
{
}

void RadioButton::onSerialize(Serializer* serializer) {
    Button::onSerialize(serializer);
}

void RadioButton::onDeserialize(Serializer* serializer) {
    Button::onDeserialize(serializer);
    bool checked = serializer->readBool("selected", false);
    if (checked) {
        RadioButton::clearSelected(_groupId);
        _selected = true;
    }
    serializer->readString("group", _groupId, "");
}

bool RadioButton::isSelected() const
{
    return _selected;
}

void RadioButton::setSelected(bool selected)
{
    if (selected)
        RadioButton::clearSelected(_groupId);

    if (selected != _selected)
    {
        _selected = selected;
        setDirty(DIRTY_STATE);
        notifyListeners(Control::Listener::VALUE_CHANGED);
    }
}

void RadioButton::addListener(Control::Listener* listener, int eventFlags)
{
    if ((eventFlags & Control::Listener::TEXT_CHANGED) == Control::Listener::TEXT_CHANGED)
    {
        GP_ERROR("TEXT_CHANGED event is not applicable to RadioButton.");
    }

    Control::addListener(listener, eventFlags);
}

void RadioButton::clearSelected(const std::string& groupId)
{
    Container* parent = getParent();
    while (parent) {
        bool found = false;
        for (int i = 0; i < parent->getControlCount(); ++i)
        {
            RadioButton* radioButton = dynamic_cast<RadioButton*>(parent->getControl(i));
            if (radioButton && groupId == radioButton->_groupId)
            {
                radioButton->setSelected(false);
                found = true;
            }
        }
        if (found) break;
        parent = parent->getParent();
    }
}

bool RadioButton::keyEvent(Keyboard::KeyEvent evt, int key)
{
    if (getState() == ACTIVE && evt == Keyboard::KEY_RELEASE && key == Keyboard::KEY_RETURN && !_selected)
    {
        RadioButton::clearSelected(_groupId);
        _selected = true;
        notifyListeners(Control::Listener::VALUE_CHANGED);
    }

    return Button::keyEvent(evt, key);
}

void RadioButton::controlEvent(Control::Listener::EventType evt)
{
    Button::controlEvent(evt);

    switch (evt)
    {
    case Control::Listener::CLICK:
        if (!_selected)
        {
            RadioButton::clearSelected(_groupId);
            _selected = true;
            notifyListeners(Control::Listener::VALUE_CHANGED);
        }
        break;
    }
}

void RadioButton::updateState(State state)
{
    Label::updateState(state);

    _image = getTheme()->getImage(_selected ? "selected" : "unselected");
}

void RadioButton::updateBounds()
{
    Label::updateBounds();

    Vector2 size;
    const Rectangle& selectedRegion = _image->getRegion();
    size.set(selectedRegion.width, selectedRegion.height);

    if (_autoSize & AUTO_SIZE_HEIGHT)
    {
        // Text-only width was already measured in Label::update - append image
        //const Border& border = getBorder(NORMAL);
        //const Border& padding = getPadding();
        //setHeightInternal(std::max(_bounds.height, size.y));
    }

    if (_autoSize & AUTO_SIZE_WIDTH)
    {
        // Text-only width was already measured in Label::update - append image
        setWidthInternal(_localBounds.height + 5 + _localBounds.width);
    }
}

void RadioButton::updateAbsoluteBounds(const Vector2& offset)
{
    Label::updateAbsoluteBounds(offset);

    _textBounds.x += _localBounds.height + 5;
}

unsigned int RadioButton::drawImages(Form* form, const Rectangle& clip, RenderInfo* view)
{
    if (!_image)
        return 0;

    // Left, v-center.
    // TODO: Set an alignment for radio button images.   
    const Rectangle& region = _image->getRegion();
    //const Vector4& uvs = _image->getUVs();
    Vector4 color = getStyle()->getBgColor((Style::OverlayType)getState());
    color.w *= _opacity;

    //Vector2 pos(_viewportBounds.x, _viewportBounds.y);

    SpriteBatch* batch = getStyle()->getTheme()->getSpriteBatch();
    startBatch(form, batch);
    batch->drawImage(Rectangle(_viewportBounds.x, _viewportBounds.y, _viewportBounds.height, _viewportBounds.height),
        region, color, &_viewportClipBounds);
    finishBatch(form, batch, view);

    return 1;
}

void RadioButton::setGroupId(const char* groupId)
{
    _groupId = groupId;
}

const char* RadioButton::getGroupId() const
{
    return _groupId.c_str();
}

}
