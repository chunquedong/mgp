#include "base/Base.h"
#include "platform/Toolkit.h"
#include "Control.h"
#include "FormManager.h"
#include "Theme.h"
#include <algorithm>
#include "ScrollContainer.h"

namespace mgp
{

Control::Control()
    : _id(""), _boundsBits(0), _dirtyBits(DIRTY_BOUNDS | DIRTY_STATE), _consumeInputEvents(true), _alignment(ALIGN_TOP_LEFT),
    _autoSize(AUTO_SIZE_BOTH), _listeners(NULL), _style(NULL), _visible(true), _opacity(0.0f), _zIndex(-1),
    _contactIndex(INVALID_CONTACT_INDEX), _focusIndex(-1), _canFocus(false), _state(NORMAL), _parent(NULL), _styleOverridden(false)
{
#if GP_SCRIPT_ENABLE
    GP_REGISTER_SCRIPT_EVENTS();
#endif
    _styleName = "Control";
}

Control::~Control()
{
    FormManager::cur()->verifyRemovedControlState(this);

    if (_listeners)
    {
        for (std::map<Control::Listener::EventType, std::list<Control::Listener*>*>::const_iterator itr = _listeners->begin(); itr != _listeners->end(); ++itr)
        {
            std::list<Control::Listener*>* list = itr->second;
            SAFE_DELETE(list);
        }
        SAFE_DELETE(_listeners);
    }

    if (_style)
    {
        // Release the style's theme since we addRef'd it in initialize()
        getStyle()->getTheme()->release();

        if (_styleOverridden)
        {
            SAFE_RELEASE(_style);
        }
    }
}

Control::AutoSize Control::parseAutoSize(const char* str)
{
    if (str == NULL)
        return _autoSize;
    if (strcmpnocase(str, "AUTO_SIZE_WIDTH") == 0 )
        return AUTO_SIZE_WIDTH;
    if (strcmpnocase(str, "AUTO_SIZE_HEIGHT") == 0)
        return AUTO_SIZE_HEIGHT;
    if (strcmpnocase(str, "AUTO_SIZE_BOTH") == 0)
        return AUTO_SIZE_BOTH;
    return _autoSize;
}

void Control::initialize(const char* styleName, Style* style, Properties* properties)
{
    // Load our theme style
    if (properties)
    {
        styleName = properties->getString("style", styleName);
    }

    _style = style;
    if (styleName) {
        setStyleName(styleName);
    }
    else {
        setStyleName(_styleName.c_str());
    }

    GP_ASSERT(_style);
    // Increase the reference count of the style's theme while we hold the style
    getStyle()->getTheme()->addRef();
    if (properties)
    {
        const char* id = properties->getId();
        if (id)
            _id = id;

		// Properties not defined by the style.
		const char* alignmentString = properties->getString("alignment");
		_alignment = getAlignment(alignmentString);
		_consumeInputEvents = properties->getBool("consumeInputEvents", true);
		_visible = properties->getBool("visible", true);

		if (properties->exists("zIndex"))
		{
			_zIndex = properties->getInt("zIndex");
		}
		else
		{
			_zIndex = -1;
		}
		if (properties->exists("canFocus"))
			_canFocus = properties->getBool("canFocus", false);
		if (properties->exists("focusIndex"))
		{
			_focusIndex = properties->getInt("focusIndex");
		}
		else
		{
			_focusIndex = -1;
		}
		float bounds[2];
		bool boundsBits[2];
        const char* position = properties->getString("position");
        if (position && parseCoordPair(position, &bounds[0], &bounds[1], &boundsBits[0], &boundsBits[1]))
        {
            setX(bounds[0], boundsBits[0]);
            setY(bounds[1], boundsBits[1]);
		}
		else
		{
            if (properties->exists("x"))
            {
                bounds[0] = parseCoord(properties->getString("x", "0"), &boundsBits[0]);
                setX(bounds[0], boundsBits[0]);
            }
            if (properties->exists("y"))
            {
                bounds[1] = parseCoord(properties->getString("y", "0"), &boundsBits[1]);
                setY(bounds[1], boundsBits[1]);
            }
		}
        // If there is an explicitly specified size, width or height, unset the corresponding autoSize bit
        const char* size = properties->getString("size");
        if (size && parseCoordPair(size, &bounds[0], &bounds[1], &boundsBits[0], &boundsBits[1]))
        {
            setWidth(bounds[0], boundsBits[0]);
            setHeight(bounds[1], boundsBits[1]);
        }
		else
		{
            const char* width = properties->getString("width");
            if (width)
            {
                bounds[0] = parseCoord(width, &boundsBits[0]);
                setWidth(bounds[0], boundsBits[0]);
            }
            const char* height = properties->getString("height");
            if (height)
            {
                bounds[1] = parseCoord(height, &boundsBits[1]);
                setHeight(bounds[1], boundsBits[1]);
            }
		}
        // Backwards Compatibility: Support deprecated autoWidth and autoHeight properties,
        // which resolve to width=100% and height=100%.
        if (properties->getBool("autoWidth"))
            setWidth(1.0f, true);
        if (properties->getBool("autoHeight"))
            setHeight(1.0f, true);

        // Parse the auto-size mode for the control (this overrides explicit sizes and legacy autoWidth/autoHeight)
        _autoSize = parseAutoSize(properties->getString("autoSize"));

        // If there is are simple padding or margin variables, parse them
        if (properties->exists("padding"))
        {
            float pad = properties->getFloat("padding");
            setPadding(pad, pad, pad, pad);
        }
        if (properties->exists("margin"))
        {
            float margin = properties->getFloat("margin");
            setPadding(margin, margin, margin, margin);
        }

		if (properties->exists("enabled"))
		{
			setEnabled(properties->getBool("enabled"));
		}
#if GP_SCRIPT_ENABLE
		// Register script listeners for control events
		if (properties->exists("script"))
			addScript(properties->getString("script"));
#endif
		// Potentially override themed properties for all states.
		overrideThemedProperties(properties, STATE_ALL);

		// Override themed properties on specific states.
		Properties* innerSpace = properties->getNextNamespace();
		while (innerSpace != NULL)
		{
			std::string spaceName(innerSpace->getNamespace());
			std::transform(spaceName.begin(), spaceName.end(), spaceName.begin(), (int(*)(int))toupper);
			if (spaceName == "STATENORMAL")
			{
				overrideThemedProperties(innerSpace, NORMAL);
			}
			else if (spaceName == "STATEFOCUS")
			{
				overrideThemedProperties(innerSpace, FOCUS);
			}
			else if (spaceName == "STATEACTIVE")
			{
				overrideThemedProperties(innerSpace, ACTIVE);
			}
			else if (spaceName == "STATEDISABLED")
			{
				overrideThemedProperties(innerSpace, DISABLED);
			}
			else if (spaceName == "STATEHOVER")
			{
				overrideThemedProperties(innerSpace, HOVER);
			}
			else if (spaceName == "MARGIN")
			{
				setMargin(innerSpace->getFloat("top"), innerSpace->getFloat("bottom"),
					innerSpace->getFloat("left"), innerSpace->getFloat("right"));
			}
			else if (spaceName == "PADDING")
			{
				setPadding(innerSpace->getFloat("top"), innerSpace->getFloat("bottom"),
					innerSpace->getFloat("left"), innerSpace->getFloat("right"));
			}
			innerSpace = properties->getNextNamespace();
		}
	}
}

const char* Control::getId() const
{
    return _id.c_str();
}

void Control::setId(const char* id)
{
	_id = id ? id : "";
}

float Control::getX() const
{
    return _localBounds.x;
}

void Control::setX(float x, bool percentage)
{
    if (_desiredBounds.x != x || percentage != ((_boundsBits & BOUNDS_X_PERCENTAGE_BIT) != 0))
    {
        setXInternal(x, percentage);
        setDirty(DIRTY_BOUNDS);
    }
}

void Control::setXInternal(float x, bool percentage)
{
    _desiredBounds.x = x;
    if (percentage)
    {
        _boundsBits |= BOUNDS_X_PERCENTAGE_BIT;
    }
    else
    {
        _boundsBits &= ~BOUNDS_X_PERCENTAGE_BIT;
        _localBounds.x = x;
    }
}

bool Control::isXPercentage() const
{
    return (_boundsBits & BOUNDS_X_PERCENTAGE_BIT) != 0;
}

float Control::getY() const
{
    return _localBounds.y;
}

void Control::setY(float y, bool percentage)
{
    if (_desiredBounds.y != y || percentage != ((_boundsBits & BOUNDS_Y_PERCENTAGE_BIT) != 0))
    {
        setYInternal(y, percentage);
        setDirty(DIRTY_BOUNDS);
    }
}

void Control::setYInternal(float y, bool percentage)
{
    _desiredBounds.y = y;
    if (percentage)
    {
        _boundsBits |= BOUNDS_Y_PERCENTAGE_BIT;
    }
    else
    {
        _boundsBits &= ~BOUNDS_Y_PERCENTAGE_BIT;
        _localBounds.y = y;
    }
}

bool Control::isYPercentage() const
{
    return (_boundsBits & BOUNDS_Y_PERCENTAGE_BIT) != 0;
}

float Control::getWidth() const
{
    return _localBounds.width;
}

void Control::setWidth(float width, bool percentage)
{
    _autoSize = (AutoSize)(_autoSize & ~AUTO_SIZE_WIDTH);

    if (_desiredBounds.width != width || percentage != ((_boundsBits & BOUNDS_WIDTH_PERCENTAGE_BIT) != 0))
    {
        setWidthInternal(width, percentage);
        setDirty(DIRTY_BOUNDS);
    }
}

void Control::setWidthInternal(float width, bool percentage)
{
    _desiredBounds.width = width;
    if (percentage)
    {
        _boundsBits |= BOUNDS_WIDTH_PERCENTAGE_BIT;
    }
    else
    {
        _boundsBits &= ~BOUNDS_WIDTH_PERCENTAGE_BIT;
        _localBounds.width = width;
    }
}

bool Control::isWidthPercentage() const
{
    return (_boundsBits & BOUNDS_WIDTH_PERCENTAGE_BIT) != 0;
}

float Control::getHeight() const
{
    return _localBounds.height;
}

void Control::setHeight(float height, bool percentage)
{
    _autoSize = (AutoSize)(_autoSize & ~AUTO_SIZE_HEIGHT);

    if (_desiredBounds.height != height || percentage != ((_boundsBits & BOUNDS_HEIGHT_PERCENTAGE_BIT) != 0))
    {
        setHeightInternal(height, percentage);
        setDirty(DIRTY_BOUNDS);
    }
}

void Control::setHeightInternal(float height, bool percentage)
{
    _desiredBounds.height = height;
    if (percentage)
    {
        _boundsBits |= BOUNDS_HEIGHT_PERCENTAGE_BIT;
    }
    else
    {
        _boundsBits &= ~BOUNDS_HEIGHT_PERCENTAGE_BIT;
        _localBounds.height = height;
    }
}

bool Control::isHeightPercentage() const
{
    return (_boundsBits & BOUNDS_HEIGHT_PERCENTAGE_BIT) != 0;
}

void Control::setPosition(float x, float y)
{
    setX(x);
    setY(y);
}

void Control::setSize(float width, float height)
{
    setWidth(width);
    setHeight(height);
}

const Rectangle& Control::getBounds() const
{
    return _localBounds;
}

void Control::setBounds(const Rectangle& bounds)
{
    setX(bounds.x);
    setY(bounds.y);
    setWidth(bounds.width);
    setHeight(bounds.height);
}

const Rectangle& Control::getAbsoluteBounds() const
{
    return _absoluteBounds;
}

// const Rectangle& Control::getClipBounds() const
// {
//     return _clipBounds;
// }

const Rectangle& Control::getClip() const
{
    return _viewportClipBounds;
}

void Control::setAlignment(Alignment alignment)
{
    if (_alignment != alignment)
    {
        _alignment = alignment;
        setDirty(DIRTY_BOUNDS);
    }
}

Control::Alignment Control::getAlignment() const
{
    return _alignment;
}

Control::AutoSize Control::getAutoSize() const
{
    return _autoSize;
}

void Control::setAutoSize(AutoSize mode)
{
    if (_autoSize != mode)
    {
        _autoSize = mode;
        setDirty(DIRTY_BOUNDS);
    }
}

void Control::setVisible(bool visible)
{
    if (_visible != visible)
    {
        _visible = visible;

        if (!_visible) {
            Form* form = getTopLevelForm();
            if (form) form->controlDisabled(this);
        }
        setDirty(DIRTY_BOUNDS);

        // force to update parent boundaries when child is hidden
        Container* parent = _parent;
        while (parent && (parent->_autoSize != AUTO_SIZE_NONE || static_cast<Container *>(parent)->getLayout()->getType() != Layout::LAYOUT_ABSOLUTE))
        {
            parent->setDirty(DIRTY_BOUNDS);
            parent = parent->_parent;
        }
    }
}

bool Control::isVisible() const
{
    return _visible;
}

bool Control::isVisibleInHierarchy() const
{
    if (!_visible)
        return false;

    if (_parent)
        return _parent->isVisibleInHierarchy();

    return true;
}

bool Control::canFocus() const
{
    return _canFocus;
}

bool Control::canReceiveFocus() const {
    if (this->getFocusIndex() < 0 || !(this->isEnabled() && this->isVisible()))
        return false;

    if (this->canFocus())
        return true;

    return false;
}

void Control::setCanFocus(bool acceptsFocus)
{
    _canFocus = acceptsFocus;
}

bool Control::hasFocus() const
{
    Form* form = getTopLevelForm();
    return (form && form->getFocusControl() == this);
}

bool Control::setFocus()
{
    Form* form = getTopLevelForm();
    if (form && form->getFocusControl() != this && canFocus())
    {
        form->setFocusControl(this);
        return true;
    }

    return false;
}

void Control::setEnabled(bool enabled)
{
    if (enabled != isEnabled())
    {
        if (!enabled)
        {
            Form* form = getTopLevelForm();
            if (form) form->controlDisabled(this);
        }
        _state = enabled ? NORMAL : DISABLED;
        setDirty(DIRTY_STATE);
    }
}

bool Control::isEnabled() const
{
    return (_state != DISABLED);
}

bool Control::isEnabledInHierarchy() const
{
    if (!isEnabled())
        return false;
    if (_parent)
        return _parent->isEnabledInHierarchy();

    return true;
}

void Control::setMargin(float top, float right, float bottom, float left)
{
    _margin.top = top;
    _margin.bottom = bottom;
    _margin.left = left;
    _margin.right = right;
}

const Margin& Control::getMargin() const
{
    return _margin;
}

void Control::setPadding(float top, float right, float bottom, float left)
{
    _padding.top = top;
    _padding.bottom = bottom;
    _padding.left = left;
    _padding.right = right;
}

const Padding& Control::getPadding() const
{
    return _padding;
}

Style* Control::getStyle() const
{
    return _style;
}

void Control::setStyle(Style* style)
{
    if (style != _style)
    {
        _style = style;
        setDirty(DIRTY_BOUNDS);
    }
}

Theme* Control::getTheme() const
{
    return _style ? getStyle()->getTheme() : NULL;
}

const char* Control::getStyleName() const {
    return _styleName.c_str();
}

void Control::setStyleName(const char* styleName) {
    _styleName = styleName;
    if (_styleName.size() == 0) return;

    Style* styleObj = NULL;
    if (_style) {
        Style* styleObj = getTheme()->getStyle(styleName);
        if (!styleObj) {
            styleObj = Theme::getDefault()->getStyle(styleName);
        }
        if (!styleObj)
        {
            // No style was found, use an empty style
            styleObj = getTheme()->getEmptyStyle();
        }
    }
    else {
        styleObj = Theme::getDefault()->getStyle(styleName);
    }

    if (!styleObj)
    {
        // No style was found, use an empty style
        styleObj = Theme::getDefault()->getEmptyStyle();
    }

    setStyle(styleObj);
}

Control::State Control::getState() const
{
    Form* form = getTopLevelForm();
    if (form && form->getFocusControl() == this)
    {
        // Active is the only state that overrides focus state
        return _state == ACTIVE ? ACTIVE : FOCUS;
    }

    return _state;
}

void Control::setConsumeInputEvents(bool consume)
{
    _consumeInputEvents = consume;
}

bool Control::getConsumeInputEvents()
{
    return _consumeInputEvents;
}

int Control::getZIndex() const
{
    return _zIndex;
}

void Control::setZIndex(int zIndex)
{
    if (zIndex != _zIndex)
    {
        _zIndex = zIndex;

        if (_parent)
        {
			_parent->sortControls();
        }
    }
}

int Control::getFocusIndex() const
{
    return _focusIndex;
}

void Control::setFocusIndex(int focusIndex)
{
    _focusIndex = focusIndex;
}

void Control::addListener(Control::Listener* listener, int eventFlags)
{
    GP_ASSERT(listener);

    for (int i = 0; i < 32; ++i) {
        uint32_t flag = 1U << i;
        if (eventFlags & flag) {
            addSpecificListener(listener, (Listener::EventType)flag);
        }
    }
}

void Control::removeListener(Control::Listener* listener)
{
    if (_listeners == NULL || listener == NULL)
        return;

    for (std::map<Control::Listener::EventType, std::list<Control::Listener*>*>::iterator itr = _listeners->begin(); itr != _listeners->end();)
    {
        itr->second->remove(listener);

        if(itr->second->empty())
        {
            std::list<Control::Listener*>* list = itr->second;
            _listeners->erase(itr++);
            SAFE_DELETE(list);
        }
        else
            ++itr;
    }

    if (_listeners->empty())
        SAFE_DELETE(_listeners);
}

void Control::addSpecificListener(Control::Listener* listener, Control::Listener::EventType eventType)
{
    GP_ASSERT(listener);

    if (!_listeners)
    {
        _listeners = new std::map<Control::Listener::EventType, std::list<Control::Listener*>*>();
    }

    std::map<Control::Listener::EventType, std::list<Control::Listener*>*>::const_iterator itr = _listeners->find(eventType);
    if (itr == _listeners->end())
    {
        _listeners->insert(std::make_pair(eventType, new std::list<Control::Listener*>()));
        itr = _listeners->find(eventType);
    }

    std::list<Control::Listener*>* listenerList = itr->second;
    listenerList->push_back(listener);
}

bool Control::touchEvent(MotionEvent::MotionType evt, int x, int y, unsigned int contactIndex)
{
    return false;
}

bool Control::keyEvent(Keyboard::KeyEvent evt, int key)
{
    return false;
}

bool Control::mouseEvent(MotionEvent::MotionType evt, int x, int y, int wheelDelta)
{
    // Return false instead of _consumeInputEvents to allow handling to be
    // routed to touchEvent before consuming.
    return false;
}

void Control::notifyListeners(Control::Listener::EventType eventType)
{
    // This method runs untrusted code by notifying listeners of events.
    // If the user calls exit() or otherwise releases this control, we
    // need to keep it alive until the method returns.
    this->addRef();

    controlEvent(eventType);

    if (_listeners)
    {
        std::map<Control::Listener::EventType, std::list<Control::Listener*>*>::const_iterator itr = _listeners->find(eventType);
        if (itr != _listeners->end())
        {
            std::list<Control::Listener*>* listenerList = itr->second;
            for (std::list<Control::Listener*>::iterator listenerItr = listenerList->begin(); listenerItr != listenerList->end(); ++listenerItr)
            {
                GP_ASSERT(*listenerItr);
                (*listenerItr)->controlEvent(this, eventType);
            }
        }
    }
#if GP_SCRIPT_ENABLE
    fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(Control, controlEvent), dynamic_cast<void*>(this), eventType);
#endif
    this->release();
}

void Control::controlEvent(Control::Listener::EventType evt)
{
}

void Control::setDirty(int bits, bool recursive)
{
    _dirtyBits |= bits;
}

bool Control::isDirty(int bit) const
{
    return (_dirtyBits & bit) == bit;
}

void Control::update(float elapsedTime)
{
    State state = getState();

    // Update visual state if it's dirty
    if (_dirtyBits & DIRTY_STATE)
        updateState(getState());

    // Since opacity is pre-multiplied, we compute it every frame so that we don't need to
    // dirty the entire hierarchy any time a state changes (which could affect opacity).
    _opacity = getStyle()->getOpacity();
    if (_parent)
        _opacity *= _parent->_opacity;
}

void Control::updateState(State state)
{
    // Clear dirty state bit
    _dirtyBits &= ~DIRTY_STATE;
}

bool Control::updateChildBounds() {
    return false;
}

bool Control::updateLayout(const Vector2& offset)
{
    // If our state is currently dirty, update it here so that any rendering state objects needed
    // for bounds computation are accessible.
    State state = getState();
    if (_dirtyBits & DIRTY_STATE)
    {
        updateState(state);
        _dirtyBits &= ~DIRTY_STATE;
    }

    // Clear our dirty bounds bit
    bool dirtyBounds = (_dirtyBits & DIRTY_BOUNDS) != 0;
    _dirtyBits &= ~DIRTY_BOUNDS;

    // If we are a container, always update child bounds first
    bool changed = updateChildBounds();

    if (dirtyBounds)
    {
        // Store old bounds so we can determine if they change
        Rectangle oldAbsoluteBounds(_absoluteBounds);
        Rectangle oldAbsoluteClipBounds(_absoluteClipBounds);
        Rectangle oldViewportBounds(_viewportBounds);
        Rectangle oldViewportClipBounds(_viewportClipBounds);

        updateBounds();
        updateAbsoluteBounds(offset);

        if (_absoluteBounds != oldAbsoluteBounds ||
            _absoluteClipBounds != oldAbsoluteClipBounds ||
            _viewportBounds != oldViewportBounds ||
            _viewportClipBounds != oldViewportClipBounds)
        {
            onBoundsUpdate();
            changed = true;
        }
    }

    return changed;
}

void Control::onBoundsUpdate() {

}

void Control::updateBounds()
{
    Toolkit* game = Toolkit::cur();

    const Rectangle parentAbsoluteBounds = _parent ? _parent->_viewportBounds : Rectangle(0, 0, game->getDpWidth(), game->getDpHeight());

    const Margin& margin = getMargin();

    // Calculate local unclipped bounds.
    _localBounds.set(_desiredBounds);
    if (isXPercentage())
        _localBounds.x = _localBounds.x * parentAbsoluteBounds.width + margin.left;
    if (isYPercentage())
        _localBounds.y = _localBounds.y * parentAbsoluteBounds.height + margin.top;
    if (isWidthPercentage())
        _localBounds.width *= parentAbsoluteBounds.width;
    if (isHeightPercentage())
        _localBounds.height *= parentAbsoluteBounds.height;

    // Apply control alignment
    if (_alignment != Control::ALIGN_TOP_LEFT)
    {
        const Margin& margin = getMargin();
        const Rectangle& parentBounds = _parent ? _parent->getBounds() : Rectangle(0, 0, game->getDpWidth(), game->getDpHeight());
        //const Border& parentBorder = _parent ? _parent->getBorder(_parent->getState()) : Border::empty();
        const Padding& parentPadding = _parent ? _parent->getPadding() : Padding::empty();

        float clipWidth, clipHeight;
        ScrollContainer* parent = dynamic_cast<ScrollContainer*>(_parent);
        if (parent && (parent->getScroll() != ScrollContainer::SCROLL_NONE))
        {
            int vertical;
            int horizontal;
            parent->getBarPadding(&vertical, &horizontal);
            //const Rectangle& verticalScrollBarBounds = parent->getImageRegion("verticalScrollBar", _parent->getState());
            //const Rectangle& horizontalScrollBarBounds = parent->getImageRegion("horizontalScrollBar", _parent->getState());
            clipWidth = parentBounds.width - parentPadding.left - parentPadding.right - vertical;
            clipHeight = parentBounds.height - parentPadding.top - parentPadding.bottom - horizontal;
        }
        else
        {
            clipWidth = parentBounds.width - parentPadding.left - parentPadding.right;
            clipHeight = parentBounds.height - parentPadding.top - parentPadding.bottom;
        }

        // Vertical alignment
        if ((_alignment & Control::ALIGN_BOTTOM) == Control::ALIGN_BOTTOM)
        {
            _localBounds.y = clipHeight - _localBounds.height - margin.bottom;
        }
        else if ((_alignment & Control::ALIGN_VCENTER) == Control::ALIGN_VCENTER)
        {
            _localBounds.y = clipHeight * 0.5f - _localBounds.height * 0.5f + (margin.top - margin.bottom) * 0.5f;
        }
        else if ((_alignment & Control::ALIGN_TOP) == Control::ALIGN_TOP)
        {
            _localBounds.y = margin.top;
        }

        // Horizontal alignment
        if ((_alignment & Control::ALIGN_RIGHT) == Control::ALIGN_RIGHT)
        {
            _localBounds.x = clipWidth - _localBounds.width - margin.right;
        }
        else if ((_alignment & Control::ALIGN_HCENTER) == Control::ALIGN_HCENTER)
        {
            _localBounds.x = clipWidth * 0.5f - _localBounds.width * 0.5f + (margin.left - margin.right) * 0.5f;
        }
        else if ((_alignment & Control::ALIGN_LEFT) == Control::ALIGN_LEFT)
        {
            _localBounds.x = margin.left;
        }
    }
}

void Control::updateAbsoluteBounds(const Vector2& offset)
{
    Toolkit* game = Toolkit::cur();

    const Rectangle parentAbsoluteBounds = _parent ? _parent->_viewportBounds : Rectangle(0, 0, game->getDpWidth(), game->getDpHeight());
    const Rectangle parentAbsoluteClip = _parent ? _parent->_viewportClipBounds : parentAbsoluteBounds;

    // Compute content area padding values
    //const Border& border = getBorder(NORMAL);
    const Padding& padding = getPadding();
    float lpadding = padding.left;
    float rpadding = padding.right;
    float tpadding = padding.top;
    float bpadding = padding.bottom;
    float hpadding = lpadding + rpadding;
    float vpadding = tpadding + bpadding;

    // Calculate absolute unclipped bounds
    _absoluteBounds.set(
        parentAbsoluteBounds.x + offset.x + _localBounds.x,
        parentAbsoluteBounds.y + offset.y + _localBounds.y,
        _localBounds.width,
        _localBounds.height);

    // Calculate absolute clipped bounds
    Rectangle::intersect(_absoluteBounds, parentAbsoluteClip, &_absoluteClipBounds);

    // Calculate the local clipped bounds
    // _clipBounds.set(
    //     max(_absoluteClipBounds.x - _absoluteBounds.x, 0.0f),
    //     max(_absoluteClipBounds.y - _absoluteBounds.y, 0.0f),
    //     _absoluteClipBounds.width,
    //     _absoluteClipBounds.height
    //     );

    // Calculate the absolute unclipped viewport bounds (content area, which does not include border and padding)
    _viewportBounds.set(
        _absoluteBounds.x + lpadding,
        _absoluteBounds.y + tpadding,
        _absoluteBounds.width - hpadding,
        _absoluteBounds.height - vpadding);

    // Calculate the absolute clipped viewport bounds
    Rectangle::intersect(_viewportBounds, parentAbsoluteClip, &_viewportClipBounds);
}

void Control::startBatch(Form* form, BatchableLayer* batch)
{
    form->startBatch(batch);
}

void Control::finishBatch(Form* form, BatchableLayer* batch, RenderInfo* view)
{
    form->finishBatch(batch, view);
}

unsigned int Control::draw(Form* form, const Rectangle& clip, RenderInfo* view)
{
    if (!_visible)
        return 0;

    unsigned int drawCalls = drawBorder(form, clip, view);
    drawCalls += drawImages(form, clip, view);
    drawCalls += drawText(form, clip, view);
    return drawCalls;
}

unsigned int Control::drawBorder(Form* form, const Rectangle& clip, RenderInfo* view)
{
    BorderImage* _skin = getStyle()->getBgImage();
    if (!form || !_skin || _absoluteBounds.width <= 0 || _absoluteBounds.height <= 0)
        return 0;

    unsigned int drawCalls = 0;

    SpriteBatch* batch = getStyle()->getTheme()->getSpriteBatch();
    startBatch(form, batch);

    Vector4 skinColor = getStyle()->getBgColor((Style::OverlayType)getState());
    skinColor.w *= getStyle()->getOpacity();

    drawCalls += _skin->draw(batch, _absoluteBounds, skinColor, clip);

    finishBatch(form, batch, view);

    return drawCalls;
}

unsigned int Control::drawImages(Form* form, const Rectangle& position, RenderInfo* view)
{
    return 0;
}

unsigned int Control::drawText(Form* form, const Rectangle& position, RenderInfo* view)
{
    return 0;
}

Control::State Control::getState(const char* state)
{
    if (!state)
    {
        return NORMAL;
    }

    if (strcmp(state, "NORMAL") == 0)
    {
        return NORMAL;
    }
    else if (strcmp(state, "ACTIVE") == 0)
    {
        return ACTIVE;
    }
    else if (strcmp(state, "FOCUS") == 0)
    {
        return FOCUS;
    }
    else if (strcmp(state, "DISABLED") == 0)
    {
        return DISABLED;
    }
    else if (strcmp(state, "HOVER") == 0)
    {
        return HOVER;
    }

    return NORMAL;
}

Container* Control::getParent() const
{
    return _parent;
}

bool Control::isChild(Control* control) const
{
    if (!control)
        return false;

    Control* parent = _parent;
    while (parent)
    {
        if (parent == control)
            return true;
        parent = parent->_parent;
    }

    return false;
}

Form* Control::getTopLevelForm() const
{
    if (_parent)
        return _parent->getTopLevelForm();
    return NULL;
}

// Implementation of AnimationHandler
unsigned int Control::getAnimationPropertyComponentCount(int propertyId) const
{
    switch(propertyId)
    {
    case ANIMATE_POSITION:
    case ANIMATE_SIZE:
        return 2;

    case ANIMATE_POSITION_X:
    case ANIMATE_POSITION_Y:
    case ANIMATE_SIZE_WIDTH:
    case ANIMATE_SIZE_HEIGHT:
    case ANIMATE_OPACITY:
        return 1;

    default:
        return -1;
    }
}

void Control::getAnimationPropertyValue(int propertyId, AnimationValue* value)
{
    GP_ASSERT(value);

    switch (propertyId)
    {
    case ANIMATE_POSITION:
        value->setFloat(0, _localBounds.x);
        value->setFloat(1, _localBounds.y);
        break;
    case ANIMATE_SIZE:
        value->setFloat(0, _localBounds.width);
        value->setFloat(1, _localBounds.height);
        break;
    case ANIMATE_POSITION_X:
        value->setFloat(0, _localBounds.x);
        break;
    case ANIMATE_POSITION_Y:
        value->setFloat(0, _localBounds.y);
        break;
    case ANIMATE_SIZE_WIDTH:
        value->setFloat(0, _localBounds.width);
        break;
    case ANIMATE_SIZE_HEIGHT:
        value->setFloat(0, _localBounds.height);
        break;
    case ANIMATE_OPACITY: {
        float _opacity = getStyle()->getOpacity();
        value->setFloat(0, _opacity);
    }
        break;
    default:
        break;
    }
}

void Control::setAnimationPropertyValue(int propertyId, AnimationValue* value, float blendWeight)
{
    GP_ASSERT(value);

    switch(propertyId)
    {
    case ANIMATE_POSITION:
        setX(Curve::lerp(blendWeight, _localBounds.x, value->getFloat(0)), isXPercentage());
        setY(Curve::lerp(blendWeight, _localBounds.y, value->getFloat(1)), isYPercentage());
        break;
    case ANIMATE_POSITION_X:
        setX(Curve::lerp(blendWeight, _localBounds.x, value->getFloat(0)), isXPercentage());
        break;
    case ANIMATE_POSITION_Y:
        setY(Curve::lerp(blendWeight, _localBounds.y, value->getFloat(0)), isYPercentage());
        break;
    case ANIMATE_SIZE:
        setWidth(Curve::lerp(blendWeight, _localBounds.width, value->getFloat(0)), isWidthPercentage());
        setHeight(Curve::lerp(blendWeight, _localBounds.height, value->getFloat(1)), isHeightPercentage());
        break;
    case ANIMATE_SIZE_WIDTH:
        setWidth(Curve::lerp(blendWeight, _localBounds.width, value->getFloat(0)), isWidthPercentage());
        break;
    case ANIMATE_SIZE_HEIGHT:
        setHeight(Curve::lerp(blendWeight, _localBounds.height, value->getFloat(0)), isHeightPercentage());
        break;
    case ANIMATE_OPACITY: {
        float _opacity = getStyle()->getOpacity();
        getStyle()->setOpacity(Curve::lerp(blendWeight, _opacity, value->getFloat(0)));
    }
        break;
    }
}

void Control::overrideStyle()
{
    if (_styleOverridden)
    {
        return;
    }

    // Copy the style.
    GP_ASSERT(_style);
    _style = new Style(*_style);
    _styleOverridden = true;
}

void Control::overrideThemedProperties(Properties* properties, unsigned char states)
{
    GP_ASSERT(properties);
    GP_ASSERT(_style);
    GP_ASSERT(getStyle()->_theme);


    if (properties->exists("skin"))
    {
        //getStyle()->setSkin(skin, states);
    }

    if (properties->exists("font"))
    {
        UPtr<Font> font = Font::create(properties->getString("font"));
        getStyle()->setFont(font.get());
        //font->release();
    }

    if (properties->exists("fontSize"))
    {
        getStyle()->setFontSize(properties->getInt("fontSize"));
    }

    if (properties->exists("textColor"))
    {
        Vector4 textColor(0, 0, 0, 1);
        properties->getColor("textColor", &textColor);
        getStyle()->setTextColor(textColor);
    }

    if (properties->exists("textAlignment"))
    {
        getStyle()->setTextAlignment(FontLayout::getJustify(properties->getString("textAlignment")));
    }

    if (properties->exists("rightToLeft"))
    {
        getStyle()->setTextRightToLeft(properties->getBool("rightToLeft"));
    }
    if (properties->exists("opacity"))
    {
        getStyle()->setOpacity(properties->getFloat("opacity"));
    }
}

Control::Alignment Control::getAlignment(const char* alignment)
{
    if (!alignment)
    {
        return Control::ALIGN_TOP_LEFT;
    }

    if (strcmp(alignment, "ALIGN_LEFT") == 0)
    {
        return Control::ALIGN_LEFT;
    }
    else if (strcmp(alignment, "ALIGN_HCENTER") == 0)
    {
        return Control::ALIGN_HCENTER;
    }
    else if (strcmp(alignment, "ALIGN_RIGHT") == 0)
    {
        return Control::ALIGN_RIGHT;
    }
    else if (strcmp(alignment, "ALIGN_TOP") == 0)
    {
        return Control::ALIGN_TOP;
    }
    else if (strcmp(alignment, "ALIGN_VCENTER") == 0)
    {
        return Control::ALIGN_VCENTER;
    }
    else if (strcmp(alignment, "ALIGN_BOTTOM") == 0)
    {
        return Control::ALIGN_BOTTOM;
    }
    else if (strcmp(alignment, "ALIGN_TOP_LEFT") == 0)
    {
        return Control::ALIGN_TOP_LEFT;
    }
    else if (strcmp(alignment, "ALIGN_VCENTER_LEFT") == 0)
    {
        return Control::ALIGN_VCENTER_LEFT;
    }
    else if (strcmp(alignment, "ALIGN_BOTTOM_LEFT") == 0)
    {
        return Control::ALIGN_BOTTOM_LEFT;
    }
    else if (strcmp(alignment, "ALIGN_TOP_HCENTER") == 0)
    {
        return Control::ALIGN_TOP_HCENTER;
    }
    else if (strcmp(alignment, "ALIGN_VCENTER_HCENTER") == 0)
    {
        return Control::ALIGN_VCENTER_HCENTER;
    }
    else if (strcmp(alignment, "ALIGN_BOTTOM_HCENTER") == 0)
    {
        return Control::ALIGN_BOTTOM_HCENTER;
    }
    else if (strcmp(alignment, "ALIGN_TOP_RIGHT") == 0)
    {
        return Control::ALIGN_TOP_RIGHT;
    }
    else if (strcmp(alignment, "ALIGN_VCENTER_RIGHT") == 0)
    {
        return Control::ALIGN_VCENTER_RIGHT;
    }
    else if (strcmp(alignment, "ALIGN_BOTTOM_RIGHT") == 0)
    {
        return Control::ALIGN_BOTTOM_RIGHT;
    }
    else
    {
        GP_ERROR("Failed to get corresponding control alignment for unsupported value '%s'.", alignment);
    }
    return Control::ALIGN_TOP_LEFT;
}

float Control::parseCoord(const char* s, bool* isPercentage)
{
    const char* p;
    if ((p = strchr(s, '%')) != NULL)
    {
        std::string value(s, (std::string::size_type)(p - s));
        *isPercentage = true;
        return (float)(atof(value.c_str()) * 0.01);
    }
    *isPercentage = false;
    return (float)atof(s);
}

bool Control::parseCoordPair(const char* s, float* v1, float* v2, bool* v1Percentage, bool* v2Percentage)
{
    size_t len = strlen(s);
    const char* s2 = strchr(s, ',');
    if (s2 == NULL)
        return false;
    std::string v1Str(s, (std::string::size_type)(s2 - s));
    std::string v2Str(s2 + 1);
    *v1 = parseCoord(v1Str.c_str(), v1Percentage);
    *v2 = parseCoord(v2Str.c_str(), v2Percentage);
    return true;
}

Control* Control::findInputControl(int x, int y, bool focus, unsigned int contactIndex)
{
    Control* control = this;
    if (!(control->_visible && control->isEnabled()))
        return NULL;

    Control* result = NULL;

    // Does the passed in control's bounds intersect the specified coordinates - and 
    // does the control support the specified input state?
    if (control->_consumeInputEvents && (!focus || control->canFocus()))
    {
        if (control->_absoluteClipBounds.contains(x, y))
            result = control;
    }

    return result;
}

Control* Control::getControl(const char* id) {
    if (strcmp(id, this->getId()) == 0)
    {
        return this;
    }
    return NULL;
}

bool Control::moveFocus(Direction direction) {
    return false;
}
bool Control::moveFocusDirectional(Direction direction) {
    return false;
}

}
