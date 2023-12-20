#include "TreeView.h"
#include "CheckBox.h"
#include "Icon.h"

using namespace mgp;

SPtr<TreeView::TreeItem> TreeView::TreeItem::create(uint64_t id, const char* name, const std::vector<SPtr<TreeView::TreeItem> >& cs) {
    SPtr<TreeView::TreeItem> item = SPtr<TreeView::TreeItem>(new TreeView::TreeItem());
    item->name = name;
    item->id = id;
    item->children = cs;
    return item;
}

bool TreeView::TreeItem::isChecked() {
    if (_parent) {
        return _parent->isChecked() && _isChecked;
    }
    return _isChecked;
}
void TreeView::TreeItem::setChecked(bool v) {
    if (!v) {
        _isChecked = v;
    }
    else {
        _isChecked = v;
        if (_parent) {
            if (!_parent->isChecked()) {
                for (SPtr<TreeItem>& it : _parent->children) {
                    if (it.get() != this) {
                        it->_isChecked = false;
                    }
                }
            }
            _parent->setChecked(true);
        }
    }
}

TreeView::TreeView()
{
    setPadding(8, 8, 8, 8);
    setScroll(SCROLL_BOTH);
    root = TreeItem::create(0, "name", {});
    root->expanded = true;
    setLayout(Layout::LAYOUT_VERTICAL);
    _className = "TreeView";
}

TreeView::~TreeView()
{

}

void TreeView::onSerialize(Serializer* serializer) {
    ScrollContainer::onSerialize(serializer);
}

void TreeView::onDeserialize(Serializer* serializer) {
    ScrollContainer::onDeserialize(serializer);
}

void TreeView::addItemLabel(TreeItem* item, int level) {
    if (item->_contronl.isNull()) {
        item->_contronl = Control::create<Container>("tree_item");
        item->_contronl->setHeight(25);
        //item->_contronl->setWidth(200);
        //item->_contronl->setLayout(Layout::LAYOUT_HORIZONTAL);

        UPtr<Icon> image = Control::create<Icon>("image");
        image->setImagePath("res/ui/right.png");
        image->setSize(23, 23);
        image->setPadding(4, 4, 4, 4);
        image->addListener(this, Listener::CLICK);
        item->_contronl->addControl(std::move(image));
        
        UPtr<Label> label = Control::create<Label>("treeItemLabel");
        label->addListener(this, Listener::CLICK);
        //label->setWidth(1, true);
        item->_contronl->addControl(std::move(label));

        if (_useCheckBox) {
            UPtr<CheckBox> checkbox = Control::create<CheckBox>("tree_item_checkbox");
            checkbox->setHeight(1, true);
            checkbox->addListener(this, Listener::CLICK);
            item->_contronl->addControl(std::move(checkbox));
        }
    }
    Container* control = item->_contronl.get();
    Icon* icon = dynamic_cast<Icon*>(control->getControl(0));
    if (item->expanded) {
        if (strcmp(icon->getImagePath(), "res/ui/down.png") != 0) {
            icon->setImagePath("res/ui/down.png");
        }
    }
    else {
        if (strcmp(icon->getImagePath(), "res/ui/right.png") != 0) {
            icon->setImagePath("res/ui/right.png");
        }
    }
    if (item->expanded && item->children.size() == 0) {
        icon->setVisible(false);
    }
    else {
        icon->setVisible(true);
    }

    Label* label = dynamic_cast<Label*>(control->getControl(1));
    label->setText(item->name.c_str());

    float baseX = (level - 1) * 20;
    if (_useCheckBox) {
        baseX += 30;
    }
    icon->setX(baseX);
    label->setX(baseX+20);

    if (_useCheckBox) {
        CheckBox* checkbox = dynamic_cast<CheckBox*>(control->getControl(2));
        checkbox->setChecked(item->isChecked());
    }

    this->addControl(uniqueFromInstant(control));
    if (item->expanded) {
        for (SPtr<TreeItem>& it : item->children) {
            it.get()->_parent = item;
            addItemLabel(it.get(), level + 1);
        }
    }
}

void TreeView::setCheckbox(bool v) {
    GP_ASSERT(root->children.size() == 0);
    _useCheckBox = v;
}

void TreeView::update(float elapsedTime) {
    if (_isDirty) {
        _isDirty = false;
        this->clear();
        root->expanded = true;
        for (SPtr<TreeItem>& it : root->children) {
            it.get()->_parent = root.get();
            addItemLabel(it.get(), 1);
        }
        requestLayout();
    }
    ScrollContainer::update(elapsedTime);
}

TreeView::TreeItem* TreeView::findTreeItem(Control* control, TreeItem* item) {
    if (item->_contronl.get() == control) {
        return item;
    }
    if (item->_contronl.get()) {
        for (int i = 0; i < item->_contronl->getControlCount(); ++i) {
            if (item->_contronl->getControl(i) == control) {
                return item;
            }
        }
    }
    if (item->expanded) {
        for (SPtr<TreeItem>& it : item->children) {
            TreeView::TreeItem* item = findTreeItem(control, it.get());
            if (item != NULL) return item;
        }
    }
    return NULL;
}

void TreeView::controlEvent(Control* control, Listener::EventType evt) {
    if (evt == Listener::CLICK) {
        TreeItem* item = findTreeItem(control, root.get());

        setSelectItem(item);

        if (CheckBox* checkbox = dynamic_cast<CheckBox*>(control)) {
            item->setChecked(checkbox->isChecked());
            _isDirty = true;
            notifyListeners(Control::Listener::VALUE_CHANGED);
        }

        if (Icon* checkbox = dynamic_cast<Icon*>(control)) {
            item->expanded = !item->expanded;
            _isDirty = true;
            notifyListeners(Listener::EXPANDED);
        }

    }
}

void TreeView::setSelectItem(TreeItem* item) {
    if (item != _selectItem) {
        if (_selectItem && _selectItem->_contronl.get()) {
            Control* control = _selectItem->_contronl->findControl("treeItemLabel");
            if (control) {
                control->setStyleName("Label");
            }
        }

        _selectItem = item;
        _isDirty = true;
        notifyListeners(Listener::SELECT_CHANGE);

        if (_selectItem && _selectItem->_contronl.get()) {
            Control* control = _selectItem->_contronl->findControl("treeItemLabel");
            if (control) {
                Vector4 color = control->getStyle()->getTextColor();
                color.x *= 0.5;
                color.y *= 1.5;
                control->overrideStyle()->setTextColor(color);
            }
        }
    }
}