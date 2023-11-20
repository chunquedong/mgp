#ifndef COMPONENT_H
#define COMPONENT_H

#include "base/Serializable.h"

namespace mgp
{
class Node;

class Component
{
    friend void doFree(Component* p);
public:
    Component();
protected:
    virtual ~Component();

public:
    /**
     * Sets the node associated with this camera.
     */
    virtual void setNode(Node* node) {}
};

//For UniquePtr<>
inline void doFree(Component* p) {
    Refable* r = dynamic_cast<Refable*>(p);
    if (r) {
        r->release();
    }
    else {
        delete p;
    }
}

}
#endif // COMPONENT_H
