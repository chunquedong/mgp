#include "TreeView.h"

using namespace mgp;

TreeView::TreeView()
{
    setPadding(8, 8, 8, 8);
}

TreeView::~TreeView()
{

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