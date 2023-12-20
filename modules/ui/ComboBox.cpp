#include "ComboBox.h"
#include "base/Base.h"
#include "platform/Toolkit.h"
#include "MenuList.h"


namespace mgp {


ComboBox::ComboBox()
{
    setPadding(8, 8, 8, 8);
    _className = "ComboBox";
}

ComboBox::~ComboBox()
{
}

void ComboBox::onSerialize(Serializer* serializer) {
    Button::onSerialize(serializer);
}

void ComboBox::onDeserialize(Serializer* serializer) {
    Button::onDeserialize(serializer);
    _selectIndex = serializer->readBool("selIndex", -1);
}

void ComboBox::setSelectIndex(int v, bool fireEvent) {
    if (_selectIndex != v) {
        _selectIndex = v;
        if (fireEvent) this->notifyListeners(Listener::SELECT_CHANGE);
        if (_selectIndex >= 0 && _selectIndex < _items.size()) {
            setText(_items[_selectIndex].c_str());
        }
        else {
            setText("");
        }
    }
}

void ComboBox::controlEvent(Control::Listener::EventType evt)
{
    Button::controlEvent(evt);

    switch (evt)
    {
    case Control::Listener::CLICK:
        UPtr<MenuList> list = Control::create<MenuList>((_id + "_menuList").c_str());
        list->initItems(this->_items);
        auto b = this->getAbsoluteBounds();
        list->setPosition(b.x, b.bottom());
        list->addListener(this, Listener::SELECT_CHANGE);
        list->show(this);
        break;
    }
}

void ComboBox::controlEvent(Control* control, EventType evt) {
    MenuList* list = dynamic_cast<MenuList*>(control);
    if (list && evt == Listener::SELECT_CHANGE) {
        setSelectIndex(list->getSelectIndex());
    }
}

}