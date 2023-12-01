#ifndef TREE_VIEW_H_
#define TREE_VIEW_H_

#include "ScrollContainer.h"
#include "Label.h"
#include "Theme.h"
#include "base/Properties.h"

namespace mgp
{

class TreeView : public ScrollContainer, public Control::Listener {
    friend class Control;
public:

    class TreeItem: public Refable {
        friend class TreeView;
    public:
        std::vector<SPtr<TreeItem> > children;
        std::string name;
        uint64_t id = 0;
        bool _isChecked = true;

        bool isChecked();
        void setChecked(bool v);
        static SPtr<TreeView::TreeItem> create(uint64_t id, const char* name, const std::vector<SPtr<TreeItem> >& cs);
    private:
        bool expanded = false;
        UPtr<Container> _contronl;
        TreeItem* _parent = NULL;
    };

    SPtr<TreeItem> root;

    

private:
    TreeItem* _selectItem = NULL;
    bool _isDirty = true;
    bool _useCheckBox = true;
public:
    const char* getTypeName() const;

    void setCheckbox(bool v);

    TreeItem* getSelectItem() { return _selectItem; }
    void setSelectItem(TreeItem* item);

protected:
    TreeView();
    ~TreeView();
    
    void initialize(const char* typeName, Style* style, Properties* properties);

    void update(float elapsedTime);

    void controlEvent(Control* control, EventType evt);
private:
    void addItemLabel(TreeItem* item, int level);

    TreeItem* findTreeItem(Control* control, TreeItem* item);
};

}


#endif