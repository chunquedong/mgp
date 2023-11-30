#include "ComboBox.h"
#include "base/Base.h"
#include "platform/Toolkit.h"
#include "MenuList.h"


namespace mgp {


ComboBox::ComboBox()
{
    setPadding(8, 8, 8, 8);
}

ComboBox::~ComboBox()
{
}

UPtr<ComboBox> ComboBox::create(const char* id, Style* style)
{
    ComboBox* cb = new ComboBox();
    cb->_id = id ? id : "";
    cb->initialize("ComboBox", style, NULL);
    return UPtr<ComboBox>(cb);
}

Control* ComboBox::create(Style* style, Properties* properties)
{
    ComboBox* cb = new ComboBox();
    cb->initialize("ComboBox", style, properties);
    return cb;
}

void ComboBox::initialize(const char* typeName, Style* style, Properties* properties)
{
    Button::initialize(typeName, style, properties);

    if (properties)
    {
        _selectIndex = properties->getBool("selIndex");
    }
}

const char* ComboBox::getTypeName() const
{
    return "ComboBox";
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
        UPtr<MenuList> list = MenuList::create((_id + "_menuList").c_str());
        list->initItems(this->_items);
        auto b = this->getAbsoluteBounds();
        list->setPosition(b.x, b.bottom());
        list->setWidth(150);
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