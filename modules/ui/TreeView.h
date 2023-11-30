#ifndef TREE_VIEW_H_
#define TREE_VIEW_H_

#include "ScrollContainer.h"
#include "Label.h"
#include "Theme.h"
#include "base/Properties.h"

namespace mgp
{

class TreeView : public ScrollContainer {
    friend class Control;
public:

    const char* getTypeName() const;

protected:
    TreeView();
    ~TreeView();
    
    void initialize(const char* typeName, Style* style, Properties* properties);
};

}


#endif