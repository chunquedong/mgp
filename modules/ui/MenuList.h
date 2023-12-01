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
    friend class Control;

    int _selectIndex = -1;
public:

protected:
    MenuList();
    ~MenuList();
    
    virtual void onSerialize(Serializer* serializer);

    virtual void onDeserialize(Serializer* serializer);

    void controlEvent(Control* control, EventType evt);
public:
    int getSelectIndex() { return _selectIndex; }
    void initItems(std::vector<std::string>& items);

    void show(Control* any);
    void close();
};

}


#endif