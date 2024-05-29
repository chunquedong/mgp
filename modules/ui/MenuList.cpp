#include "MenuList.h"
#include "Form.h"
#include "Button.h"
#include "ModalLayer.h"

using namespace mgp;

MenuList::MenuList()
{
    setPadding(4);
    setLayout(Layout::LAYOUT_VERTICAL);
    setScroll(SCROLL_VERTICAL);
    _className = "MenuList";
    //setHeight(0.8, true);
    setWidth(250);
}

MenuList::~MenuList()
{

}

void MenuList::onSerialize(Serializer* serializer) {
    ScrollContainer::onSerialize(serializer);
    serializer->writeList("items", _items.size());
    for (std::string& it : _items) {
        serializer->writeString(NULL, it.c_str(), "");
    }
    serializer->finishColloction();
}

void MenuList::onDeserialize(Serializer* serializer) {
    ScrollContainer::onDeserialize(serializer);
    int size = serializer->readList("items");
    for (int i = 0; i < size; ++i) {
        std::string value;
        serializer->readString(NULL, value, "");
        _items.push_back(value);
    }
    serializer->finishColloction();
}

void MenuList::controlEvent(Control* control, EventType evt) {
    if (evt == Control::Listener::CLICK) {
        int index = -1;
        for (int i = 0; i < getControlCount(); ++i) {
            Control* c = getControl(i);
            if (control == c) {
                index = i;
                break;
            }
        }

        _selectIndex = index;
        if (index != -1) {
            notifyListeners(Control::Listener::SELECT_CHANGE);
        }

        close();
    }
}

void MenuList::initItems(std::vector<std::string>& items) {
    _items = items;
    for (std::string& name : items) {
        UPtr<Button> label = Control::create<Button>((this->_id + "_items").c_str());
        label->setPadding(4);
        label->setStyleName("MenuItem");
        label->setText(name.c_str());
        label->setWidth(1, Control::AUTO_PERCENT_PARENT);
        label->addListener(this, Control::Listener::CLICK);
        this->addControl(std::move(label));
    }
}

void MenuList::measureSize() {
    ScrollContainer::measureSize();
    if (_parent) {
        float bottom = _parent->getClip().height;
        if (_measureBounds.height > bottom) {
            _measureBounds.height = bottom;
        }

        if (_measureBounds.height + _measureBounds.y > bottom) {
            _measureBounds.y -= _measureBounds.height + _measureBounds.y - bottom;
        }
    }
}

void MenuList::show(Control* any) {
    GP_ASSERT(any);
    any->getTopLevelForm()->getOverlay()->add(this, 1);
}

void MenuList::close() {
    this->getTopLevelForm()->getOverlay()->remove(this);
}
