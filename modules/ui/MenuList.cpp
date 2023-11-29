#include "MenuList.h"

using namespace mgp;

MenuList::MenuList()
{
    setPadding(8, 8, 8, 8);
}

MenuList::~MenuList()
{

}

UPtr<MenuList> MenuList::create(const char* id, Style* style)
{
    MenuList* cb = new MenuList();
    cb->_id = id ? id : "";
    cb->initialize("MenuList", style, NULL);
    return UPtr<MenuList>(cb);
}

Control* MenuList::create(Style* style, Properties* properties)
{
    MenuList* cb = new MenuList();
    cb->initialize("MenuList", style, properties);
    return cb;
}

void MenuList::initialize(const char* typeName, Style* style, Properties* properties)
{
    ScrollContainer::initialize(typeName, style, properties);

    if (properties)
    {
    }
}

const char* MenuList::getTypeName() const
{
    return "MenuList";
}

void MenuList::initItems(std::vector<std::string>& items) {
    for (std::string& name : items) {
        UPtr<Label> label = Label::create((this->_id + "_items").c_str());
        this->addControl(std::move(label));
    }
}