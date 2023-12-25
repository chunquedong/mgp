#ifndef MODAL_LAYER_H_
#define MODAL_LAYER_H_

#include "ScrollContainer.h"
#include "Label.h"
#include "Theme.h"
#include "base/Properties.h"
#include <vector>

namespace mgp
{

class ModalLayer : public Container {
    friend class Control;

    int _modal = 1;
public:

protected:
    ModalLayer();
    ~ModalLayer();
    
    void controlEvent(Listener::EventType evt);
public:
    unsigned int draw(Form* form, const Rectangle& clip, RenderInfo* view);

    void add(Control* content, int modal);
    void pop();
    void remove(Control* content);
};

}


#endif