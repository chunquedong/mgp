#include "base/Base.h"
#include "Container.h"
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
#include "ImageControl.h"
#include "Form.h"
#include "platform/Toolkit.h"
#include "ControlFactory.h"
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

/**
 * Sort function for use with _controls.sort(), based on Z-Order.
 * 
 * @param c1 The first control
 * @param c2 The second control
 * return true if the first controls z index is less than the second.
 */
static bool sortControlsByZOrder(Control* c1, Control* c2);

void Container::clearContacts()
{
	for (int i = 0; i < MAX_CONTACT_INDICES; ++i)
		_contactIndices[i] = false;
}

Container::Container()
    : _layout(NULL), _activeControl(NULL),
    _zIndexDefault(0), _form(NULL)
{
    clearContacts();
}

Container::~Container()
{
    std::vector<Control*>::iterator it;
    for (it = _controls.begin(); it < _controls.end(); it++)
    {
        (*it)->_parent = nullptr;
        SAFE_RELEASE((*it));
    }
    SAFE_RELEASE(_layout);
}

UPtr<Container> Container::create(const char* id, Style* style, Layout::Type layout)
{
    Container* container = new Container();
    container->_id = id ? id : "";
    container->_layout = createLayout(layout);
    container->initialize("Container", style, NULL);
    return UPtr<Container>(container);
}

Control* Container::create(Style* style, Properties* properties)
{
    Container* container = new Container();
    container->initialize("Container", style, properties);
    return container;
}

void Container::initialize(const char* typeName, Style* style, Properties* properties)
{
    Control::initialize(typeName, style, properties);

    if (properties)
    {
        // Parse layout
        Properties* layoutNS = properties->getNamespace("layout", true, false);
        if (layoutNS)
        {
            _layout = createLayout(getLayoutType(layoutNS->getString("type")));
            switch (_layout->getType())
            {
            case Layout::LAYOUT_FLOW:
                static_cast<FlowLayout*>(_layout)->setSpacing(layoutNS->getInt("horizontalSpacing"), layoutNS->getInt("verticalSpacing"));
                break;
            case Layout::LAYOUT_VERTICAL:
                static_cast<VerticalLayout*>(_layout)->setSpacing(layoutNS->getInt("spacing"));
                break;
            }
        }
        else
        {
            _layout = createLayout(getLayoutType(properties->getString("layout")));
        }

        addControls(properties);

        const char* activeControl = properties->getString("activeControl");
        if (activeControl)
        {
            for (size_t i = 0, count = _controls.size(); i < count; ++i)
            {
                if (_controls[i]->_id == activeControl)
                {
                    _activeControl = _controls[i];
                    break;
                }
            }
        }
    }

    // Create a default layout if one does not yet exist
    if (_layout == NULL)
        _layout = createLayout(Layout::LAYOUT_ABSOLUTE);
}


void Container::addControls(Properties* properties)
{
    GP_ASSERT(properties);

    // Add all the controls to this container.
    Properties* controlSpace = properties->getNextNamespace();
    while (controlSpace != NULL)
    {
        const char* controlName = controlSpace->getNamespace();

        // Pass our own style into the creation of the child control.
        // The child control's style will be looked up using the passed in style's theme.
        UPtr<Control> control = ControlFactory::getInstance()->createControl(controlName, _style, controlSpace);

        // Add the new control to the form.
        if (control.get())
        {
            addControl(std::move(control));
            //control->release();
        }

        // Get the next control.
        controlSpace = properties->getNextNamespace();
    }

    // Sort controls by Z-Order.
    sortControls();
}

const char* Container::getTypeName() const
{
    return "Container";
}

Layout* Container::getLayout()
{
    return _layout;
}

void Container::setLayout(Layout::Type type)
{
	if (_layout == NULL || _layout->getType() != type)
	{
		SAFE_RELEASE(_layout);

		_layout = createLayout(type);
        setDirty(Control::DIRTY_BOUNDS);
	}
}

unsigned int Container::addControl(UPtr<Control> control)
{
	GP_ASSERT(control.get());

	if( control->_parent == this )
	{
		// Control is already in this container.
		// Do nothing but determine and return control's index.
		const size_t size = _controls.size();
		for( size_t i = 0; i < size; ++i ) {
			Control* c = _controls[ i ];
			if( c == control.get() ) {
				return (unsigned int)i;
			}
		}

		// Should never reach this.
		GP_ASSERT( false );
		return 0;
	}

	if( control->getZIndex() == -1 ) {
		control->setZIndex( _zIndexDefault++ );
	}

	if( control->getFocusIndex() == -1 ) {
		// Find the current largest focus index
		int maxFocusIndex = 0;
		for( size_t i = 0, count = _controls.size(); i < count; ++i ) {
			if( _controls[ i ]->_focusIndex > maxFocusIndex )
				maxFocusIndex = _controls[ i ]->_focusIndex;
		}
		control->setFocusIndex( maxFocusIndex + 1 );
	}

    Control* tempControl = control.take();
	_controls.push_back(tempControl);
	//control->addRef();

	// Remove the control from its current parent
	if(tempControl->_parent )
	{
        tempControl->_parent->removeControl(tempControl);
	}

    tempControl->_parent = this;

	sortControls();
    setDirty(Control::DIRTY_BOUNDS);

	return (unsigned int)( _controls.size() - 1 );
}

void Container::insertControl(Control* control, unsigned int index)
{
    GP_ASSERT(control);

    if (control->_parent && control->_parent != this)
    {
        control->_parent->removeControl(control);
    }

    if (control->_parent != this)
    {
        std::vector<Control*>::iterator it = _controls.begin() + index;
        _controls.insert(it, control);
        control->addRef();
        control->_parent = this;
        setDirty(Control::DIRTY_BOUNDS);
    }
}

void Container::removeControl(unsigned int index)
{
    GP_ASSERT(index < _controls.size());

    std::vector<Control*>::iterator it = _controls.begin() + index;
    Control* control = *it;
    _controls.erase(it);
    control->_parent = NULL;
    setDirty(Control::DIRTY_BOUNDS);

    if (_activeControl == control)
        _activeControl = NULL;

    FormManager::cur()->verifyRemovedControlState(control);

    SAFE_RELEASE(control);
}

void Container::removeControl(const char* id)
{
    GP_ASSERT(id);

    for (size_t i = 0, size = _controls.size(); i < size; ++i)
    {
        Control* c = _controls[i];
        if (strcmp(id, c->getId()) == 0)
        {
            removeControl((unsigned int)i);
            return;
        }
    }
}

void Container::removeControl(Control* control)
{
    GP_ASSERT(control);

    for (size_t i = 0, size = _controls.size(); i < size; ++i)
    {
        Control* c = _controls[i];
        if (c == control)
        {
            removeControl((unsigned int)i);
            return;
        }
    }
}

Control* Container::getControl(unsigned int index) const
{
    GP_ASSERT(index < _controls.size());
    return _controls[index];
}

Control* Container::getControl(const char* id)
{
    GP_ASSERT(id);
    if (strcmp(id, this->getId()) == 0)
    {
        return this;
    }

    std::vector<Control*>::const_iterator it;
    for (it = _controls.begin(); it < _controls.end(); it++)
    {
        Control* c = *it;
        GP_ASSERT(c);
        Control* found = c->getControl(id);
        if (found)
        {
            return found;
        }
    }
    return NULL;
}

unsigned int Container::getControlCount() const
{
    return (unsigned int)_controls.size();
}

const std::vector<Control*>& Container::getControls() const
{
    return _controls;
}

bool Container::isRoot() const
{
    return _parent == NULL;
}


Animation* Container::getAnimation(const char* id) const
{
    Animation* animation = Control::getAnimation(id);
    if (animation)
        return animation;

    std::vector<Control*>::const_iterator itr = _controls.begin();
    std::vector<Control*>::const_iterator end = _controls.end();
    Control* control = NULL;
    for (; itr != end; itr++)
    {
        control = *itr;
        GP_ASSERT(control);
        Animation* animation = control->getAnimation(id);
        if (animation)
            return animation;
    }
    return NULL;
}



bool Container::setFocus()
{
    Form* form = this->getTopLevelForm();
    // If this container (or one of its children) already has focus, do nothing
    if (form->getFocusControl() && (form->getFocusControl() == this || form->getFocusControl()->isChild(this)))
        return true;

    // First try to set focus to our active control
    if (_activeControl)
    {
        if (_activeControl->setFocus())
            return true;
    }

    // Try to set focus to one of our children
    for (size_t i = 0, count = _controls.size(); i < count; ++i)
    {
        if (_controls[i]->setFocus())
            return true;
    }

    // Lastly, try to set focus to ourself if none of our children will accept it
    return Control::setFocus();
}

Control* Container::getActiveControl() const
{
    return _activeControl;
}

void Container::setActiveControl(Control* control)
{
    if (std::find(_controls.begin(), _controls.end(), control) != _controls.end())
    {
        _activeControl = control;

        Form* form = this->getTopLevelForm();
        // If a control within this container currently has focus, switch focus to the new active control
        if (form->getFocusControl() && form->getFocusControl() != control && form->getFocusControl()->isChild(this))
            form->setFocusControl(control);
    }
}

void Container::setDirty(int bits, bool recursive)
{
    Control::setDirty(bits, recursive);
    if (recursive) {
        for (size_t i = 0, count = _controls.size(); i < count; ++i)
        {
            Control* ctrl = _controls[i];
            ctrl->setDirty(bits, recursive);
        }
    }
}

Form* Container::getTopLevelForm() const
{
    if (_parent)
        return _parent->getTopLevelForm();
    else
        return this->_form;
    return NULL;
}

void Container::update(float elapsedTime)
{
    Control::update(elapsedTime);

    for (size_t i = 0, count = _controls.size(); i < count; ++i)
        _controls[i]->update(elapsedTime);
}

Control* Container::findInputControl(int x, int y, bool focus, unsigned int contactIndex)
{
    Control* control = this;
    if (!(control->_visible && control->isEnabled()))
        return NULL;

    Container* container = this;
    for (unsigned int i = 0, childCount = container->getControlCount(); i < childCount; ++i)
    {
        Control* ctrl = container->getControl(i)->findInputControl(x, y, focus, contactIndex);
        if (ctrl) return ctrl;
    }

    Control* result = Control::findInputControl(x, y, focus, contactIndex);
    return result;
}


void Container::onBoundsUpdate() {
    //setChildrenDirty(DIRTY_BOUNDS, true);
    for (size_t i = 0, count = _controls.size(); i < count; ++i)
    {
        Control* ctrl = _controls[i];
        ctrl->setDirty(DIRTY_BOUNDS, true);
    }
}

bool Container::updateChildBounds()
{
    bool result = false;

    for (size_t i = 0, count = _controls.size(); i < count; ++i)
    {
        Control* ctrl = _controls[i];
        GP_ASSERT(ctrl);

        if (ctrl->isVisible())
        {
            bool changed = ctrl->updateLayout(Vector2());

            // If the child bounds have changed, dirty our bounds and all of our
            // parent bounds so that our layout and/or bounds are recomputed.
            if (changed)
            {
                Control* parent = this;
                while (parent && (parent->_autoSize != AUTO_SIZE_NONE || static_cast<Container*>(parent)->getLayout()->getType() != Layout::LAYOUT_ABSOLUTE))
                {
                    parent->setDirty(DIRTY_BOUNDS);
                    parent = parent->_parent;
                }
            }

            result = result || changed;
        }
    }

    return result;
}


unsigned int Container::draw(Form* form, const Rectangle& clip, RenderInfo* view)
{
    if (!_visible)
        return 0;

    // Draw container skin
    unsigned int drawCalls = Control::draw(form, clip, view);

    // Draw child controls
    for (size_t i = 0, count = _controls.size(); i < count; ++i)
    {
        Control* control = _controls[i];
        if (control && control->_absoluteClipBounds.intersects(_absoluteClipBounds))
        {
            drawCalls += control->draw(form, _viewportClipBounds, view);
        }
    }

    return drawCalls;
}


bool Container::canReceiveFocus() const
{
    if (this->getFocusIndex() < 0 || !(this->isEnabled() && this->isVisible()))
        return false;

    if (this->canFocus())
        return true;

    for (unsigned int i = 0, count = (unsigned int)getControlCount(); i < count; ++i)
    {
        if (getControl(i)->canReceiveFocus())
            return true;
    }
    return false;
}

bool Container::moveFocus(Direction direction)
{
    switch (direction)
    {
    case NEXT:
    case PREVIOUS:
        return moveFocusNextPrevious(direction);

    case UP:
    case DOWN:
    case LEFT:
    case RIGHT:
        return moveFocusDirectional(direction);

    default:
        return false;
    }
}

bool Container::moveFocusNextPrevious(Direction direction)
{
    // Get the current control that has focus (either directly or indirectly) within this container
    Form* form = getTopLevelForm();
    Control* currentFocus = form ? form->getFocusControl() : NULL;
    Control* current = NULL;
    if (currentFocus && currentFocus->isChild(this))
    {
        if (currentFocus->_parent == this)
        {
            // Currently focused control is a direct child of us
            current = currentFocus;
        }
        else
        {
            // Currently focused control is a child of one of our child containers
            for (size_t i = 0, count = _controls.size(); i < count; ++i)
            {
                if (currentFocus->isChild(_controls[i]))
                {
                    current = _controls[i];
                    break;
                }
            }
        }
    }

    Control* nextCtrl = NULL;
    int nextIndex = direction == NEXT ? INT_MAX : INT_MIN;
    bool moveFirst = false;

    if (current)
    {
        // There is a control inside us that currently has focus, so find the next control that
        // should receive focus.
        int focusableControlCount = 0; // track the number of valid focusable controls in this container

        for (size_t i = 0, count = _controls.size(); i < count; ++i)
        {
            Control* ctrl = _controls[i];
            if (!ctrl->canReceiveFocus())
                continue;

            if ((direction == NEXT && ctrl->_focusIndex > current->_focusIndex && ctrl->_focusIndex < nextIndex) ||
                (direction == PREVIOUS && ctrl->_focusIndex < current->_focusIndex && ctrl->_focusIndex > nextIndex))
            {
                nextCtrl = ctrl;
                nextIndex = ctrl->_focusIndex;
            }

            ++focusableControlCount;
        }

        if (nextCtrl)
        {
            if (nextCtrl->moveFocus(direction))
                return true;
            if (nextCtrl->setFocus())
                return true;
        }

        // Search up into our parent container for a focus move
        if (_parent && _parent->moveFocus(direction))
            return true;

        // We didn't find a control to move to, so we must be the first or last focusable control in our parent.
        // Wrap focus to the other side of the container.
        if (focusableControlCount > 1)
        {
            moveFirst = true;
        }
    }
    else
    {
        moveFirst = true;
    }

    if (moveFirst)
    {
        nextIndex = direction == NEXT ? INT_MAX : INT_MIN;
        nextCtrl = NULL;
        for (size_t i = 0, count = _controls.size(); i < count; ++i)
        {
            Control* ctrl = _controls[i];
            if (!ctrl->canReceiveFocus())
                continue;
            if ((direction == NEXT && ctrl->_focusIndex < nextIndex) ||
                (direction == PREVIOUS && ctrl->_focusIndex > nextIndex))
            {
                nextCtrl = ctrl;
                nextIndex = ctrl->_focusIndex;
            }
        }

        if (nextCtrl)
        {
            if (nextCtrl->moveFocus(direction))
                return true;
            if (nextCtrl->setFocus())
                return true;
        }
    }

    return false;
}

bool Container::moveFocusDirectional(Direction direction)
{
    Form* form = getTopLevelForm();
    Control* startControl = NULL;
    if (form) {
        startControl = form->getFocusControl();
    }
    if (startControl == NULL)
        return false;

    const Rectangle& startBounds = startControl->_absoluteBounds;

    Control* next = NULL;
    Vector2 vStart, vNext;
    float distance = FLT_MAX;

    switch (direction)
    {
    case UP:
        vStart.set(startBounds.x + startBounds.width * 0.5f, startBounds.y);
        break;
    case DOWN:
        vStart.set(startBounds.x + startBounds.width * 0.5f, startBounds.bottom());
        break;
    case LEFT:
        vStart.set(startBounds.x, startBounds.y + startBounds.height * 0.5f);
        break;
    case RIGHT:
        vStart.set(startBounds.right(), startBounds.y + startBounds.height * 0.5f);
        break;
    }

    for (size_t i = 0, count = _controls.size(); i < count; ++i)
    {
        Control* ctrl = _controls[i];
        if (!ctrl->canReceiveFocus())
            continue;

        const Rectangle& nextBounds = ctrl->getAbsoluteBounds();
        switch (direction)
        {
        case UP:
            vNext.set(nextBounds.x + nextBounds.width * 0.5f, nextBounds.bottom());
            if (vNext.y > vStart.y)
                continue;
            break;
        case DOWN:
            vNext.set(nextBounds.x + nextBounds.width * 0.5f, nextBounds.y);
            if (vNext.y < vStart.y)
                continue;
            break;
        case LEFT:
            vNext.set(nextBounds.right(), nextBounds.y + nextBounds.height * 0.5f);
            if (vNext.x > vStart.x)
                continue;
            break;
        case RIGHT:
            vNext.set(nextBounds.x, nextBounds.y + nextBounds.height * 0.5f);
            if (vNext.x < vStart.x)
                continue;
            break;
        }

        float nextDistance = vStart.distance(vNext);
        if (std::fabs(nextDistance) < distance)
        {
            distance = nextDistance;
            next = ctrl;
        }
    }

    if (next)
    {
        // If this control is a container, try to move focus to the first control within it
        if (next->moveFocusDirectional(direction)) {
            return true;
        }

        if (next->setFocus())
            return true;
    }
    else
    {
        // If no control was found, try searching in our parent container
        if (_parent && _parent->moveFocusDirectional(direction))
            return true;
    }

    return false;
}


void Container::sortControls()
{
    if (_layout->getType() == Layout::LAYOUT_ABSOLUTE)
    {
        std::sort(_controls.begin(), _controls.end(), &sortControlsByZOrder);
    }
}


bool Container::inContact()
{
    for (int i = 0; i < MAX_CONTACT_INDICES; ++i)
    {
        if (_contactIndices[i])
            return true;
    }
    return false;
}


void Container::updateBounds()
{
    // Handle automatically sizing based on our children
    if (_autoSize != AUTO_SIZE_NONE)
    {
        if (_autoSize & AUTO_SIZE_WIDTH)
        {
            // Size ourself to tightly fit the width of our children
            float width = 0;
            for (size_t i = 0, count = _controls.size(); i < count; ++i)
            {
                Control* ctrl = _controls[i];
                if (ctrl->isVisible() && !ctrl->isWidthPercentage())
                {
                    float w = ctrl->getWidth() + ctrl->getMargin().right;
                    if (!ctrl->isXPercentage())
                        w += ctrl->getX();
                    if (width < w)
                        width = w;
                }
            }
            width += getPadding().left + getPadding().right;
            setWidthInternal(width);
        }

        if (_autoSize & AUTO_SIZE_HEIGHT)
        {
            // Size ourself to tightly fit the height of our children
            float height = 0;
            for (size_t i = 0, count = _controls.size(); i < count; ++i)
            {
                Control* ctrl = _controls[i];
                if (ctrl->isVisible() && !ctrl->isHeightPercentage())
                {
                    float h = ctrl->getHeight() + ctrl->getMargin().bottom;
                    if (!ctrl->isYPercentage())
                        h += ctrl->getY();
                    if (height < h)
                        height = h;
                }
            }
            height += getPadding().top + getPadding().bottom;
            setHeightInternal(height);
        }
    }

    // Compute total bounds of container
    Control::updateBounds();

    // Update layout to position children correctly within us
    GP_ASSERT(_layout);
    _layout->update(this);
}


Layout::Type Container::getLayoutType(const char* layoutString)
{
    if (!layoutString)
    {
        return Layout::LAYOUT_ABSOLUTE;
    }

    std::string layoutName(layoutString);
    std::transform(layoutName.begin(), layoutName.end(), layoutName.begin(), (int(*)(int))toupper);
    if (layoutName == "LAYOUT_ABSOLUTE")
    {
        return Layout::LAYOUT_ABSOLUTE;
    }
    else if (layoutName == "LAYOUT_VERTICAL")
    {
        return Layout::LAYOUT_VERTICAL;
    }
    else if (layoutName == "LAYOUT_FLOW")
    {
        return Layout::LAYOUT_FLOW;
    }
    else
    {
        // Default.
        return Layout::LAYOUT_ABSOLUTE;
    }
}

Layout* Container::createLayout(Layout::Type type)
{
    switch (type)
    {
    case Layout::LAYOUT_ABSOLUTE:
        return AbsoluteLayout::create();
    case Layout::LAYOUT_FLOW:
        return FlowLayout::create();
    case Layout::LAYOUT_VERTICAL:
        return VerticalLayout::create();
    default:
        return AbsoluteLayout::create();
    }
}


////////////////////////////////////////////////////////////////////////////////////////
// Scroll
////////////////////////////////////////////////////////////////////////////////////////


ScrollContainer::ScrollContainer()
    : _scrollBarVertical(NULL),
    _scrollBarHorizontal(NULL),
    _scroll(SCROLL_NONE), _scrollBarBounds(Rectangle::empty()), _scrollPosition(Vector2::zero()),
    _scrollBarsAutoHide(false), _scrollBarOpacity(1.0f), _scrolling(false),
    _scrollingVeryFirstX(0), _scrollingVeryFirstY(0), _scrollingFirstX(0), _scrollingFirstY(0), _scrollingLastX(0), _scrollingLastY(0),
    _scrollingStartTimeX(0), _scrollingStartTimeY(0), _scrollingLastTime(0),
    _scrollingVelocity(Vector2::zero()), _scrollingFriction(1.0f), _scrollWheelSpeed(400.0f),
    _scrollingRight(false), _scrollingDown(false),
    _scrollingMouseVertically(false), _scrollingMouseHorizontally(false),
    _scrollBarOpacityClip(NULL),
    _lastFrameTime(0), _totalWidth(0), _totalHeight(0),
    _initializedWithScroll(false), _scrollWheelRequiresFocus(false)
{
}

ScrollContainer::~ScrollContainer()
{

}

UPtr<ScrollContainer> ScrollContainer::create(const char* id, Style* style, Layout::Type layout)
{
    ScrollContainer* container = new ScrollContainer();
    container->_id = id ? id : "";
    container->_layout = createLayout(layout);
    container->initialize("ScrollContainer", style, NULL);
    return UPtr<ScrollContainer>(container);
}

Control* ScrollContainer::create(Style* style, Properties* properties)
{
    ScrollContainer* container = new ScrollContainer();
    container->initialize("ScrollContainer", style, properties);
    return container;
}

const char* ScrollContainer::getTypeName() const
{
    return "ScrollContainer";
}

void ScrollContainer::initialize(const char* typeName, Style* style, Properties* properties)
{
    Container::initialize(typeName, style, properties);

    if (properties)
    {

        setScroll(getScroll(properties->getString("scroll")));
        _scrollBarsAutoHide = properties->getBool("scrollBarsAutoHide");
        if (_scrollBarsAutoHide)
        {
            _scrollBarOpacity = 0.0f;
        }

        _scrollWheelRequiresFocus = properties->getBool("scrollWheelRequiresFocus");
        if (properties->exists("scrollingFriction"))
            _scrollingFriction = properties->getFloat("scrollingFriction");
        if (properties->exists("scrollWheelSpeed"))
            _scrollWheelSpeed = properties->getFloat("scrollWheelSpeed");
    }
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

    // Update scroll position and scrollbars after updating absolute bounds since
    // computation relies on up-to-date absolute bounds information.
    updateScroll();
}


bool ScrollContainer::updateChildBounds()
{
    bool result = false;

    for (size_t i = 0, count = _controls.size(); i < count; ++i)
    {
        Control* ctrl = _controls[i];
        GP_ASSERT(ctrl);

        if (ctrl->isVisible())
        {
            bool changed = ctrl->updateLayout(_scrollPosition);

            // If the child bounds have changed, dirty our bounds and all of our
            // parent bounds so that our layout and/or bounds are recomputed.
            if (changed)
            {
                Control* parent = this;
                while (parent && (parent->_autoSize != AUTO_SIZE_NONE || static_cast<Container*>(parent)->getLayout()->getType() != Layout::LAYOUT_ABSOLUTE))
                {
                    parent->setDirty(DIRTY_BOUNDS);
                    parent = parent->_parent;
                }
            }

            result = result || changed;
        }
    }

    return result;
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

            Rectangle bounds(_viewportBounds.right() + (_absoluteBounds.right() - _viewportBounds.right()) * 0.5f - topRegion.width * 0.5f, _viewportBounds.y + _scrollBarBounds.y, topRegion.width, topRegion.height);
            batch->drawImage(bounds, topRegion, topColor, &clipRegion);

            bounds.y += topRegion.height;
            bounds.height = _scrollBarBounds.height - topRegion.height - bottomRegion.height;
            batch->drawImage(bounds, verticalRegion, verticalColor, &clipRegion);

            bounds.y += bounds.height;
            bounds.height = bottomRegion.height;
            batch->drawImage(bounds, bottomRegion, bottomColor, &clipRegion);

            drawCalls += 3;
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
        _lastFrameTime = System::currentTimeMillis();
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
        _lastFrameTime = System::currentTimeMillis();
    }
    double frameTime = System::currentTimeMillis();
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
    float clipWidth = _absoluteBounds.width - containerPadding.left - containerPadding.right - vWidth;
    float clipHeight = _absoluteBounds.height - containerPadding.top - containerPadding.bottom - hHeight;

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
        float to = 0;
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
            double gameTime = System::currentTimeMillis();

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
            double gameTime = System::currentTimeMillis();
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
                float localVpRight = _bounds.width - rightPadding;
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
                float localVpBottom = _bounds.height - bottomPadding;
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
                _lastFrameTime = System::currentTimeMillis();
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


ScrollContainer::Scroll ScrollContainer::getScroll(const char* scroll)
{
    if (!scroll)
        return ScrollContainer::SCROLL_NONE;

    if (strcmp(scroll, "SCROLL_NONE") == 0)
    {
        return ScrollContainer::SCROLL_NONE;
    }
    else if (strcmp(scroll, "SCROLL_HORIZONTAL") == 0)
    {
        return ScrollContainer::SCROLL_HORIZONTAL;
    }
    else if (strcmp(scroll, "SCROLL_VERTICAL") == 0)
    {
        return ScrollContainer::SCROLL_VERTICAL;
    }
    else if (strcmp(scroll, "SCROLL_BOTH") == 0)
    {
        return ScrollContainer::SCROLL_BOTH;
    }
    else
    {
        GP_ERROR("Failed to get corresponding scroll state for unsupported value '%s'.", scroll);
    }

    return ScrollContainer::SCROLL_NONE;
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

static bool sortControlsByZOrder(Control* c1, Control* c2)
{
    if (c1->getZIndex() < c2->getZIndex())
        return true;

    return false;
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

}
