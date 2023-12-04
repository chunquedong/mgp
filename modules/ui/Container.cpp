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
//#include "ImageControl.h"
#include "FormManager.h"
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

/**
 * Sort function for use with _controls.sort(), based on Z-Order.
 * 
 * @param c1 The first control
 * @param c2 The second control
 * return true if the first controls z index is less than the second.
 */
static bool sortControlsByZOrder(Control* c1, Control* c2)
{
    if (c1->getZIndex() < c2->getZIndex())
        return true;

    return false;
}


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
    _consumeInputEvents = false;
    _className = "Container";
    _layout = createLayout(Layout::LAYOUT_ABSOLUTE);
}

Container::~Container()
{
    std::vector<Control*>::iterator it;
    for (it = _controls.begin(); it < _controls.end(); it++)
    {
        (*it)->_parent = nullptr;
        SAFE_RELEASE((*it));
    }
}

void Container::onSerialize(Serializer* serializer) {
    Control::onSerialize(serializer);
}

void Container::onDeserialize(Serializer* serializer) {
    Control::onDeserialize(serializer);
    std::string layoutStr;
    serializer->readString("layout", layoutStr, "");
    if (layoutStr.size() > 0)
    {
        _layout = createLayout(getLayoutType(layoutStr.c_str()));
        switch (_layout->getType())
        {
        case Layout::LAYOUT_FLOW:
            static_cast<FlowLayout*>(_layout.get())->setSpacing(serializer->readInt("horizontalSpacing", 0), 
                serializer->readInt("verticalSpacing", 0));
            break;
        case Layout::LAYOUT_VERTICAL:
            static_cast<VerticalLayout*>(_layout.get())->setSpacing(serializer->readInt("spacing", 0));
            break;
        }
    }

    int size = serializer->readList("_children");
    for (int i = 0; i < size; ++i) {
        Serializable* control = serializer->readObject(NULL);

        // Add the new control to the form.
        if (control)
        {
            addControl(UPtr<Control>(dynamic_cast<Control*>(control)));
            //control->release();
        }
    }

    // Sort controls by Z-Order.
    sortControls();

    std::string activeControl;
    serializer->readString("activeControl", activeControl, "");
    if (activeControl.size() > 0)
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

Layout* Container::getLayout()
{
    return _layout.get();
}

void Container::setLayout(Layout::Type type)
{
	if (_layout.isNull() || _layout->getType() != type)
	{
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

void Container::clear() {
    for (int i= _controls.size()-1; i>=0; --i)
    {
        removeControl(i);
    }
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

Control* Container::findControl(const char* id)
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
        Control* found = c->findControl(id);
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
    for (int i = container->getControlCount()-1; i >= 0; --i)
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
    GP_ASSERT(_layout.get());
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

UPtr<Layout> Container::createLayout(Layout::Type type)
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

}
