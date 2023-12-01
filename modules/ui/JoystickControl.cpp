#include "base/Base.h"
#include "JoystickControl.h"

namespace mgp
{

JoystickControl::JoystickControl() : _radiusPixels(1.0f), _relative(true), _index(0), _radiusCoord(0.5f),
    outer(NULL), inner(NULL)
{
    setBoundsBit(true, _boundsBits, BOUNDS_RADIUS_PERCENTAGE_BIT);
    setCanFocus(true);
    _className = "JoystickControl";
}

JoystickControl::~JoystickControl()
{

}

const Vector2& JoystickControl::getValue() const
{
    return _value;
}

void JoystickControl::setRelative(bool relative)
{
    _relative = relative;
}

bool JoystickControl::isRelative() const
{
    return _relative;
}

unsigned int JoystickControl::getIndex() const
{
    return _index;
}

void JoystickControl::setBoundsBit(bool set, int& bitSetOut, int bit)
{
    if(set)
    {
        bitSetOut |= bit;
    }
    else
    {
        bitSetOut &= ~bit;
    }
}

void JoystickControl::setRadius(float radius, bool isPercentage)
{
    _radiusCoord = radius;
    setBoundsBit(isPercentage, _boundsBits, BOUNDS_RADIUS_PERCENTAGE_BIT);
    updateAbsoluteSizes();
}

float JoystickControl::getRadius() const
{
    return _radiusCoord;
}

bool JoystickControl::isRadiusPercentage() const
{
    return _boundsBits & BOUNDS_RADIUS_PERCENTAGE_BIT;
}

void JoystickControl::onSerialize(Serializer* serializer) {
    Control::onSerialize(serializer);
}

void JoystickControl::onDeserialize(Serializer* serializer) {
    Control::onDeserialize(serializer);
    std::string radiusStr;
    serializer->readString("radius", radiusStr, "");
    if (radiusStr.size() > 0)
    {
        bool isPercentage = false;
        _radiusCoord = parseCoord(radiusStr.c_str(), &isPercentage);
        setBoundsBit(isPercentage, _boundsBits, BOUNDS_RADIUS_PERCENTAGE_BIT);
    }

    bool r = serializer->readBool("relative", false);
    setRelative(r);

    _index = serializer->readInt("index", 0);
}

void JoystickControl::updateBounds()
{

    //if (_autoSize & AUTO_SIZE_WIDTH)
    //{
    //    setWidthInternal(_screenRegionPixels.width);
    //}

    //if (_autoSize & AUTO_SIZE_HEIGHT)
    //{
    //    setHeightInternal(_screenRegionPixels.height);
    //}
    
    Control::updateBounds();
}

void JoystickControl::updateAbsoluteBounds(const Vector2& offset)
{
    Control::updateAbsoluteBounds(offset);
    updateAbsoluteSizes();
}

void JoystickControl::updateAbsoluteSizes()
{
    _radiusPixels = std::max(1.0, _boundsBits & BOUNDS_RADIUS_PERCENTAGE_BIT ?
                std::min(_viewportClipBounds.width, _viewportClipBounds.height) * _radiusCoord : _radiusCoord);
}

void JoystickControl::addListener(Control::Listener* listener, int eventFlags)
{
    if ((eventFlags & Control::Listener::TEXT_CHANGED) == Control::Listener::TEXT_CHANGED)
    {
        GP_ERROR("TEXT_CHANGED event is not applicable to this control.");
    }

    Control::addListener(listener, eventFlags);
}

bool JoystickControl::touchEvent(MotionEvent::MotionType evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
        case MotionEvent::press:
        {
            if (_contactIndex == INVALID_CONTACT_INDEX)
            {
                float dx = 0.0f;
                float dy = 0.0f;

                _contactIndex = (int)contactIndex;

                // Get the displacement of the touch from the centre.
                if (!_relative)
                {
                    dx = x - _viewportClipBounds.width * 0.5f;
                    dy = _viewportClipBounds.height * 0.5f - y;
                }
                else
                {
                    _pressOffset.x = x + _localBounds.x - _viewportClipBounds.width * 0.5f;
                    _pressOffset.y = y + _localBounds.y - _viewportClipBounds.height * 0.5f;
                }

                _displacement.set(dx, dy);

                // If the displacement is greater than the radius, then cap the displacement to the
                // radius.

                Vector2 value;
                if ((fabs(_displacement.x) > _radiusPixels) || (fabs(_displacement.y) > _radiusPixels))
                {
                    _displacement.normalize();
                    value.set(_displacement);
                    _displacement.scale(_radiusPixels);
                }
                else
                {
                    value.set(_displacement);
                    GP_ASSERT(_radiusPixels);
                    value.scale(1.0f / _radiusPixels);
                }

                // Check if the value has changed. Won't this always be the case?
                if (_value != value)
                {
                    _value.set(value);
                    notifyListeners(Control::Listener::VALUE_CHANGED);
                }

                return true;
            }
            break;
        }

        case MotionEvent::touchMove:
        {
            if (_contactIndex == (int) contactIndex)
            {
                float dx = x - ((_relative) ? _pressOffset.x - _localBounds.x : 0.0f) - _viewportClipBounds.width * 0.5f;
                float dy = -(y - ((_relative) ? _pressOffset.y - _localBounds.y : 0.0f) - _viewportClipBounds.height * 0.5f);

                _displacement.set(dx, dy);

                Vector2 value;
                if ((fabs(_displacement.x) > _radiusPixels) || (fabs(_displacement.y) > _radiusPixels))
                {
                    _displacement.normalize();
                    value.set(_displacement);
                    _displacement.scale(_radiusPixels);
                }
                else
                {
                    value.set(_displacement);
                    GP_ASSERT(_radiusPixels);
                    value.scale(1.0f / _radiusPixels);
                }

                if (_value != value)
                {
                    _value.set(value);
                    notifyListeners(Control::Listener::VALUE_CHANGED);
                }

                return true;
            }
            break;
        }

        case MotionEvent::release:
        {
            if (_contactIndex == (int) contactIndex)
            {
                _contactIndex = INVALID_CONTACT_INDEX;

                // Reset displacement and direction vectors.
                _displacement.set(0.0f, 0.0f);
                Vector2 value(_displacement);
                if (_value != value)
                {
                    _value.set(value);
                    notifyListeners(Control::Listener::VALUE_CHANGED);
                }

                return true;
            }
            break;
        }
    }

    return Control::touchEvent(evt, x, y, contactIndex);
}

unsigned int JoystickControl::drawImages(Form* form, const Rectangle& clip, RenderInfo* view)
{
    //const Control::State state = getState();
    unsigned int drawCalls = 0;

    SpriteBatch* batch = getStyle()->getTheme()->getSpriteBatch();
    startBatch(form, batch);

    // Draw the outer image.
    if (true)
    {
        if (!outer) outer = getTheme()->getImage("joystickOuter");
        //const Vector4& uvs = outer->getUVs();
        const Vector4 color = Vector4::one();

        if (_relative)
            batch->drawImage(_viewportClipBounds, outer->getRegion(), color);
        else
            batch->drawImage(_viewportClipBounds, outer->getRegion(), color, &_viewportClipBounds);
        ++drawCalls;
    }

    // Draw the inner image.
    if (true)
    {
        if (!inner) inner = getTheme()->getImage("joystickInner");
        Vector2 position(_viewportClipBounds.x, _viewportClipBounds.y);

        // Adjust position to reflect displacement.
        position.x += _displacement.x;
        position.y += -_displacement.y;

        // Get the uvs and color and draw.
        //const Vector4& uvs = inner->getUVs();
        const Vector4 color = Vector4::one();
        if (_relative)
            batch->drawImage(Rectangle(position.x, position.y, _viewportClipBounds.width, _viewportClipBounds.height), inner->getRegion(), color);
        else
            batch->drawImage(Rectangle(position.x, position.y, _viewportClipBounds.width, _viewportClipBounds.height), inner->getRegion(), color, &_viewportClipBounds);
        ++drawCalls;
    }

    finishBatch(form, batch, view);
    

    return drawCalls;
}

}
