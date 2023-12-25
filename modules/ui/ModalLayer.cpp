#include "ModalLayer.h"
#include "Form.h"

using namespace mgp;

ModalLayer::ModalLayer()
{
    setPadding(0, 0, 0, 0);
    setLayout(Layout::LAYOUT_ABSOLUTE);
    this->setVisible(false);
    _className = "ModalLayer";
}

ModalLayer::~ModalLayer()
{
}

void ModalLayer::controlEvent(Listener::EventType evt) {
    if (evt == Listener::CLICK) {
        if (_modal < 2) {
            pop();
        }
    }
}

unsigned int ModalLayer::draw(Form* form, const Rectangle& clip, RenderInfo* view) {
    form->flushBatch(view);
    return Container::draw(form, clip, view);
}


void ModalLayer::add(Control* content, int modal) {
    GP_ASSERT(content);
    addControl(uniqueFromInstant(content));

    this->setVisible(true);
    _modal = modal;
    setConsumeInputEvents(_modal > 0);
}

void ModalLayer::pop() {
    int n = getControlCount();
    if (n > 0) {
        removeControl(n - 1);
        if (n == 1) {
            this->setVisible(false);
        }
    }
}

void ModalLayer::remove(Control* content) {
    int n = getControlCount();
    if (n > 0) {
        removeControl(content);
        if (n == 1) {
            this->setVisible(false);
        }
    }
}
