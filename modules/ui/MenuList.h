#ifndef MENU_LIST_H_
#define MENU_LIST_H_

#include "ScrollContainer.h"
#include "Label.h"
#include "Theme.h"
#include "base/Properties.h"
#include <vector>

namespace mgp
{

class MenuList : public ScrollContainer, public Control::Listener {
    int _selectIndex = -1;
public:

    static UPtr<MenuList> create(const char* id, Style* style = NULL);

    const char* getTypeName() const;

    static Control* create(Style* style, Properties* properties = NULL);

protected:
    MenuList();
    ~MenuList();
    
    void initialize(const char* typeName, Style* style, Properties* properties);

    void controlEvent(Control* control, EventType evt);
public:
    int getSelectIndex() { return _selectIndex; }
    void initItems(std::vector<std::string>& items);

    void show(Control* any);
    void close();
};

}


#endif