#include "FormManager.h"
#include "platform/Toolkit.h"
#include "Container.h"

using namespace mgp;


FormManager* __formManager;

FormManager::FormManager() {
    __formManager = this;
}

FormManager* FormManager::cur() {
    return __formManager;
}

FormManager::~FormManager() {
}

void FormManager::finalize() {
    for (int i = 0; i < __forms.size(); ++i) {
        Form* form = __forms[i];
        form->release();
    }
    __forms.clear();
    _focusForm = NULL;
}

void FormManager::add(UPtr<Form> f) {
    __forms.push_back(f.take());
}

unsigned int FormManager::draw(RenderInfo* view) {
    int i = 0;
    for (int i = 0; i < __forms.size(); ++i) {
        Form* form = __forms[i];
        i += form->draw(view);
    }
    return i;
}

void FormManager::updateInternal(float elapsedTime)
{
    //pollGamepads();

    for (size_t i = 0, size = __forms.size(); i < size; ++i)
    {
        Form* form = __forms[i];
        if (form)
        {
            form->update(elapsedTime);
        }
    }
}

void FormManager::resizeEventInternal(unsigned int width, unsigned int height)
{
    for (size_t i = 0, size = __forms.size(); i < size; ++i)
    {
        Form* form = __forms[i];
        if (form)
        {
            // Dirty the form
            form->getRoot()->setDirty(Control::DIRTY_BOUNDS | Control::DIRTY_STATE);
        }
    }
}

bool FormManager::keyEventInternal(Keyboard::KeyEvent evt, int key) {
    if (_focusForm) {
        return _focusForm->keyEventInternal(evt, key);
    }
    return false;
}

bool FormManager::mouseEventInternal(MotionEvent& evt)
{
    // Do not process mouse input when mouse is captured
    if (Toolkit::cur()->isMouseCaptured())
        return false;

    int x = evt.x / Toolkit::cur()->getScreenScale();
    int y = evt.y / Toolkit::cur()->getScreenScale();

    for (size_t i = 0, size = __forms.size(); i < size; ++i)
    {
        Form* form = __forms[i];
        if (form)
        {
            if (form->pointerEventInternal(true, evt.type, x, y, evt.wheelDelta, evt.contactIndex, evt.button)) {
                return true;
            }
        }
    }
    return false;
}

void FormManager::verifyRemovedControlState(Control* control) {
    Form* form = control->getTopLevelForm();
    if (form) {
        form->verifyRemovedControlState(control);
    }
    else if (_focusForm) {
        _focusForm->verifyRemovedControlState(control);
    }
}
