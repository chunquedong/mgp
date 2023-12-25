#include "base/Base.h"
#include "Form.h"
#include "ModalLayer.h"
#include "AbsoluteLayout.h"
#include "FlowLayout.h"
#include "VerticalLayout.h"
#include "platform/Toolkit.h"
#include "Theme.h"
#include "scene/Scene.h"
#include "scene/Renderer.h"
#include "ScrollContainer.h"
#include "FormManager.h"

// Scroll speed when using a joystick.
static const float GAMEPAD_SCROLL_SPEED = 600.0f;
// Distance a joystick must be pushed in order to trigger focus-change and/or scrolling.
static const float JOYSTICK_THRESHOLD = 0.75f;
// If the DPad or joystick is held down, this is the initial delay in milliseconds between focus changes.
static const float GAMEPAD_FOCUS_REPEAT_DELAY = 300.0f;

// Shaders used for drawing offscreen quad when form is attached to a node
#define FORM_VSH "res/shaders/sprite.vert"
#define FORM_FSH "res/shaders/sprite.frag"

namespace mgp
{


Form::Form() : Drawable(), _batched(true)
{
    //memset(__activeControl, 0, sizeof(__activeControl));
    _root = Control::create<Container>("_form_root");
    _root->_form = this;
}

Form::~Form()
{
    if (FormManager::cur()->_focusForm == this) {
        FormManager::cur()->_focusForm = NULL;
    }

    //SAFE_RELEASE(_root);

    // Remove this Form from the global list.
    std::vector<Form*>::iterator it = std::find(FormManager::cur()->__forms.begin(), FormManager::cur()->__forms.end(), this);
    if (it != FormManager::cur()->__forms.end())
    {
        FormManager::cur()->__forms.erase(it);
    }
}

UPtr<Form> Form::create(const char* url)
{
    Form* form = new Form();

    // Load Form from .form file.
    UPtr<Properties> properties = Properties::create(url);
    if (!properties.get())
    {
        GP_WARN("Failed to load properties file for Form.");
        return UPtr<Form>(NULL);
    }
    // Check if the Properties is valid and has a valid namespace.
    Properties* formProperties = (strlen(properties->getNamespace()) > 0) ? properties.get() : properties->getNextNamespace();
    if (!formProperties || !(strcmpnocase(formProperties->getNamespace(), "form") == 0))
    {
        GP_WARN("Invalid properties file for form: %s", url);
        //SAFE_DELETE(properties);
        return UPtr<Form>(NULL);
    }

    // Load the form's theme style.
    Theme* theme = NULL;
    Style* style = NULL;
    if (formProperties->exists("theme"))
    {
        std::string themeFile;
        if (formProperties->getPath("theme", &themeFile))
        {
            theme = Theme::create(themeFile.c_str()).take();
        }
        if (theme == NULL) {
            GP_WARN("Invalid theme: %s", formProperties->getString("theme"));
        }
    }
    if (!theme)
    {
        theme = Theme::getDefault();
    }

    if (theme)
    {
        // Load the form's style
        const char* styleName = formProperties->getString("style", "Form");
        style = theme->getStyle(styleName);
        if (!style)
            style = theme->getEmptyStyle();
    }

    form->_batched = formProperties->getBool("batchingEnabled", true);

    // Initialize the form and all of its child controls
    form->initialize(style, formProperties);

    // After creation, update our bounds once so code that runs immediately after form
    // creation has access to up-to-date bounds.
    //if (form->_root->updateLayout(Vector2::zero()))
    form->_root->updateLayout(Vector2::zero());

    // Release the theme: its lifetime is controlled by addRef() and release() calls in initialize (above) and ~Control.
    if (theme != Theme::getDefault())
    {
        SAFE_RELEASE(theme);
    }

    //SAFE_DELETE(properties);

    return UPtr<Form>(form);
}

UPtr<Form> Form::create()
{
    Form* form = new Form();
    form->initialize(NULL, NULL);
    return UPtr<Form>(form);
}

void Form::initialize(Style* style, Properties* properties)
{
    _root->setWidth(1, Control::AUTO_PERCENT_PARENT);
    _root->setHeight(1, Control::AUTO_PERCENT_PARENT);
    _root->_consumeInputEvents = false;

    auto content = Control::create<ScrollContainer>("_form_content", style, "Form");
    _content = content.get();

    auto overlay = Control::create<ModalLayer>("_form_overlay");
    _overlay = overlay.get();
    _overlay->setWidth(1, Control::AUTO_PERCENT_PARENT);
    _overlay->setHeight(1, Control::AUTO_PERCENT_PARENT);

    _root->addControl(std::move(content));
    _root->addControl(std::move(overlay));
}

Control* Form::getActiveControl(unsigned int touchPoint)
{
    if (touchPoint >= MotionEvent::MAX_TOUCH_POINTS)
        return NULL;

    return __activeControl[touchPoint].get();
}

Control* Form::getFocusControl()
{
    return __focusControl.get();
}

void Form::clearFocus()
{
    setFocusControl(NULL);
}

Container* Form::getRoot() {
    return _root.get();
}

Container* Form::getContent() {
    return _content;
}
ModalLayer* Form::getOverlay() {
    return _overlay;
}

void Form::update(float elapsedTime)
{
    if (!_root->isEnabled() || !_root->isVisible() )  {
        return;
    }

    _root->update(elapsedTime);

    // Do a two-pass bounds update:
    //  1. First pass updates leaf controls
    //  2. Second pass updates parent controls that depend on child sizes
    //if (_root->updateLayout(Vector2::zero()))
    _root->updateLayout(Vector2::zero());
}

void Form::startBatch(BatchableLayer* batch)
{
    // TODO (note: might want to pass a level number/depth here so that batch draw calls can be sorted correctly, such as all text on top)
    if (!batch->isStarted())
    {
        batch->setProjectionMatrix(_projectionMatrix);
        batch->start();

        if (_batched) {
            int i = 0;
            for (; i < _batches.size(); ++i) {
                if (_batches[i]->zorder > batch->zorder) {
                    _batches.insert(_batches.begin()+i, batch);
                    break;
                }
            }
            if (i == _batches.size()) {
                _batches.push_back(batch);
            }
        }
    }
}

void Form::finishBatch(BatchableLayer* batch, RenderInfo* view)
{
    if (!_batched)
    {
        batch->finish(view);
    }
}

int Form::flushBatch(RenderInfo* view) {
    // Flush all batches that were queued during drawing and then empty the batch list
    if (_batched)
    {
        unsigned int batchCount = _batches.size();
        for (unsigned int i = 0; i < batchCount; ++i)
            _batches[i]->finish(view);
        _batches.clear();
        return batchCount;
    }
    return 0;
}

const Matrix& Form::getProjectionMatrix() const
{
    return  _projectionMatrix;
}

unsigned int Form::draw(RenderInfo* view)
{
    if (!_root->_visible || _root->_absoluteClipBounds.width == 0 || _root->_absoluteClipBounds.height == 0)
        return 0;

    // If we're drawing in 2D (i.e. not attached to a node), we need to clear the depth buffer
    if (_node)
    {
        // Drawing in 3D.
        // Setup a projection matrix for drawing the form via the node's world transform.
        Matrix world(_node->getWorldMatrix());
        world.scale(1, -1, 1);
        world.translate(0, -_root->_absoluteClipBounds.height, 0);
        GP_ASSERT(_node->getScene()->getActiveCamera());
        Matrix::multiply(_node->getScene()->getActiveCamera()->getViewProjectionMatrix(), world, &_projectionMatrix);
    }
    else
    {
        // Drawing in 2D, so we need to clear the depth buffer
        Renderer::cur()->clear(Renderer::CLEAR_DEPTH);

        // Setup an ortho matrix that maps to the current viewport
        int w = Toolkit::cur()->getDpWidth();
        int h = Toolkit::cur()->getDpHeight();
        //printf("Form projectionMatrix:%d,%d,%f\n", w, h, Toolkit::cur()->getScreenScale());
        Matrix::createOrthographicOffCenter(0, w, h, 0, 0, 1, &_projectionMatrix);
    }

    // Draw the form
    unsigned int drawCalls = _root->draw(this, _root->_absoluteClipBounds, view);

    // Flush all batches that were queued during drawing and then empty the batch list
    if (_batched)
    {
        drawCalls = flushBatch(view);
    }
    return drawCalls;
}

UPtr<Drawable> Form::clone(NodeCloneContext& context)
{
    // TODO:
    return UPtr<Drawable>(NULL);
}

bool Form::isBatchingEnabled() const
{
    return _batched;
}

void Form::setBatchingEnabled(bool enabled)
{
    _batched = enabled;
}


bool Form::screenToForm(int* x, int* y)
{
    Form* form = this;
    if (form)
    {
        if (form->_node)
        {
            // Form is attached to a scene node, so project the screen space point into the
            // form's coordinate space (which may be transformed by the node).
            Vector3 point;
            if (form->projectPoint(*x, *y, &point))
            {
                *x = (int)point.x;
                *y = form->_root->_absoluteBounds.height - (int)point.y;
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

Control* Form::findInputControl(int* x, int* y, bool focus, unsigned int contactIndex, bool* consumed)
{
    Form* form = this;
    if (!form || !form->_root->isEnabled() || !form->_root->isVisible())
        return NULL;

    // Convert to local form coordinates
    int formX = *x;
    int formY = *y;
    if (!form->screenToForm(&formX, &formY))
        return NULL;

    // Search for an input control within this form
    Control* ctrl = form->_root->findInputControl(formX, formY, focus, contactIndex);
    if (ctrl)
    {
        *x = formX;
        *y = formY;
        return ctrl;
    }

    // If the form consumes input events and the point intersects the form,
    // don't traverse other forms below it.
    if (form->_root->_consumeInputEvents && form->_root->_absoluteClipBounds.contains(formX, formY))
        *consumed = true;

    return NULL;
}


SPtr<Control> Form::handlePointerPress(int* x, int* y, bool pressed, unsigned int contactIndex, bool* consumed)
{
    if (contactIndex >= MotionEvent::MAX_TOUCH_POINTS)
        return SPtr<Control>();

    SPtr<Control> ctrl;

    // Update active state changes
    if ((ctrl = findInputControl(x, y, false, contactIndex, consumed)).get())
    {
        if (__activeControl[contactIndex] != ctrl || ctrl->_getState() != Control::ACTIVE)
        {
            if (__activeControl[contactIndex].get())
            {
                __activeControl[contactIndex]->setState(Control::NORMAL);
            }

            __activeControl[contactIndex] = ctrl.get();
            ctrl->setState(Control::ACTIVE);
        }

        ctrl->notifyListeners(Control::Listener::PRESS);
    }

    return ctrl;
}

SPtr<Control> Form::handlePointerRelease(int* x, int* y, bool pressed, unsigned int contactIndex, bool* consumed)
{
    if (contactIndex >= MotionEvent::MAX_TOUCH_POINTS)
        return SPtr<Control>();

    SPtr<Control> ctrl;

    Control* active = (__activeControl[contactIndex].get() && __activeControl[contactIndex]->_getState() == Control::ACTIVE) ? __activeControl[contactIndex].get() : NULL;

    if (active)
    {
        //active->addRef(); // protect against event-hanlder evil

        // Release happened for an active control (that was pressed)
        ctrl = active;

        // Transform point to form-space
        screenToForm(x, y);

        // No longer any active control
        active->setState(Control::NORMAL);
        __activeControl[contactIndex] = NULL;

        // Fire release event for the previously active control
        active->notifyListeners(Control::Listener::RELEASE);

        // If the release event was received on the same control that was
        // originally pressed, fire a click event
        if (active->getAbsoluteClipBounds().contains(*x, *y))
        {
            ScrollContainer* parent = dynamic_cast<ScrollContainer*>(active->_parent);
            if (!parent || !parent->isScrolling())
            {
                active->notifyListeners(Control::Listener::CLICK);
            }
        }

        //active->release();
    }
    else
    {
        // Update active and hover control state on release
        Control* inputControl = findInputControl(x, y, false, contactIndex, consumed);
        if (inputControl)
        {
            ctrl = inputControl;

            if (__activeControl[contactIndex] != ctrl || ctrl->_getState() != Control::HOVER)
            {
                if (__activeControl[contactIndex].get())
                {
                    __activeControl[contactIndex]->setState(Control::NORMAL);
                }

                __activeControl[contactIndex] = ctrl;
                ctrl->setState(Control::HOVER);
            }
        }
        else
        {
            // Control no longer active
            if (__activeControl[contactIndex].get())
            {
                __activeControl[contactIndex]->setState(Control::NORMAL);
                __activeControl[contactIndex] = NULL;
            }
        }
    }


    return ctrl;
}

SPtr<Control> Form::handlePointerMove(int* x, int* y, unsigned int contactIndex, bool* consumed)
{
    if (contactIndex >= MotionEvent::MAX_TOUCH_POINTS)
        return SPtr<Control>();

    SPtr<Control> ctrl;

    // Handle hover control changes on move, only if there is no currently active control
    // (i.e. when the mouse or a finger is not down).
    if (__activeControl[contactIndex].get() && __activeControl[contactIndex]->_getState() == Control::ACTIVE)
    {
        // Active controls always continue receiving pointer events, even when the pointer
        // is not on top of the control.
        ctrl = __activeControl[contactIndex];
        screenToForm(x, y);
    }
    else
    {
        ctrl = findInputControl(x, y, false, contactIndex, consumed);
        if (ctrl.get())
        {
            // Update hover control
            if (__activeControl[contactIndex] != ctrl || ctrl->_getState() != Control::HOVER)
            {
                if (__activeControl[contactIndex].get())
                {
                    __activeControl[contactIndex]->setState(Control::NORMAL);
                }

                __activeControl[contactIndex] = ctrl;
                ctrl->setState(Control::HOVER);
            }
        }
        else
        {
            // No active/hover control
            if (__activeControl[contactIndex].get())
            {
                __activeControl[contactIndex]->setState(Control::NORMAL);
                __activeControl[contactIndex] = NULL;
            }
        }
    }

    return ctrl;
}

void Form::verifyRemovedControlState(Control* control)
{
    if (__focusControl.get() == control)
    {
        __focusControl = NULL;
    }

    if (control->_getState() == Control::ACTIVE || control->_getState() == Control::HOVER)
    {
        for (unsigned int i = 0; i < MotionEvent::MAX_TOUCH_POINTS; ++i)
        {
            if (__activeControl[i].get() == control)
            {
                __activeControl[i] = NULL;
            }
        }
        control->setState(Control::NORMAL);
    }
}

bool Form::bubblingTouch(SPtr<Control> ctrl, int formX, int formY, bool mouse, unsigned int contactIndex, int wheelDelta, int evt) {
    // Dispatch the event from the bottom upwards, until a control intersecting the point consumes the event
    while (ctrl.get())
    {
        int localX = formX - ctrl->getAbsoluteBounds().x;
        int localY = formY - ctrl->getAbsoluteBounds().y;
        if (mouse)
        {
            if (ctrl->mouseEvent((MotionEvent::MotionType)evt, localX, localY, wheelDelta))
                return true;

            //// Forward to touch event hanlder if unhandled by mouse handler
            //switch (evt)
            //{
            //case MotionEvent::press:
            //    if (ctrl->touchEvent(MotionEvent::press, localX, localY, 0))
            //        return true;
            //    break;
            //case MotionEvent::release:
            //    if (ctrl->touchEvent(MotionEvent::release, localX, localY, 0))
            //        return true;
            //    break;
            //case MotionEvent::mouseMove:
            //    if (ctrl->touchEvent(MotionEvent::touchMove, localX, localY, 0))
            //        return true;
            //    break;
            //}
        }

        if (ctrl->touchEvent((MotionEvent::MotionType)evt, localX, localY, contactIndex))
            return true;


        // Handle container scrolling
        Control* tmp = ctrl.get();
        while (tmp)
        {
            if (ScrollContainer* container = dynamic_cast<ScrollContainer*>(tmp))
            {
                if (container->_scroll != ScrollContainer::SCROLL_NONE)
                {
                    if (mouse)
                    {
                        if (container->mouseEventScroll((MotionEvent::MotionType)evt, formX - tmp->getAbsoluteBounds().x, formY - tmp->getAbsoluteBounds().y, wheelDelta))
                            return true;
                    }
                    else
                    {
                        if (container->touchEventScroll((MotionEvent::MotionType)evt, formX - tmp->getAbsoluteBounds().x, formY - tmp->getAbsoluteBounds().y, contactIndex))
                            return true;
                    }
                    break; // scrollable parent container found
                }
            }
            tmp = tmp->getParent();
        }

        // Consume all input events anyways?
        if (ctrl->getConsumeInputEvents())
            return true;

        ctrl = ctrl->getParent();
    }
    return false;
}

// Generic pointer event handler that both touch and mouse events map to.
// Mappings:
//   mouse - true for mouse events, false for touch events
//   evt - MotionEvent::MotionType or MotionEvent::MotionType
//   x, y - Point of event
//   param - wheelData for mouse events, contactIndex for touch events
bool Form::pointerEventInternal(bool mouse, int evt, int x, int y, int wheelDelta, unsigned int contactIndex, int button)
{
    // Is this a press event (TOUCH_PRESS has the same value as MOUSE_PRESS_LEFT_BUTTON)
    bool pressEvent = evt == MotionEvent::press;// || (mouse && (evt == Mouse::MOUSE_PRESS_MIDDLE_BUTTON || evt == Mouse::MOUSE_PRESS_RIGHT_BUTTON));

    SPtr<Control> ctrl;
    int formX = x;
    int formY = y;
    //unsigned int contactIndex = mouse ? 0 : param;
    bool consumed = false;

    //update hove state
    // Note: TOUCH_PRESS and TOUCH_RELEASE have same values as MOUSE_PRESS_LEFT_BUTTON and MOUSE_RELEASE_LEFT_BUTTON
    if (evt == MotionEvent::press)
    {
        ctrl = handlePointerPress(&formX, &formY, true, contactIndex, &consumed);
    }
    else if (evt == MotionEvent::release)
    {
        ctrl = handlePointerRelease(&formX, &formY, false, contactIndex, &consumed);
    }
    else if ((mouse && evt == MotionEvent::mouseMove) || (!mouse && evt == MotionEvent::touchMove))
    {
        ctrl = handlePointerMove(&formX, &formY, contactIndex, &consumed);
    }

    // Dispatch input events to all controls that intersect this point
    if (ctrl.isNull())
    {
        formX = x;
        formY = y;
        ctrl = findInputControl(&formX, &formY, false, contactIndex, &consumed);
    }

    if (ctrl.get())
    {
        // Handle setting focus for all press events
        if (pressEvent)
        {
            Control* focusControl = ctrl.get();
            while (focusControl && !focusControl->setFocus())
                focusControl = focusControl->getParent();

            if (focusControl == NULL)
            {
                // Nothing got focus on this press, so remove current focused control
                setFocusControl(NULL);
            }
        }

        //notify __focusControl
        if (__focusControl.get()) {
            int localX = formX - __focusControl->getAbsoluteBounds().x;
            int localY = formY - __focusControl->getAbsoluteBounds().y;
            if (__focusControl->touchEvent((MotionEvent::MotionType)evt, localX, localY, contactIndex))
                return true;
        }

        if (bubblingTouch(ctrl, formX, formY, mouse, contactIndex, wheelDelta, evt)) {
            return true;
        }

        if (ctrl->getConsumeInputEvents()) {
            consumed = true;
        }
    }
    else
    {
        // If this was a press event, remove all focus
        if (pressEvent)
        {
            setFocusControl(NULL);
        }
    }

    return consumed;
}



bool Form::keyEventInternal(Keyboard::KeyEvent evt, int key)
{
    switch (key)
    {
    case Keyboard::KEY_ESCAPE:
        return false; // ignore escape key presses

    case Keyboard::KEY_SHIFT:
        if (evt == Keyboard::KEY_PRESS)
            __shiftKeyDown = true;
        else if (evt == Keyboard::KEY_RELEASE)
            __shiftKeyDown = false;
        break;
    }

    // Handle focus changing
    if (__focusControl.get())
    {
        switch (evt)
        {
        case Keyboard::KEY_CHAR:
            switch (key)
            {
            case Keyboard::KEY_TAB:
                if (__focusControl->getParent())
                {
                    if (__focusControl->getParent()->moveFocus(__shiftKeyDown ? Container::PREVIOUS : Container::NEXT))
                        return true;
                }
                break;
            }
            break;
        }
    }

    // Dispatch key events
    SPtr<Control> ctrl = __focusControl;
    while (ctrl.get())
    {
        if (ctrl->isEnabled() && ctrl->isVisible())
        {
            if (ctrl->keyEvent(evt, key))
                return true;
        }

        ctrl = ctrl->getParent();
    }

    return false;
}


bool Form::projectPoint(int x, int y, Vector3* point)
{
    if (!_node)
        return false;

    Scene* scene = _node->getScene();
    Camera* camera;

    if (scene && (camera = scene->getActiveCamera()))
    {
        // Get info about the form's position.
        Matrix m = _node->getWorldMatrix();
        Vector3 pointOnPlane(0, 0, 0);
        m.transformPoint(&pointOnPlane);

        // Unproject point into world space.
        Ray ray;
        Rectangle vp(0, 0, Toolkit::cur()->getWidth(), Toolkit::cur()->getHeight());
        camera->pickRay(vp, x, y, &ray);

        // Find the quad's plane.  We know its normal is the quad's forward vector.
        Vector3 normal = _node->getForwardVectorWorld().normalize();

        // To get the plane's distance from the origin, we project a point on the
        // plane onto the plane's normal vector.
        const float distance = Vector3::dot(pointOnPlane, normal);
        Plane plane(normal, -distance);

        // Check for collision with plane.
        float collisionDistance = ray.intersectsQuery(plane);
        if (collisionDistance != Ray::INTERSECTS_NONE)
        {
            // Multiply the ray's direction vector by collision distance and add that to the ray's origin.
            point->set(ray.getOrigin() + collisionDistance*ray.getDirection());

            // Project this point into the plane.
            m.invert();
            m.transformPoint(point);

            return true;
        }
    }
    return false;
}

void Form::controlDisabled(Control* control)
{
    if (__focusControl.get() && (__focusControl.get() == control || __focusControl->isChild(control)))
    {
        setFocusControl(NULL);
    }

    if (control->_getState() == Control::ACTIVE || control->_getState() == Control::HOVER)
    {
        for (unsigned int i = 0; i < MotionEvent::MAX_TOUCH_POINTS; ++i)
        {
            if (__activeControl[i].get() == control)
            {
                __activeControl[i] = NULL;
            }
        }
    }
}

void Form::setFocusControl(Control* control)
{
    SPtr<Control> oldFocus = __focusControl;

    __focusControl = control;

    // Deactivate the old focus control
    if (oldFocus.get())
    {
        oldFocus->setDirty(Control::DIRTY_STATE);
        oldFocus->notifyListeners(Control::Listener::FOCUS_LOST);
    }

    // Activate the new focus control
    if (__focusControl.get())
    {
        __focusControl->setDirty(Control::DIRTY_STATE);
        __focusControl->notifyListeners(Control::Listener::FOCUS_GAINED);

        // Set the activeControl property of the control's parent container
        Container* group = NULL;
        if (__focusControl->getParent())
        {
            group = __focusControl->getParent();
            group->_activeControl = __focusControl.get();
        }

        // If this control is inside a scrollable container and is not fully visible,
        // scroll the container so that it is.
        ScrollContainer* parent = dynamic_cast<ScrollContainer*>(group);
        if (parent && parent->_scroll != ScrollContainer::SCROLL_NONE && !parent->_viewportBounds.isEmpty())
        {
            const Rectangle& bounds = __focusControl->getBounds();
            if (bounds.x < parent->_scrollPosition.x)
            {
                // Control is to the left of the scrolled viewport.
                parent->_scrollPosition.x = -bounds.x;
            }
            else if (bounds.x > parent->_scrollPosition.x && bounds.x + bounds.width > parent->_scrollPosition.x + parent->getWidth())
            {
                // Control is off to the right.
                parent->_scrollPosition.x = -(bounds.x + bounds.width - parent->getWidth());
            }

            if (bounds.y < parent->getPadding().top - parent->_scrollPosition.y)
            {
                // Control is above the viewport.
                parent->_scrollPosition.y = -bounds.y;
            }
            else if (bounds.y > parent->getPadding().top - parent->_scrollPosition.y && bounds.y + bounds.height > parent->getHeight() - parent->_scrollPosition.y)
            {
                // Control is below the viewport.
                parent->_scrollPosition.y = -(bounds.y + bounds.height - parent->getHeight());
            }
        }
    }

    if (control == NULL) {
        FormManager::cur()->_focusForm = NULL;
    }
    else {
        FormManager::cur()->_focusForm = this;
    }
}


}
