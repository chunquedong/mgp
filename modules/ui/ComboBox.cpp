#include "ComboBox.h"
#include "base/Base.h"
#include "platform/Toolkit.h"
#include "MenuList.h"
#include "Form.h"

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
        _selIndex = properties->getBool("selIndex");
    }
}

const char* ComboBox::getTypeName() const
{
    return "ComboBox";
}

void ComboBox::setSelIndex(int v) {
    _selIndex = v;
}

void ComboBox::controlEvent(Control::Listener::EventType evt)
{
    Button::controlEvent(evt);

    switch (evt)
    {
    case Control::Listener::CLICK:
        UPtr<MenuList> list = MenuList::create((_id + "_menuList").c_str());
        list->initItems(this->_items);
        getParent()->getTopLevelForm()->getRoot()->addControl(std::move(list));
        break;
    }
}

}