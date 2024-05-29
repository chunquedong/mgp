#include "ComboBox.h"
#include "base/Base.h"
#include "platform/Toolkit.h"
#include "MenuList.h"


namespace mgp {


ComboBox::ComboBox()
{
    setPadding(4, 4, 4, 4);
    _className = "ComboBox";
}

ComboBox::~ComboBox()
{
}

void ComboBox::onSerialize(Serializer* serializer) {
    Button::onSerialize(serializer);
    serializer->writeList("items", _items.size());
    for (std::string& it : _items) {
        serializer->writeString(NULL, it.c_str(), "");
    }
    serializer->finishColloction();
    serializer->writeInt("selectIndex", _selectIndex, -1);
}

void ComboBox::onDeserialize(Serializer* serializer) {
    Button::onDeserialize(serializer);
    int size = serializer->readList("items");
    for (int i = 0; i < size; ++i) {
        std::string value;
        serializer->readString(NULL, value, "");
        _items.push_back(value);
    }
    serializer->finishColloction();
    int selectIndex = serializer->readInt("selectIndex", -1);
    setSelectIndex(selectIndex, false);
}

void ComboBox::setSelectIndex(int v, bool fireEvent) {
    if (_selectIndex != v) {
        _selectIndex = v;
        
        if (_selectIndex >= 0 && _selectIndex < _items.size()) {
            setText(_items[_selectIndex].c_str());
        }
        else {
            setText("");
        }
        if (fireEvent) this->notifyListeners(Control::Listener::SELECT_CHANGE);
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
        list->addListener(this, Control::Listener::SELECT_CHANGE);
        list->show(this);
        break;
    }
}

void ComboBox::controlEvent(Control* control, EventType evt) {
    MenuList* list = dynamic_cast<MenuList*>(control);
    if (list && evt == Control::Listener::SELECT_CHANGE) {
        setSelectIndex(list->getSelectIndex());
    }
}

}
