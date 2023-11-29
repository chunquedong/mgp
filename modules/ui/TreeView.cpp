#include "TreeView.h"

using namespace mgp;

TreeView::TreeView()
{
    setPadding(8, 8, 8, 8);
}

TreeView::~TreeView()
{

}

UPtr<TreeView> TreeView::create(const char* id, Style* style)
{
    TreeView* cb = new TreeView();
    cb->_id = id ? id : "";
    cb->initialize("TreeView", style, NULL);
    return UPtr<TreeView>(cb);
}

Control* TreeView::create(Style* style, Properties* properties)
{
    TreeView* cb = new TreeView();
    cb->initialize("TreeView", style, properties);
    return cb;
}

void TreeView::initialize(const char* typeName, Style* style, Properties* properties)
{
    ScrollContainer::initialize(typeName, style, properties);

    if (properties)
    {
    }
}

const char* TreeView::getTypeName() const
{
    return "TreeView";
}