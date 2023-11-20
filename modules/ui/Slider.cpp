#include "Slider.h"
#include "platform/Toolkit.h"

namespace mgp
{

// Fraction of slider to scroll when mouse scrollwheel is used.
static const float SCROLLWHEEL_FRACTION = 0.1f;
// Fraction of slider to scroll for a delta of 1.0f when a gamepad or keyboard is used.
static const float MOVE_FRACTION = 0.005f;

Slider::Slider() : _min(0.0f), _max(1.0f), _step(0.0f), _value(0.0f), _delta(0.0f),
    _trackImage(NULL), _markerImage(NULL), _valueTextVisible(false),
    _valueTextAlignment(FontLayout::ALIGN_BOTTOM_HCENTER), _valueTextPrecision(0), _valueText(""), 
    _trackHeight(0.0f), _gamepadValue(0.0f)
{
    _canFocus = true;
}

Slider::~Slider()
{
}

UPtr<Slider> Slider::create(const char* id, Style* style)
{
    Slider* slider = new Slider();
    slider->_id = id ? id : "";
    slider->initialize("Slider", style, NULL);
    return UPtr<Slider>(slider);
}

Control* Slider::create(Style* style, Properties* properties)
{
    Slider* slider = new Slider();
    slider->initialize("Slider", style, properties);
    return slider;
}

void Slider::initialize(const char* typeName, Style* style, Properties* properties)
{
    Label::initialize(typeName, style, properties);

    if (properties)
    {
        _min = properties->getFloat("min");
        _max = properties->getFloat("max");
        _value = properties->getFloat("value");
        _step = properties->getFloat("step");
        _valueTextVisible = properties->getBool("valueTextVisible");
        _valueTextPrecision = properties->getInt("valueTextPrecision");

        if (properties->exists("valueTextAlignment"))
        {
            _valueTextAlignment = FontLayout::getJustify(properties->getString("valueTextAlignment"));
        }
    }

    // Force value text to be updated
    setValue(_value);
}

const char* Slider::getTypeName() const
{
    return "Slider";
}

void Slider::setMin(float min)
{
    _min = min;
}

float Slider::getMin() const
{
    return _min;
}

void Slider::setMax(float max)
{
    _max = max;
}

float Slider::getMax() const
{
    return _max;
}

void Slider::setStep(float step)
{
    _step = step;
}

float Slider::getStep() const
{
    return _step;
}

float Slider::getValue() const
{
    return _value;
}

void Slider::setValue(float value)
{
    value = MATH_CLAMP(value, _min, _max);

    if (value != _value)
    {
        _value = value;
        notifyListeners(Control::Listener::VALUE_CHANGED);
    }

    // Always update value text if it's visible
    if (_valueTextVisible)
    {
        char s[32];
        sprintf(s, "%.*f", _valueTextPrecision, _value);
        _valueText = s;
    }
}

void Slider::setValueTextVisible(bool valueTextVisible)
{
    if (valueTextVisible != _valueTextVisible)
    {
        _valueTextVisible = valueTextVisible;
        if (_autoSize & AUTO_SIZE_HEIGHT)
            setDirty(DIRTY_BOUNDS);
    }
}

bool Slider::isValueTextVisible() const
{
    return _valueTextVisible;
}

void Slider::setValueTextAlignment(FontLayout::Justify alignment)
{
    _valueTextAlignment = alignment;
}

FontLayout::Justify Slider::getValueTextAlignment() const
{
    return _valueTextAlignment;
}

void Slider::setValueTextPrecision(unsigned int precision)
{
    _valueTextPrecision = precision;
}

unsigned int Slider::getValueTextPrecision() const
{
    return _valueTextPrecision;
}

void Slider::addListener(Control::Listener* listener, int eventFlags)
{
    if ((eventFlags & Control::Listener::TEXT_CHANGED) == Control::Listener::TEXT_CHANGED)
    {
        GP_ERROR("TEXT_CHANGED event is not applicable to Slider.");
    }

    Control::addListener(listener, eventFlags);
}

void Slider::updateValue(int x, int y)
{
    State state = getState();

    // Horizontal case.
    //const Border& border = getBorder(state);
    //const Padding& padding = getPadding();
    //const Rectangle& minCapRegion = _minImage->getRegion();
    //const Rectangle& maxCapRegion = _maxImage->getRegion();
    const Rectangle& markerRegion = _markerImage->getRegion();

    float markerPosition = (x - markerRegion.width * 0.5f) / (_viewportBounds.width - markerRegion.width);
            
    if (markerPosition > 1.0f)
    {
        markerPosition = 1.0f;
    }
    else if (markerPosition < 0.0f)
    {
        markerPosition = 0.0f;
    }

    float value = (markerPosition * (_max - _min)) + _min;
    if (_step > 0.0f)
    {            
        int numSteps = round(value / _step);
        value = _step * numSteps;
    }

    setValue(value);
}

bool Slider::touchEvent(MotionEvent::MotionType evt, int x, int y, unsigned int contactIndex)
{
    State state = getState();

    switch (evt)
    {
    case MotionEvent::press:
        updateValue(x, y);
        return true;

    case MotionEvent::touchMove:
        if (state == ACTIVE)
        {
            updateValue(x, y);
            return true;
        }
        break;
    }

    return Control::touchEvent(evt, x, y, contactIndex);
}

static bool isScrollable(Container* group)
{
    ScrollContainer* container = dynamic_cast<ScrollContainer*>(group);
    if (!container)
        return false;

    if (container->getScroll() != ScrollContainer::SCROLL_NONE)
        return true;

    Container* parent = static_cast<Container*>(container->getParent());
    return parent ? isScrollable(parent) : false;
}

bool Slider::mouseEvent(MotionEvent::MotionType evt, int x, int y, int wheelDelta)
{
    switch (evt)
    {
        case MotionEvent::wheel:
        {
            if (hasFocus() && !isScrollable(_parent))
            {
                float total = _max - _min;
                float value = _value + (total * SCROLLWHEEL_FRACTION) * wheelDelta;

                if (_step > 0.0f)
                {            
                    int numSteps = round(value / _step);
                    value = _step * numSteps;
                }

                setValue(value);
                return true;
            }
            break;
        }

        default:
            break;
    }

    // Return false to fall through to touch handling
    return false;
}

// bool Slider::gamepadJoystickEvent(Gamepad* gamepad, unsigned int index)
// {
//     // The right analog stick can be used to change a slider's value.
//     if (index == 1)
//     {
//         Vector2 joy;
//         gamepad->getJoystickValues(index, &joy);
//         _gamepadValue = _value;
//         _delta = joy.x;
//         return true;
//     }
//     return Label::gamepadJoystickEvent(gamepad, index);
// }

bool Slider::keyEvent(Keyboard::KeyEvent evt, int key)
{
    switch (evt)
    {
    case Keyboard::KEY_PRESS:
        switch (key)
        {
        case Keyboard::KEY_LEFT_ARROW:
            if (_step > 0.0f)
            {
                setValue(std::max(_value - _step, _min));
            }
            else
            {
                setValue(std::max(_value - (_max - _min) * MOVE_FRACTION, _min));
            }
            return true;

        case Keyboard::KEY_RIGHT_ARROW:
            if (_step > 0.0f)
            {
                setValue(std::min(_value + _step, _max));
            }
            else
            {
                setValue(std::min(_value + (_max - _min) * MOVE_FRACTION, _max));
            }
            return true;
        }
        break;
    }

    return Control::keyEvent(evt, key);
}

void Slider::update(float elapsedTime)
{
    Label::update(elapsedTime);

    if (_delta != 0.0f)
    {
        float total = _max - _min;

        if (_step > 0.0f)
        {
            _gamepadValue += (total * MOVE_FRACTION) * _delta;
            int numSteps = round(_gamepadValue / _step);
            setValue(_step * numSteps);
        }
        else
        {
            setValue(_value + (total * MOVE_FRACTION) * _delta);
        }
    }
}

void Slider::updateState(State state)
{
    Label::updateState(state);

    //_minImage = getTheme()->getImage("minCap");
    //_maxImage = getTheme()->getImage("maxCap");
    _markerImage = getTheme()->getImage("marker");
    _trackImage = getTheme()->getImage("track");
}

void Slider::updateBounds()
{
    Label::updateBounds();

    // Compute height of track (max of track, min/max and marker
    _trackHeight = _markerImage->getRegion().height;
    //_trackHeight = std::max(_trackHeight, _maxImage->getRegion().height);
    //_trackHeight = std::max(_trackHeight, _markerImage->getRegion().height);
    _trackHeight = std::max((Float)_trackHeight, _trackImage->getRegion().height);

    if (_autoSize & AUTO_SIZE_HEIGHT)
    {
        float height = _bounds.height + _trackHeight;
        if (_valueTextVisible)
            height += getStyle()->getFontSize();
        setHeightInternal(height);
    }
}

unsigned int Slider::drawImages(Form* form, const Rectangle& clip, RenderInfo* view)
{
    if (!(_markerImage && _trackImage))
        return 0;

    // TODO: Vertical slider.

    // The slider is drawn in the center of the control (perpendicular to orientation).
    // The track is stretched according to orientation.
    // Caps and marker are not stretched.
    const Rectangle& allRegion = _trackImage->getRegion();
    const Rectangle& minCapRegion = Rectangle(allRegion.x, allRegion.y, allRegion.height, allRegion.height);
    const Rectangle& maxCapRegion = Rectangle(allRegion.x + allRegion.width - allRegion.height, allRegion.y, allRegion.height, allRegion.height);
    const Rectangle& markerRegion = _markerImage->getRegion();
    const Rectangle& trackRegion = Rectangle(allRegion.x + allRegion.width/2, allRegion.y, 1, allRegion.height);

    /*const Vector4& minCap = _minImage->getUVs();
    const Vector4& maxCap = _maxImage->getUVs();
    const Vector4& marker = _markerImage->getUVs();
    const Vector4& track = _trackImage->getUVs();*/

    Vector4 minCapColor = Vector4::one();
    Vector4 maxCapColor = Vector4::one();
    Vector4 markerColor = Vector4::one();
    Vector4 trackColor = Vector4::one();

    Control::State state = getState();

    minCapColor.w *= _opacity;
    maxCapColor.w *= _opacity;
    markerColor.w *= _opacity;
    trackColor.w *= _opacity;

    SpriteBatch* batch = getStyle()->getTheme()->getSpriteBatch();
    startBatch(form, batch);

    // Compute area to draw the slider track
    unsigned int fontSize = getStyle()->getFontSize();
    float startY, endY;
    if (_text.length() > 0)
    {
        if (_valueTextVisible)
        {
            // Both label and value text are visible.
            // Draw slider in the middle.
            startY = fontSize;
            endY = _viewportBounds.height - fontSize;
        }
        else
        {
            // Only label is visible
            if (getStyle()->getTextAlignment() & ALIGN_BOTTOM)
            {
                // Draw slider above label
                startY = 0;
                endY = _viewportBounds.height - fontSize;
            }
            else
            {
                // Draw slider below label
                startY = fontSize;
                endY = _viewportBounds.height;
            }
        }
    }
    else if (_valueTextVisible)
    {
        // Only value text is visible.
        if (_valueTextAlignment & ALIGN_BOTTOM)
        {
            // Draw slider above value text
            startY = 0;
            endY = _viewportBounds.height - fontSize;
        }
        else
        {
            // Draw slider below value text
            startY = fontSize;
            endY = _viewportBounds.height;
        }
    }
    else
    {
        // Only the slider track is visible
        startY = 0;
        endY = _viewportBounds.height;
    }

    // Compute midpoint of track location
    float midY = _viewportBounds.y + startY + (endY - startY) * 0.5f;

    // Draw track below the slider text
    Vector2 pos(_viewportBounds.x + minCapRegion.width, midY - trackRegion.height * 0.5f);
    batch->drawImage(Rectangle(pos.x, pos.y, _viewportBounds.width - minCapRegion.width - maxCapRegion.width, trackRegion.height), trackRegion, trackColor, &_viewportClipBounds);

    // Draw min cap to the left of the track
    pos.y = midY - minCapRegion.height * 0.5f;
    pos.x = _viewportBounds.x;
    batch->drawImage(Rectangle(pos.x, pos.y, minCapRegion.width, minCapRegion.height), minCapRegion, minCapColor, &_viewportClipBounds);

    // Draw max cap to the right of the track
    pos.x = _viewportBounds.right() - maxCapRegion.width;
    batch->drawImage(Rectangle(pos.x, pos.y, maxCapRegion.width, maxCapRegion.height), maxCapRegion, maxCapColor, &_viewportClipBounds);

    // Draw the marker at the correct position
    float markerPosition = (_value - _min) / (_max - _min);
    markerPosition *= _viewportBounds.width - markerRegion.width;
    pos.x = _viewportBounds.x + markerPosition;
    pos.y = midY - markerRegion.height * 0.5f;
    batch->drawImage(Rectangle(pos.x, pos.y, markerRegion.width, markerRegion.height), markerRegion, markerColor, &_viewportClipBounds);

    finishBatch(form, batch, view);

    return 4;
}

unsigned int Slider::drawText(Form* form, const Rectangle& clip, RenderInfo* view)
{
    unsigned int drawCalls = Label::drawText(form, clip, view);

    if (_valueTextVisible && _font)
    {
        Control::State state = getState();
        unsigned int fontSize = getStyle()->getFontSize();

        //SpriteBatch* batch = _font->getSpriteBatch(fontSize);
        startBatch(form, _font);
        
        _font->drawText(_valueText.c_str(), _textBounds.x, _textBounds.y, _textColor, fontSize);
       
        finishBatch(form, _font, view);

        ++drawCalls;
    }

    return drawCalls;
}

}
