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

void FormManager::remove(Form* form_) {
    for (int i = 0; i < __forms.size(); ++i) {
        Form* form = __forms[i];
        if (form == form_) {
            __forms.erase(__forms.begin() + i);
            break;
        }
    }
    form_->release();
}

unsigned int FormManager::draw(RenderInfo* view) {
    int n = 0;
    for (int i = 0; i < __forms.size(); ++i) {
        Form* form = __forms[i];
        if (!form->isVisiable()) continue;
        n += form->draw(view);
    }
    return n;
}

void FormManager::updateInternal(float elapsedTime)
{
    //pollGamepads();

    for (size_t i = 0, size = __forms.size(); i < size; ++i)
    {
        Form* form = __forms[i];
        if (form)
        {
            if (!form->isVisiable()) continue;
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
            form->getRoot()->requestLayout(true);
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
            if (!form->isVisiable()) continue;
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

#include "base/SerializerManager.h"
#include "ScrollContainer.h"
#include "ui/Label.h"
#include "ui/Button.h"
#include "ui/CheckBox.h"
#include "ui/TextBox.h"
#include "ui/RadioButton.h"
#include "ui/Slider.h"
#include "ui/ImageView.h"
#include "ui/JoystickControl.h"
#include "ui/Layout.h"
#include "ui/AbsoluteLayout.h"
#include "ui/VerticalLayout.h"
#include "ui/FlowLayout.h"
#include "ui/ComboBox.h"
#include "ui/TreeView.h"
#include "ui/ScrollContainer.h"
#include "ui/MenuList.h"
#include "ui/FormManager.h"
#include "ui/Icon.h"
#include "ui/ProgressBar.h"

void FormManager::regiseterSerializer(SerializerManager *mgr) {
    mgr->registerType("mgp::Container", Control::_create<mgp::Container>);
    mgr->registerType("mgp::ScrollContainer", Control::_create<mgp::ScrollContainer>);
    mgr->registerType("mgp::Label", Control::_create<mgp::Label>);
    mgr->registerType("mgp::Button", Control::_create<mgp::Button>);
    mgr->registerType("mgp::CheckBox", Control::_create<mgp::CheckBox>);
    mgr->registerType("mgp::TextBox", Control::_create<mgp::TextBox>);
    mgr->registerType("mgp::RadioButton", Control::_create<mgp::RadioButton>);
    mgr->registerType("mgp::Slider", Control::_create<mgp::Slider>);
    mgr->registerType("mgp::ImageView", Control::_create<mgp::ImageView>);
    mgr->registerType("mgp::JoystickControl", Control::_create<mgp::JoystickControl>);
    mgr->registerType("mgp::ComboBox", Control::_create<mgp::ComboBox>);
    mgr->registerType("mgp::TreeView", Control::_create<mgp::TreeView>);
    mgr->registerType("mgp::MenuList", Control::_create<mgp::MenuList>);
    mgr->registerType("mgp::Icon", Control::_create<mgp::Icon>);
    mgr->registerType("mgp::ProgressBar", Control::_create<mgp::ProgressBar>);
    mgr->registerType("mgp::LoadingView", Control::_create<mgp::LoadingView>);

    mgr->registerEnum("mgp::Control::AutoSize", Control::enumToString, Control::enumParse);
    mgr->registerEnum("mgp::Control::Alignment", Control::enumToString, Control::enumParse);
    mgr->registerEnum("mgp::ScrollContainer::Scroll", ScrollContainer::enumToString, ScrollContainer::enumParse);
    mgr->registerEnum("mgp::Container::Layout", Container::enumToString, Container::enumParse);
    mgr->registerEnum("mgp::TextBox::InputMode", TextBox::enumToString, TextBox::enumParse);
    mgr->registerEnum("mgp::FontLayout::Justify", FontLayout::enumToString, FontLayout::enumParse);
}