#include "base/Base.h"
#include "ScrollContainer.h"
#include "Layout.h"
#include "AbsoluteLayout.h"
#include "FlowLayout.h"
#include "VerticalLayout.h"
#include "Label.h"
#include "Button.h"
#include "CheckBox.h"
#include "RadioButton.h"
#include "Slider.h"
#include "TextBox.h"
#include "JoystickControl.h"
//#include "ImageControl.h"
#include "Form.h"
#include "platform/Toolkit.h"
//#include "ControlFactory.h"
#include <algorithm>
#include <float.h>

namespace mgp
{

// If the user stops scrolling for this amount of time (in millis) before touch/click release, don't apply inertia.
static const long SCROLL_INERTIA_DELAY = 100L;
// Factor to multiply friction by before applying to velocity.
static const float SCROLL_FRICTION_FACTOR = 5.0f;
// Distance that must be scrolled before isScrolling() will return true, used e.g. to cancel button-click events.
static const float SCROLL_THRESHOLD = 10.0f;
// Number of milliseconds to fade auto-hide scrollbars out for
static const long SCROLLBAR_FADE_TIME = 1500L;
// If the DPad or joystick is held down, this is the initial delay in milliseconds between focus change events.
static const float FOCUS_CHANGE_REPEAT_DELAY = 300.0f;


ScrollContainer::ScrollContainer()
    : _scrollBarVertical(NULL),
    _scrollBarHorizontal(NULL),
    _scroll(SCROLL_NONE), _scrollBarBounds(Rectangle::empty()), _scrollPosition(Vector2::zero()),
    _scrollBarsAutoHide(true), _scrollBarOpacity(1.0f), _scrolling(false),
    _scrollingVeryFirstX(0), _scrollingVeryFirstY(0), _scrollingFirstX(0), _scrollingFirstY(0), _scrollingLastX(0), _scrollingLastY(0),
    _scrollingStartTimeX(0), _scrollingStartTimeY(0), _scrollingLastTime(0),
    _scrollingVelocity(Vector2::zero()), _scrollingFriction(1.0f), _scrollWheelSpeed(400.0f),
    _scrollingRight(false), _scrollingDown(false),
    _scrollingMouseVertically(false), _scrollingMouseHorizontally(false),
    _scrollBarOpacityClip(NULL),
    _lastFrameTime(0), _totalWidth(0), _totalHeight(0),
    _initializedWithScroll(false), _scrollWheelRequiresFocus(false)
{
    _className = "ScrollContainer";
    _consumeInputEvents = true;
}

ScrollContainer::~ScrollContainer()
{

}

void ScrollContainer::onSerialize(Serializer* serializer) {
    Container::onSerialize(serializer);

    serializer->writeEnum("scroll", "mgp::ScrollContainer::Scroll", _scroll, Scroll::SCROLL_NONE);

    serializer->writeBool("scrollBarsAutoHide", _scrollBarsAutoHide, false);
    serializer->writeBool("scrollWheelRequiresFocus", _scrollWheelRequiresFocus, false);
    serializer->writeFloat("scrollingFriction", _scrollingFriction, 1.0f);
    serializer->writeFloat("scrollWheelSpeed", _scrollWheelSpeed, 400.0f);
}

void ScrollContainer::onDeserialize(Serializer* serializer) {
    Container::onDeserialize(serializer);
    
    Scroll scroll = (Scroll)serializer->readEnum("scroll", "mgp::ScrollContainer::Scroll", Scroll::SCROLL_NONE);
    setScroll(scroll);

    _scrollBarsAutoHide = serializer->readBool("scrollBarsAutoHide", false);
    if (_scrollBarsAutoHide)
    {
        _scrollBarOpacity = 0.0f;
    }
    _scrollWheelRequiresFocus = serializer->readBool("scrollWheelRequiresFocus", false);

    _scrollingFriction = serializer->readFloat("scrollingFriction", 1.0f);
    _scrollWheelSpeed = serializer->readFloat("scrollWheelSpeed", 400.0f);
}

void ScrollContainer::setScroll(Scroll scroll)
{
    if (scroll != _scroll)
    {
        _scroll = scroll;

        if (_scroll == SCROLL_NONE)
        {
            _scrollPosition.set(0, 0);
        }
        else
        {
            // Scrollable containers can be focused (to allow scrolling)
            _canFocus = true;
        }

        setDirty(DIRTY_BOUNDS | DIRTY_STATE);
    }
}

ScrollContainer::Scroll ScrollContainer::getScroll() const
{
    return _scroll;
}

void ScrollContainer::setScrollBarsAutoHide(bool autoHide)
{
    if (autoHide != _scrollBarsAutoHide)
    {
        _scrollBarsAutoHide = autoHide;
        setDirty(DIRTY_BOUNDS | DIRTY_STATE);
    }
}

bool ScrollContainer::isScrollBarsAutoHide() const
{
    return _scrollBarsAutoHide;
}

bool ScrollContainer::isScrolling() const
{
    if (_scrolling &&
        (abs(_scrollingLastX - _scrollingVeryFirstX) > SCROLL_THRESHOLD ||
        abs(_scrollingLastY - _scrollingVeryFirstY) > SCROLL_THRESHOLD))
    {
        return true;
    }

    ScrollContainer* parent = dynamic_cast<ScrollContainer*>(_parent);

    if (parent && parent->isScrolling())
        return true;

    return false;
}

const Vector2& ScrollContainer::getScrollPosition() const
{
    return _scrollPosition;
}

void ScrollContainer::setScrollPosition(const Vector2& scrollPosition)
{
    _scrollPosition = scrollPosition;
    setDirty(DIRTY_BOUNDS);
}

bool ScrollContainer::getScrollWheelRequiresFocus() const
{
    return _scrollWheelRequiresFocus;
}

void ScrollContainer::setScrollWheelRequiresFocus(bool required)
{
    _scrollWheelRequiresFocus = required;
}


void ScrollContainer::updateState(Control::State state)
{
    Container::updateState(state);

    // Get scrollbar images and diminish clipping bounds to make room for scrollbars.
    if ((_scroll & SCROLL_HORIZONTAL) == SCROLL_HORIZONTAL)
    {
        //_scrollBarLeftCap = getTheme()->getImage("scrollBarLeftCap");
        _scrollBarHorizontal = getTheme()->getImage("horizontalScrollBar");
        //_scrollBarRightCap = getTheme()->getImage("scrollBarRightCap");
    }

    if ((_scroll & SCROLL_VERTICAL) == SCROLL_VERTICAL)
    {
        //_scrollBarTopCap = getTheme()->getImage("scrollBarTopCap");
        _scrollBarVertical = getTheme()->getImage("verticalScrollBar");
        //_scrollBarBottomCap = getTheme()->getImage("scrollBarBottomCap");
    }
}

void ScrollContainer::updateAbsoluteBounds(const Vector2& offset)
{
   Control::updateAbsoluteBounds(offset);

   // Get scrollbar images and diminish clipping bounds to make room for scrollbars.
   if ((_scroll & SCROLL_HORIZONTAL) == SCROLL_HORIZONTAL)
   {
       GP_ASSERT(_scrollBarHorizontal);
       _viewportBounds.height -= _scrollBarHorizontal->getRegion().height;
       _viewportClipBounds.height -= _scrollBarHorizontal->getRegion().height;
   }

   if ((_scroll & SCROLL_VERTICAL) == SCROLL_VERTICAL)
   {
       GP_ASSERT(_scrollBarVertical);
       _viewportBounds.width -= _scrollBarVertical->getRegion().width;
       _viewportClipBounds.width -= _scrollBarVertical->getRegion().width;
   }
}

void ScrollContainer::layoutChildren(bool dirtyBounds)
{
    if (dirtyBounds) {
        updateChildBounds();

        // Update scroll position and scrollbars after updating absolute bounds since
        // computation relies on up-to-date absolute bounds information.
        updateScroll();
    }

    for (size_t i = 0, count = _controls.size(); i < count; ++i)
    {
        Control* ctrl = _controls[i];
        GP_ASSERT(ctrl);

        if (ctrl->isVisible())
        {
            ctrl->updateLayout(_scrollPosition);
        }
    }
}

void ScrollContainer::getBarPadding(int* vertical, int* horizontal) {
    if (_scrollBarVertical) {
        *vertical = _scrollBarVertical->getRegion().width;
    }
    else {
        *vertical = 0;
    }

    if (_scrollBarHorizontal) {
        *horizontal = _scrollBarHorizontal->getRegion().height;
    }
    else {
        *horizontal = 0;
    }
}

unsigned int ScrollContainer::draw(Form* form, const Rectangle& clip, RenderInfo* view)
{
    if (!_visible)
        return 0;

    // Draw container skin
    unsigned int drawCalls = Container::draw(form, clip, view);

    // Draw scrollbars
    if (_scroll != SCROLL_NONE && (_scrollBarOpacity > 0.0f))
    {
        // Draw scroll bars.
        Rectangle clipRegion(_absoluteClipBounds);

        SpriteBatch* batch = getStyle()->getTheme()->getSpriteBatch();
        startBatch(form, batch);

        if (_scrollBarBounds.height > 0 && ((_scroll & SCROLL_VERTICAL) == SCROLL_VERTICAL))
        {
            const Rectangle& barRegion = _scrollBarVertical->getRegion();

            const Rectangle topRegion(barRegion.x, barRegion.y, barRegion.width, barRegion.width);
            //const Vector4& topUVs = _scrollBarTopCap->getUVs();
            Vector4 topColor = Vector4::one();
            topColor.w *= _scrollBarOpacity * _opacity;

            const Rectangle verticalRegion(barRegion.x, barRegion.y + barRegion.width, barRegion.width, barRegion.height - barRegion.width - barRegion.width);
            //const Vector4& verticalUVs = _scrollBarVertical->getUVs();
            Vector4 verticalColor = Vector4::one();
            verticalColor.w *= _scrollBarOpacity * _opacity;

            const Rectangle bottomRegion(barRegion.x, barRegion.y + barRegion.height - barRegion.width, barRegion.width, barRegion.width);
            //const Vector4& bottomUVs = _scrollBarBottomCap->getUVs();
            Vector4 bottomColor = Vector4::one();
            bottomColor.w *= _scrollBarOpacity * _opacity;

            clipRegion.width += verticalRegion.width;

            float middleHeight = _scrollBarBounds.height - topRegion.height - bottomRegion.height;
            if (middleHeight > 0) {
                //top
                Rectangle bounds(_viewportBounds.right() + (_absoluteBounds.right() - _viewportBounds.right()) * 0.5f - topRegion.width * 0.5f, _viewportBounds.y + _scrollBarBounds.y, topRegion.width, topRegion.height);
                batch->drawImage(bounds, topRegion, topColor, &clipRegion);

                //bottom
                bounds.y += topRegion.height + middleHeight;
                bounds.height = bottomRegion.height;
                batch->drawImage(bounds, bottomRegion, bottomColor, &clipRegion);

                //middle
                bounds.y -= middleHeight;
                bounds.height = middleHeight;
                batch->drawImage(bounds, verticalRegion, verticalColor, &clipRegion);

                drawCalls += 3;
            }
            else {
                Rectangle bounds(_viewportBounds.right() + (_absoluteBounds.right() - _viewportBounds.right()) * 0.5f - topRegion.width * 0.5f, _viewportBounds.y + _scrollBarBounds.y, topRegion.width, topRegion.height);
                bounds.height = _scrollBarBounds.height;
                batch->drawImage(bounds, verticalRegion, verticalColor, &clipRegion);
            }
        }

        if (_scrollBarBounds.width > 0 && ((_scroll & SCROLL_HORIZONTAL) == SCROLL_HORIZONTAL))
        {
            const Rectangle& barRegion = _scrollBarHorizontal->getRegion();

            const Rectangle leftRegion(barRegion.x, barRegion.y, barRegion.height, barRegion.height);
            //const Vector4& leftUVs = _scrollBarLeftCap->getUVs();
            Vector4 leftColor = Vector4::one();
            leftColor.w *= _scrollBarOpacity * _opacity;

            const Rectangle horizontalRegion(barRegion.x + barRegion.height, barRegion.y, barRegion.width - barRegion.height - barRegion.height, barRegion.height);
            //const Vector4& horizontalUVs = _scrollBarHorizontal->getUVs();
            Vector4 horizontalColor = Vector4::one();
            horizontalColor.w *= _scrollBarOpacity * _opacity;

            const Rectangle rightRegion(barRegion.x + barRegion.width - barRegion.height, barRegion.y, barRegion.height, barRegion.height);
            //const Vector4& rightUVs = _scrollBarRightCap->getUVs();
            Vector4 rightColor = Vector4::one();
            rightColor.w *= _scrollBarOpacity * _opacity;

            clipRegion.height += horizontalRegion.height;

            Rectangle bounds(_viewportBounds.x + _scrollBarBounds.x, _viewportBounds.bottom() + (_absoluteBounds.bottom() - _viewportBounds.bottom()) * 0.5f - leftRegion.height * 0.5f, leftRegion.width, leftRegion.height);
            batch->drawImage(bounds, leftRegion, leftColor, &clipRegion);

            bounds.x += leftRegion.width;
            bounds.width = _scrollBarBounds.width - leftRegion.width - rightRegion.width;
            batch->drawImage(bounds, horizontalRegion, horizontalColor, &clipRegion);

            bounds.x += bounds.width;
            bounds.width = rightRegion.width;
            batch->drawImage(bounds, rightRegion, rightColor, &clipRegion);

            drawCalls += 3;
        }

        finishBatch(form, batch, view);
    }

    return drawCalls;
}



void ScrollContainer::startScrolling(float x, float y, bool resetTime)
{
    _scrollingVelocity.set(-x, y);
    _scrolling = true;
    _scrollBarOpacity = 1.0f;
    setDirty(DIRTY_BOUNDS);

    if (_scrollBarOpacityClip && _scrollBarOpacityClip->isPlaying())
    {
        _scrollBarOpacityClip->stop();
        _scrollBarOpacityClip = NULL;
    }

    if (resetTime)
    {
        _lastFrameTime = System::millisTicks();
    }
}

void ScrollContainer::stopScrolling()
{
    _scrollingVelocity.set(0, 0);
    _scrolling = false;
    setDirty(DIRTY_BOUNDS);

    ScrollContainer* parent = dynamic_cast<ScrollContainer*>(_parent);
    if (parent)
        parent->stopScrolling();
}

void ScrollContainer::updateScroll()
{
    if (_scroll == SCROLL_NONE)
        return;

    Control::State state = getState();

    // Update time.
    if (!_lastFrameTime)
    {
        _lastFrameTime = System::millisTicks();
    }
    double frameTime = System::millisTicks();
    float elapsedTime = (float)(frameTime - _lastFrameTime);
    _lastFrameTime = frameTime;

    //const Border& containerBorder = getBorder(state);
    const Padding& containerPadding = getPadding();

    // Calculate total width and height.
    _totalWidth = _totalHeight = 0.0f;
    std::vector<Control*> controls = getControls();
    for (size_t i = 0, count = controls.size(); i < count; ++i)
    {
        Control* control = _controls[i];

        if (!control->isVisible())
            continue;

        const Rectangle& bounds = control->getBounds();
        const Margin& margin = control->getMargin();

        float newWidth = bounds.x + bounds.width + margin.right;
        if (newWidth > _totalWidth)
        {
            _totalWidth = newWidth;
        }

        float newHeight = bounds.y + bounds.height + margin.bottom;
        if (newHeight > _totalHeight)
        {
            _totalHeight = newHeight;
        }
    }

    float vWidth = getTheme()->getImage("verticalScrollBar")->getRegion().width;
    float hHeight = getTheme()->getImage("horizontalScrollBar")->getRegion().height;
    float clipWidth = _absoluteBounds.width - containerPadding.left - containerPadding.right;
    if ((_scroll & SCROLL_VERTICAL) == SCROLL_VERTICAL) {
        clipWidth -= vWidth;
    }
    
    float clipHeight = _absoluteBounds.height - containerPadding.top - containerPadding.bottom;
    if ((_scroll & SCROLL_HORIZONTAL) == SCROLL_HORIZONTAL) {
        clipHeight -= hHeight;
    }

    bool dirty = false;

    // Apply and dampen inertia.
    if (!_scrollingVelocity.isZero())
    {
        // Calculate the time passed since last update.
        float elapsedSecs = (float)elapsedTime * 0.001f;

        _scrollPosition.x += _scrollingVelocity.x * elapsedSecs;
        _scrollPosition.y += _scrollingVelocity.y * elapsedSecs;

        if (!_scrolling)
        {
            float dampening = 1.0f - _scrollingFriction * SCROLL_FRICTION_FACTOR * elapsedSecs;
            _scrollingVelocity.x *= dampening;
            _scrollingVelocity.y *= dampening;

            if (fabs(_scrollingVelocity.x) < 100.0f)
                _scrollingVelocity.x = 0.0f;
            if (fabs(_scrollingVelocity.y) < 100.0f)
                _scrollingVelocity.y = 0.0f;
        }

        dirty = true;
    }

    // Stop scrolling when the far edge is reached.
    Vector2 lastScrollPosition(_scrollPosition);

    if (-_scrollPosition.x > _totalWidth - clipWidth)
    {
        _scrollPosition.x = -(_totalWidth - clipWidth);
        _scrollingVelocity.x = 0;
    }
    
    if (-_scrollPosition.y > _totalHeight - clipHeight)
    {
        _scrollPosition.y = -(_totalHeight - clipHeight);
        _scrollingVelocity.y = 0;
    }

    if (_scrollPosition.x > 0)
    {
        _scrollPosition.x = 0;
        _scrollingVelocity.x = 0;
    }

    if (_scrollPosition.y > 0)
    {
        _scrollPosition.y = 0;
        _scrollingVelocity.y = 0;
    }

    if (_scrollPosition != lastScrollPosition)
        dirty = true;

    float scrollWidth = 0;
    if (clipWidth < _totalWidth)
        scrollWidth = (clipWidth / _totalWidth) * clipWidth;

    float scrollHeight = 0;
    if (clipHeight < _totalHeight)
        scrollHeight = (clipHeight / _totalHeight) * clipHeight;

    _scrollBarBounds.set(((-_scrollPosition.x) / _totalWidth) * clipWidth,
                         ((-_scrollPosition.y) / _totalHeight) * clipHeight,
                         scrollWidth, scrollHeight);

    // If scroll velocity is 0 and scrollbars are not always visible, trigger fade-out animation.
    if (!_scrolling && _scrollingVelocity.isZero() && _scrollBarsAutoHide && _scrollBarOpacity == 1.0f)
    {
        float to = 0.2;
        _scrollBarOpacity = 0.99f;
        if (!_scrollBarOpacityClip)
        {
            Animation* animation = createAnimationFromTo("scrollbar-fade-out", ANIMATE_SCROLLBAR_OPACITY, &_scrollBarOpacity, &to, Curve::QUADRATIC_IN_OUT, SCROLLBAR_FADE_TIME);
            _scrollBarOpacityClip = animation->getClip();
        }
        _scrollBarOpacityClip->play();
    }

    // When scroll position is updated, we need to recompute bounds since children
    // absolute bounds offset will need to be updated.
    if (dirty)
    {
        setDirty(DIRTY_BOUNDS);
    }
}


bool ScrollContainer::touchEventScroll(MotionEvent::MotionType evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
    case MotionEvent::press:
        if (_contactIndex == INVALID_CONTACT_INDEX)
        {
            bool dirty = !_scrollingVelocity.isZero();
            _contactIndex = (int)contactIndex;
            _scrollingLastX = _scrollingFirstX = _scrollingVeryFirstX = x;
            _scrollingLastY = _scrollingFirstY = _scrollingVeryFirstY = y;
            _scrollingVelocity.set(0, 0);
            _scrolling = true;
            _scrollingStartTimeX = _scrollingStartTimeY = 0;

            if (_scrollBarOpacityClip && _scrollBarOpacityClip->isPlaying())
            {
                _scrollBarOpacityClip->stop();
                _scrollBarOpacityClip = NULL;
            }
            _scrollBarOpacity = 1.0f;
            if (dirty)
                setDirty(DIRTY_BOUNDS);
            return false;
        }
        break;

    case MotionEvent::touchMove:
        if (_scrolling && _contactIndex == (int)contactIndex)
        {
            double gameTime = System::millisTicks();

            // Calculate the latest movement delta for the next update to use.
            int vx = x - _scrollingLastX;
            int vy = y - _scrollingLastY;
            if (_scrollingMouseVertically)
            {
                float yRatio = _totalHeight / _absoluteBounds.height;
                vy *= yRatio;

                _scrollingVelocity.set(0, -vy);
                _scrollPosition.y -= vy;
            }
            else if (_scrollingMouseHorizontally)
            {
                float xRatio = _totalWidth / _absoluteBounds.width;
                vx *= xRatio;

                _scrollingVelocity.set(-vx, 0);
                _scrollPosition.x -= vx;
            }
            else
            {
                _scrollingVelocity.set(vx, vy);
                _scrollPosition.x += vx;
                _scrollPosition.y += vy;
            }

            _scrollingLastX = x;
            _scrollingLastY = y;

            // If the user changes direction, reset the start time and position.
            bool goingRight = (vx > 0);
            if (goingRight != _scrollingRight)
            {
                _scrollingFirstX = x;
                _scrollingRight = goingRight;
                _scrollingStartTimeX = gameTime;
            }

            bool goingDown = (vy > 0);
            if (goingDown != _scrollingDown)
            {
                _scrollingFirstY = y;
                _scrollingDown = goingDown;
                _scrollingStartTimeY = gameTime;
            }

            if (!_scrollingStartTimeX)
                _scrollingStartTimeX = gameTime;

            if (!_scrollingStartTimeY)
                _scrollingStartTimeY = gameTime;

            _scrollingLastTime = gameTime;
            setDirty(DIRTY_BOUNDS);
            updateScroll();
            return false;
        }
        break;

    case MotionEvent::release:
        if (_contactIndex == (int) contactIndex)
        {
            _contactIndex = INVALID_CONTACT_INDEX;
            _scrolling = false;
            double gameTime = System::millisTicks();
            float timeSinceLastMove = (float)(gameTime - _scrollingLastTime);
            if (timeSinceLastMove > SCROLL_INERTIA_DELAY)
            {
                _scrollingVelocity.set(0, 0);
                _scrollingMouseVertically = _scrollingMouseHorizontally = false;
                return false;
            }

            int dx = _scrollingLastX - _scrollingFirstX;
            int dy = _scrollingLastY - _scrollingFirstY;

            float timeTakenX = (float)(gameTime - _scrollingStartTimeX);
            float elapsedSecsX = timeTakenX * 0.001f;
            float timeTakenY = (float)(gameTime - _scrollingStartTimeY);
            float elapsedSecsY = timeTakenY * 0.001f;

            float vx = dx;
            float vy = dy;
            if (elapsedSecsX > 0)
                vx = (float)dx / elapsedSecsX;
            if (elapsedSecsY > 0)
                vy = (float)dy / elapsedSecsY;

            if (_scrollingMouseVertically)
            {
                float yRatio = _totalHeight / _absoluteBounds.height;
                vy *= yRatio;
                _scrollingVelocity.set(0, -vy);
            }
            else if (_scrollingMouseHorizontally)
            {
                float xRatio = _totalWidth / _absoluteBounds.width;
                vx *= xRatio;
                _scrollingVelocity.set(-vx, 0);
            }
            else
            {
                _scrollingVelocity.set(vx, vy);
            }

            _scrollingMouseVertically = _scrollingMouseHorizontally = false;
            setDirty(DIRTY_BOUNDS);
            return false;
        }
        break;
    }

    return false;
}

bool ScrollContainer::mouseEventScroll(MotionEvent::MotionType evt, int x, int y, int wheelDelta)
{
    switch (evt)
    {
        case MotionEvent::press:
        {
            bool dirty = false;
            if (_scrollBarVertical)
            {
                float vWidth = _scrollBarVertical->getRegion().width;
                float rightPadding = _absoluteBounds.right() - _viewportBounds.right();
                float topPadding = _viewportBounds.y - _absoluteBounds.y;
                float localVpRight = _localBounds.width - rightPadding;
                Rectangle vBounds(
                    localVpRight + rightPadding*0.5f - vWidth*0.5f,
                    topPadding + _scrollBarBounds.y,
                    vWidth, _scrollBarBounds.height);

                if (x >= vBounds.x && x <= vBounds.right())
                {
                    // Then we're within the horizontal bounds of the vertical scrollbar.
                    // We want to either jump up or down, or drag the scrollbar itself.
                    if (y < vBounds.y)
                    {
                        _scrollPosition.y += _totalHeight / 5.0f;
                        dirty = true;
                    }
                    else if (y > vBounds.bottom())
                    {
                        _scrollPosition.y -= _totalHeight / 5.0f;
                        dirty = true;
                    }
                    else
                    {
                        _scrollingMouseVertically = true;
                    }
                }
            }

            if (_scrollBarHorizontal)
            {
                float hHeight = _scrollBarHorizontal->getRegion().height;
                float bottomPadding = _absoluteBounds.bottom() - _viewportBounds.bottom();
                float leftPadding = _viewportBounds.x - _absoluteBounds.x;
                float localVpBottom = _localBounds.height - bottomPadding;
                Rectangle hBounds(
                    leftPadding + _scrollBarBounds.x,
                    localVpBottom + bottomPadding*0.5f - hHeight*0.5f,
                    _scrollBarBounds.width, hHeight);

                if (y >= hBounds.y && y <= hBounds.bottom())
                {
                    // We're within the vertical bounds of the horizontal scrollbar.
                    if (x < hBounds.x)
                    {
                        _scrollPosition.x += _totalWidth / 5.0f;
                        dirty = true;
                    }
                    else if (x > hBounds.x + hBounds.width)
                    {
                        _scrollPosition.x -= _totalWidth / 5.0f;
                        dirty = true;
                    }
                    else
                    {
                        _scrollingMouseHorizontally = true;
                    }
                }
            }

            if (dirty)
            {
                setDirty(DIRTY_BOUNDS);
            }

            return touchEventScroll(MotionEvent::press, x, y, 0);
        }

        case MotionEvent::touchMove:
            return touchEventScroll(MotionEvent::touchMove, x, y, 0);

        case MotionEvent::release:
            return touchEventScroll(MotionEvent::release, x, y, 0);

        case MotionEvent::wheel:
        {
            if (_scrollingVelocity.isZero())
            {
                _lastFrameTime = System::millisTicks();
            }
            _scrolling = _scrollingMouseVertically = _scrollingMouseHorizontally = false;

            _scrollingVelocity.y += _scrollWheelSpeed * wheelDelta;

            if (_scrollBarOpacityClip && _scrollBarOpacityClip->isPlaying())
            {
                _scrollBarOpacityClip->stop();
                _scrollBarOpacityClip = NULL;
            }
            _scrollBarOpacity = 1.0f;
            setDirty(DIRTY_BOUNDS);
            return false;
        }
    }

    return false;
}


//ScrollContainer::Scroll ScrollContainer::getScroll(const char* scroll)
//{
//    if (!scroll)
//        return ScrollContainer::SCROLL_NONE;
//
//    if (strcmp(scroll, "SCROLL_NONE") == 0)
//    {
//        return ScrollContainer::SCROLL_NONE;
//    }
//    else if (strcmp(scroll, "SCROLL_HORIZONTAL") == 0)
//    {
//        return ScrollContainer::SCROLL_HORIZONTAL;
//    }
//    else if (strcmp(scroll, "SCROLL_VERTICAL") == 0)
//    {
//        return ScrollContainer::SCROLL_VERTICAL;
//    }
//    else if (strcmp(scroll, "SCROLL_BOTH") == 0)
//    {
//        return ScrollContainer::SCROLL_BOTH;
//    }
//    else
//    {
//        GP_ERROR("Failed to get corresponding scroll state for unsupported value '%s'.", scroll);
//    }
//
//    return ScrollContainer::SCROLL_NONE;
//}

std::string ScrollContainer::enumToString(const std::string& enumName, int value)
{
    if (enumName.compare("mgp::ScrollContainer::Scroll") == 0)
    {
        switch (value)
        {
        case static_cast<int>(SCROLL_NONE):
            return "None";
        case static_cast<int>(SCROLL_HORIZONTAL):
            return "Horizontal";
        case static_cast<int>(SCROLL_VERTICAL):
            return "Vertical";
        case static_cast<int>(SCROLL_BOTH):
            return "Both";
        default:
            return "None";
        }
    }
    return "";
}

int ScrollContainer::enumParse(const std::string& enumName, const std::string& str)
{
    if (enumName.compare("mgp::ScrollContainer::Scroll") == 0)
    {
        if (str.compare("None") == 0)
            return static_cast<int>(SCROLL_NONE);
        else if (str.compare("Horizontal") == 0)
            return static_cast<int>(SCROLL_HORIZONTAL);
        else if (str.compare("Vertical") == 0)
            return static_cast<int>(SCROLL_VERTICAL);
        else if (str.compare("Both") == 0)
            return static_cast<int>(SCROLL_BOTH);
    }
    return 0;
}

float ScrollContainer::getScrollingFriction() const
{
    return _scrollingFriction;
}

void ScrollContainer::setScrollingFriction(float friction)
{
    _scrollingFriction = friction;
}

float ScrollContainer::getScrollWheelSpeed() const
{
    return _scrollWheelSpeed;
}

void ScrollContainer::setScrollWheelSpeed(float speed)
{
    _scrollWheelSpeed = speed;
}

unsigned int ScrollContainer::getAnimationPropertyComponentCount(int propertyId) const
{
    switch(propertyId)
    {
    case ANIMATE_SCROLLBAR_OPACITY:
        return 1;
    default:
        return Control::getAnimationPropertyComponentCount(propertyId);
    }
}

void ScrollContainer::getAnimationPropertyValue(int propertyId, AnimationValue* value)
{
    GP_ASSERT(value);

    switch(propertyId)
    {
    case ANIMATE_SCROLLBAR_OPACITY:
        value->setFloat(0, _scrollBarOpacity);
        break;
    default:
        Control::getAnimationPropertyValue(propertyId, value);
        break;
    }
}

void ScrollContainer::setAnimationPropertyValue(int propertyId, AnimationValue* value, float blendWeight)
{
    GP_ASSERT(value);

    switch(propertyId)
    {
    case ANIMATE_SCROLLBAR_OPACITY:
        _scrollBarOpacity = Curve::lerp(blendWeight, _opacity, value->getFloat(0));
        break;
    default:
        Control::setAnimationPropertyValue(propertyId, value, blendWeight);
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////

Accordion::Accordion()
{
    this->setLayout(Layout::LAYOUT_VERTICAL);
    auto button = Control::create<Button>("accordinButton", NULL, "AccordionButton");
    button->setText("Accordion");
    button->setPadding(10, 10, 10, 22);
    button->setMargin(1, 0, 0, 0);
    _button = button.get();
    button->setWidth(1.0, Control::AUTO_PERCENT_PARENT);
    button->setHeight(1.0, Control::AUTO_WRAP_CONTENT);
    this->addControl(std::move(button));

    auto content = Control::create<ScrollContainer>("accordinContent");
    _content = uniqueFromInstant(content.get());
    content->setWidth(1.0, Control::AUTO_PERCENT_PARENT);
    content->setHeight(1.0, Control::AUTO_PERCENT_LEFT);
    this->addControl(std::move(content));

    this->setHeight(1.0, Control::AUTO_PERCENT_LEFT);
    this->setWidth(1.0, Control::AUTO_PERCENT_PARENT);

    _button->setListener([=](Control* control, Control::Listener::EventType evt) {
        if (evt == Listener::CLICK) {
            //_content->setVisible(_content->isVisible());
            
            //if (_content->getParent()) {
            //    _content->getParent()->removeControl(_content.get());
            //    this->setHeight(1.0, Control::AUTO_WRAP_CONTENT);
            //    _expanded = false;
            //}
            //else {
            //    this->addControl(uniqueFromInstant(_content.get()));
            //    this->setHeight(1.0, Control::AUTO_PERCENT_LEFT);
            //    _expanded = true;
            //}

            setExpand(!_expanded);

            if (onClik) {
                onClik(_expanded);
            }

            if (this->getParent()) {
                this->getParent()->setDirty(DIRTY_BOUNDS, true);
            }
        }
    });
}

void Accordion::setExpand(bool expand) {
    if (_expanded != expand) {
        if (expand) {
            if (!_content->getParent()) {
                this->addControl(uniqueFromInstant(_content.get()));
            }
            this->setHeight(1.0, Control::AUTO_PERCENT_LEFT);
        }
        else {
            _content->getParent()->removeControl(_content.get());
            this->setHeight(1.0, Control::AUTO_WRAP_CONTENT);
        }
        _expanded = expand;
    }
}

void Accordion::setContent(UPtr<Control> c) {
    if (_content->getParent()) {
        _content->getParent()->removeControl(_content.get());
    }
    _content = std::move(c);
    if (_expanded) {
        this->addControl(uniqueFromInstant(_content.get()));
    }
}

///////////////////////////////////////////////////////////////////////////////////

ListView::ListView()
{
    this->setStyleName("Panel");
    this->overrideStyle()->setColor(Vector4::fromColor(0x565656ff));
    this->setHeight(1.0, Control::AUTO_PERCENT_LEFT);
    this->setWidth(1.0, Control::AUTO_PERCENT_LEFT);
    this->setPadding(4);
    this->setLayout(Layout::LAYOUT_VERTICAL);
    this->setScroll(SCROLL_VERTICAL);
}

void ListView::setSelection(int i)
{
    if (i == _curSelection) {
        return;
    }

    if (_curSelection != -1) {
        Control* item = this->getControl(_curSelection);
        item->setStyleName(item->getClassName().c_str());
    }
    if (i >= items.size()) {
        _curSelection = -1;
    }
    _curSelection = i;
    Control* item = this->getControl(_curSelection);
    item->setStyleName("Rect");

    notifyListeners(Listener::SELECT_CHANGE);
}

void ListView::setItems(std::vector<std::string>& list)
{
    _curSelection = -1;
    this->clear();
    items = list;
    for (int i = 0; i < items.size(); ++i) {
        auto item = createRow(i);
        item->_userData = i;
        item->setCanFocus(true);
        item->setConsumeInputEvents(true);

        if (Container* pane = dynamic_cast<Container*>(item.get())) {
            for (int i2 = 0; i2 < pane->getControlCount(); ++i2) {
                Control* label = pane->getControl(i2);
                label->_userData = i;
                label->addListener(this, Listener::CLICK);
            }
        }
        item->addListener(this, Listener::CLICK);
        this->addControl(std::move(item));
    }
}

UPtr<Control> ListView::createRow(int i)
{
    UPtr<Container> pane = Control::create<Container>("input_row");
    pane->setWidth(1, Control::AUTO_PERCENT_PARENT);
    pane->setMargin(0, 10, 0, 8);
    pane->setLayout(Layout::LAYOUT_HORIZONTAL);
    //pane->setStyleName("Rect");

    auto label = Control::create<Label>("row_label"); {
        label->setWidth(1, Control::AUTO_PERCENT_PARENT);
        label->setAutoSizeH(Control::AUTO_WRAP_CONTENT);
        label->setPadding(4);
        label->setText(items.at(i).c_str());
        pane->addControl(std::move(label));
    }
    return pane;
}

void ListView::controlEvent(Control* control, EventType evt)
{
    if (evt == Listener::CLICK) {
        int i = control->_userData;
        setSelection(i);
    }
}

}
