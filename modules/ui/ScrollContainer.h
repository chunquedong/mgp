#ifndef SCROLL_CONTAINER_H_
#define SCROLL_CONTAINER_H_

#include "Container.h"
#include "Layout.h"
#include "platform/Toolkit.h"

namespace mgp
{
/**
 * scrollable container
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-UI_Forms
 */
class ScrollContainer : public Container
{
    friend class Form;
    friend class Control;
    //friend class ControlFactory;

public:

    /**
     * Constant used to auto-hide scrollbars.
     */
    static const int ANIMATE_SCROLLBAR_OPACITY = 8;

    /**
     * The definition for container scrolling.
     */
    enum Scroll
    {
        SCROLL_NONE        = 0,
        SCROLL_HORIZONTAL  = 0x01,
        SCROLL_VERTICAL    = 0x02,
        SCROLL_BOTH = SCROLL_HORIZONTAL | SCROLL_VERTICAL
    };

    /**
     * @see Control::updateAbsoluteBounds
     */
     void updateAbsoluteBounds(const Vector2& offset) override;

    /**
     * Sets the allowed scroll directions for this container.
     *
     * @param scroll The allowed scroll directions for this container.
     */
    void setScroll(Scroll scroll);

    /**
     * Gets the allowed scroll directions for this container.
     *
     * @return The allowed scroll directions for this container.
     */
    Scroll getScroll() const;

    /**
     * Set whether scrollbars auto hidden when they become static.
     *
     * @param autoHide true to auto hide the scrollbars when they become static.
     */
    void setScrollBarsAutoHide(bool autoHide);

    /**
     * Whether scrollbars are always visible, or only visible while scrolling.
     *
     * @return Whether scrollbars are always visible.
     */
    bool isScrollBarsAutoHide() const;

    /**
     * Whether this container is currently being scrolled.
     *
     * @return Whether this container is currently being scrolled.
     */
    bool isScrolling() const;

    /**
     * Stops this container from scrolling if it is currently being scrolled.
     */
    void stopScrolling();

    /**
     * Get the friction applied to scrolling velocity for this container.
     */
    float getScrollingFriction() const;

    /**
     * Get the friction applied to scrolling velocity for this container.
     * A higher value will bring the viewport to a stop sooner.
     */
    void setScrollingFriction(float friction);

    /**
     * Get the speed added to scrolling velocity on a scroll-wheel event.
     */
    float getScrollWheelSpeed() const;

    /**
     * Set the speed added to scrolling velocity on a scroll-wheel event.
     */
    void setScrollWheelSpeed(float speed);

    /**
     * Get an offset of how far this layout has been scrolled in each direction.
     */
    const Vector2& getScrollPosition() const;

    /**
     * Set an offset of how far this layout has been scrolled in each direction.
     */
    void setScrollPosition(const Vector2& scrollPosition);

    /**
     * Get whether this container requires focus in order to handle scroll-wheel events.
     */
    bool getScrollWheelRequiresFocus() const;

    /**
     * Set whether this container requires focus in order to handle scroll-wheel events.
     * If this property is set to true, scroll-wheel events will only be handled when the container has focus.
     * If this property is set tofalse, scroll-wheel events will only be handled
     * when the container is in the HOVER state.
     *
     * @param required Whether focus is required in order to handle scroll-wheel events.
     */
    void setScrollWheelRequiresFocus(bool required);

    /**
     * @see AnimationTarget::getAnimationPropertyComponentCount
     */
    virtual unsigned int getAnimationPropertyComponentCount(int propertyId) const;

    /**
     * @see AnimationTarget::getAnimationProperty
     */
    virtual void getAnimationPropertyValue(int propertyId, AnimationValue* value);

    /**
     * @see AnimationTarget::setAnimationProperty
     */
    virtual void setAnimationPropertyValue(int propertyId, AnimationValue* value, float blendWeight = 1.0f);

protected:
    void getBarPadding(int* vertical, int* horizontal);

    /**
     * Constructor.
     */
    ScrollContainer();

    /**
     * Destructor.
     */
    virtual ~ScrollContainer();


    virtual void onSerialize(Serializer* serializer);

    virtual void onDeserialize(Serializer* serializer);

    /**
     * @see Control::updateState
     */
    void updateState(Control::State state) override;


    /**
     * Updates the bounds for this container's child controls.
     */
    bool updateChildBounds() override;

    /**
     * @see Control::draw
     */
    virtual unsigned int draw(Form* form, const Rectangle& clip, RenderInfo* view) override;

    /**
     * Update scroll position and velocity.
     */
    void updateScroll();

    /**
     * Applies touch events to scroll state.
     *
     * @param evt The touch event that occurred.
     * @param x The x position of the touch in pixels. Left edge is zero.
     * @param y The y position of the touch in pixels. Top edge is zero.
     * @param contactIndex The order of occurrence for multiple touch contacts starting at zero.
     *
     * @return Whether the touch event was consumed by scrolling within this container.
     *
     * @see MotionEvent::MotionType
     */
    bool touchEventScroll(MotionEvent::MotionType evt, int x, int y, unsigned int contactIndex);

    /**
     * Mouse scroll event callback.
     *
     * @param evt The mouse scroll event that occurred.
     * @param x The x position of the scroll in pixels. Left edge is zero.
     * @param y The y position of the scroll in pixels. Top edge is zero.
     * @param wheelDelta The value change of the mouse's scroll wheel.
     *
     * @return Whether the scroll event was consumed by scrolling within this container.
     *
     * @see MotionEvent::MotionType
     */
    bool mouseEventScroll(MotionEvent::MotionType evt, int x, int y, int wheelDelta);

    /**
     * Get a Scroll enum from a matching string.
     *
     * @param scroll A string representing a Scroll enum.
     *
     * @return The Scroll enum value that matches the given string.
     */
    static Scroll getScroll(const char* scroll);

    /**
     * Scrollbar top cap image.
     */
    //ThemeImage* _scrollBarTopCap;
    /**
     * Scrollbar vertical track image.
     */
    ThemeImage* _scrollBarVertical;
    /**
     * Scrollbar bottom cap image.
     */
    //ThemeImage* _scrollBarBottomCap;
    /**
     * Scrollbar left cap image.
     */
    //ThemeImage* _scrollBarLeftCap;
    /**
     * Scrollbar horizontal track image.
     */
    ThemeImage* _scrollBarHorizontal;
    /**
     * Scrollbar horizontal image.
     */
    //ThemeImage* _scrollBarRightCap;
    /** 
     * Flag representing whether scrolling is enabled, and in which directions.
     */
    Scroll _scroll;
    /** 
     * Scroll bar bounds.
     */
    Rectangle _scrollBarBounds;
    /** 
     * How far this layout has been scrolled in each direction.
     */
    Vector2 _scrollPosition;
    /** 
     * Whether the scrollbars should auto-hide. Default is false.
     */
    bool _scrollBarsAutoHide;
    /** 
     * Used to animate scrollbars fading out.
     */
    float _scrollBarOpacity;
    /** 
     * Whether the user is currently touching / holding the mouse down within this layout's container.
     */
    bool _scrolling;
    /** 
     * First scrolling touch x position.
     */
    int _scrollingVeryFirstX;
    /**
     * First scrolling touch y position.
     */
    int _scrollingVeryFirstY;
    /**
     * First scrolling touch x position since last change in direction.
     */ 
    int _scrollingFirstX;
    /** 
     * First scrolling touch y position since last change in direction.
     */ 
    int _scrollingFirstY;
    /** 
     * The last y position when scrolling.
     */ 
    int _scrollingLastX;
    /** 
     * The last x position when scrolling.
     */ 
    int _scrollingLastY;
    /** 
     * Time we started scrolling horizontally.
     */ 
    double _scrollingStartTimeX;
    /** 
     * Time we started scrolling vertically.
     */ 
    double _scrollingStartTimeY;
    /** 
     * The last time we were scrolling.
     */
    double _scrollingLastTime;
    /** 
     * Speed to continue scrolling at after touch release or a scroll-wheel event.
     */ 
    Vector2 _scrollingVelocity;
    /** 
     * Friction dampens velocity.
     */ 
    float _scrollingFriction;
    /**
     * Amount to add to scrolling velocity on a scroll-wheel event;
     */
    float _scrollWheelSpeed;
    /** 
     * Are we scrolling to the right?
     */ 
    bool _scrollingRight;
    /** 
     * Are we scrolling down?
     */ 
    bool _scrollingDown;
    /**
     * Locked to scrolling vertically by grabbing the scrollbar with the mouse.
     */
    bool _scrollingMouseVertically;
    /**
     * Locked to scrolling horizontally by grabbing the scrollbar with the mouse.
     */
    bool _scrollingMouseHorizontally;

private:

    /**
     * Constructor.
     */
    ScrollContainer(const Container& copy);

    // Starts scrolling at the given horizontal and vertical speeds.
    void startScrolling(float x, float y, bool resetTime = true);


    AnimationClip* _scrollBarOpacityClip;

    //bool _selectButtonDown;
    double _lastFrameTime;

    float _totalWidth;
    float _totalHeight;

    bool _initializedWithScroll;
    bool _scrollWheelRequiresFocus;
};

}

#endif
