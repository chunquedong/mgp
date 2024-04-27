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
    std::vector<std::string> _items;
    int _selectIndex = -1;
public:

protected:
    MenuList();
    ~MenuList();
    
    virtual void onSerialize(Serializer* serializer) override;

    virtual void onDeserialize(Serializer* serializer) override;

    void controlEvent(Control* control, EventType evt) override;

    void measureSize() override;
public:
    int getSelectIndex() { return _selectIndex; }
    void initItems(std::vector<std::string>& items);

    void show(Control* any);
    void close();
};

}


#endif