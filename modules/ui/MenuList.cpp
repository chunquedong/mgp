#include "MenuList.h"
#include "Form.h"
#include "Button.h"

using namespace mgp;

MenuList::MenuList()
{
    setPadding(8, 8, 8, 8);
    setLayout(Layout::LAYOUT_VERTICAL);
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

void MenuList::controlEvent(Control* control, EventType evt) {
    if (evt == Listener::CLICK) {
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
            notifyListeners(Listener::SELECT_CHANGE);
        }

        close();
    }
}

void MenuList::initItems(std::vector<std::string>& items) {
    for (std::string& name : items) {
        UPtr<Label> label = Label::create((this->_id + "_items").c_str());
        label->setText(name.c_str());
        label->setWidth(1, true);
        label->addListener(this, Listener::CLICK);
        this->addControl(std::move(label));
    }
}

void MenuList::show(Control* any) {
    GP_ASSERT(any);
    any->getTopLevelForm()->getOverlay()->addControl(uniqueFromInstant(this));
}

void MenuList::close() {
    this->getTopLevelForm()->getOverlay()->clear();
}