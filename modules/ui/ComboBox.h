#ifndef _COMBO_BOX_H_
#define _COMBO_BOX_H_

#include "Container.h"
#include "Button.h"
#include "Theme.h"
#include "base/Properties.h"

#include <vector>
#include <string>

namespace mgp
{

class ComboBox : public Button, public Control::Listener {

    std::vector<std::string> _items;

    int _selectIndex = -1;

    ThemeImage* _image = NULL;

public:

    std::vector<std::string>& getItems() { return _items; }

    static UPtr<ComboBox> create(const char* id, Style* style = NULL);

    const char* getTypeName() const;

    int getSelectIndex() { return _selectIndex; }
    void setSelectIndex(int v, bool fireEvent = true);

    static Control* create(Style* style, Properties* properties = NULL);
protected:
    ComboBox();
    ~ComboBox();
    
    void initialize(const char* typeName, Style* style, Properties* properties);

    void controlEvent(Control::Listener::EventType evt);

    void controlEvent(Control* control, EventType evt);
private:
    ComboBox(const ComboBox& copy) = delete;
};


}


#endif