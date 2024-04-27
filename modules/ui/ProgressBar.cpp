#include "ProgressBar.h"
#include "platform/Toolkit.h"
#include "ScrollContainer.h"

namespace mgp
{

ProgressBar::ProgressBar() : _value(0.0f),  _trackImage(NULL),  _trackHeight(0.0f)
{
    _canFocus = false;
    _className = "ProgressBar";
}

ProgressBar::~ProgressBar()
{
}

void ProgressBar::onSerialize(Serializer* serializer) {
    Control::onSerialize(serializer);

    serializer->writeFloat("value", _value, 0.0f);
}

void ProgressBar::onDeserialize(Serializer* serializer) {
    Control::onDeserialize(serializer);

    _value = serializer->readFloat("value", 0);
    // Force value text to be updated
    setValue(_value);
}

float ProgressBar::getValue() const
{
    return _value;
}

void ProgressBar::setValue(float value, bool fireEvent)
{
    value = MATH_CLAMP(value, 0, 1);

    if (value != _value)
    {
        _value = value;
        if (fireEvent) notifyListeners(Control::Listener::VALUE_CHANGED);
    }
}

void ProgressBar::updateState(State state)
{
    Control::updateState(state);

    _trackImage = getTheme()->getImage("track");
}

void ProgressBar::measureSize()
{
    Control::measureSize();

    _trackHeight = _trackImage->getRegion().height;

    if (_autoSizeH == AUTO_WRAP_CONTENT)
    {
        setMeasureContentHeight(_trackHeight);
    }
}

unsigned int ProgressBar::drawImages(Form* form, const Rectangle& clip, RenderInfo* view)
{
    if (!(_trackImage))
        return 0;

    // TODO: Vertical slider.

    // The slider is drawn in the center of the control (perpendicular to orientation).
    // The track is stretched according to orientation.
    // Caps and marker are not stretched.
    const Rectangle& allRegion = _trackImage->getRegion();
    const Rectangle& minCapRegion = Rectangle(allRegion.x, allRegion.y, allRegion.height, allRegion.height);
    const Rectangle& maxCapRegion = Rectangle(allRegion.x + allRegion.width - allRegion.height, allRegion.y, allRegion.height, allRegion.height);
    const Rectangle& trackRegion = Rectangle(allRegion.x + allRegion.width/2, allRegion.y, 1, allRegion.height);

    //Vector4 minCapColor = Vector4::one();
    //Vector4 maxCapColor = Vector4::one();
    Vector4 trackColor = Vector4::one();
    Vector4 trackedColor = Vector4(0.0, 0.7, 1.0, 1.0);

    Control::State state = getState();

    //minCapColor.w *= _opacity;
    //maxCapColor.w *= _opacity;
    //markerColor.w *= _opacity;
    trackColor.w *= _opacity;
    trackedColor.w *= _opacity;

    SpriteBatch* batch = getStyle()->getTheme()->getSpriteBatch();
    startBatch(form, batch);

    // Compute area to draw the slider track
    unsigned int fontSize = getStyle()->getFontSize();
    float startY, endY;
    // Only the slider track is visible
    startY = 0;
    endY = _viewportBounds.height;
    

    // Compute midpoint of track location
    float midY = _viewportBounds.y + startY + (endY - startY) * 0.5f;

    // Draw track below the slider text
    Vector2 pos(_viewportBounds.x + minCapRegion.width, midY - trackRegion.height * 0.5f);
    batch->drawImage(Rectangle(pos.x, pos.y, _viewportBounds.width - minCapRegion.width - maxCapRegion.width, trackRegion.height),
        trackRegion, trackedColor, &_viewportClipBounds);

    float valueLength = _value * _absoluteBounds.width;
    pos.x += valueLength;
    batch->drawImage(Rectangle(pos.x, pos.y, _viewportBounds.width - minCapRegion.width - maxCapRegion.width - valueLength, trackRegion.height),
        trackRegion, trackColor, &_viewportClipBounds);

    // Draw min cap to the left of the track
    pos.y = midY - minCapRegion.height * 0.5f;
    pos.x = _viewportBounds.x;
    batch->drawImage(Rectangle(pos.x, pos.y, minCapRegion.width, minCapRegion.height), minCapRegion, trackedColor, &_viewportClipBounds);

    // Draw max cap to the right of the track
    pos.x = _viewportBounds.right() - maxCapRegion.width;
    batch->drawImage(Rectangle(pos.x, pos.y, maxCapRegion.width, maxCapRegion.height), maxCapRegion, trackColor, &_viewportClipBounds);


    finishBatch(form, batch, view);

    return 4;
}

}
