#include "ModalLayer.h"
#include "Form.h"

using namespace mgp;

ModalLayer::ModalLayer()
{
    setPadding(0, 0, 0, 0);
    setLayout(Layout::LAYOUT_ABSOLUTE);
    this->setVisible(false);
}

ModalLayer::~ModalLayer()
{
}

const char* ModalLayer::getTypeName() const
{
    return "ModalLayer";
}

void ModalLayer::controlEvent(Listener::EventType evt) {
    if (evt == Listener::CLICK) {
        if (!_isModal.back()) {
            pop();
        }
    }
}

unsigned int ModalLayer::draw(Form* form, const Rectangle& clip, RenderInfo* view) {
    form->flushBatch(view);
    return Container::draw(form, clip, view);
}


void ModalLayer::push(Control* content, bool isModal) {
    GP_ASSERT(content);
    addControl(uniqueFromInstant(content));

    this->setVisible(true);
    _isModal.push_back(isModal);
}

void ModalLayer::pop() {
    int n = getControlCount();
    if (n > 0) {
        removeControl(n - 1);
        if (n == 1) {
            this->setVisible(false);
        }
    }
    _isModal.pop_back();
}