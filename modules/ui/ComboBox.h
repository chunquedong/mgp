#ifndef COMBO_BOX_H_
#define COMBO_BOX_H_

#include "Container.h"
#include "Button.h"
#include "Theme.h"
#include "base/Properties.h"

namespace mgp
{

class ComboBox : public Button {
    int _selIndex = -1;
    bool _dropdown = false;

    ThemeImage* _image;
public:

    std::vector<std::string> times;

    /**
     * Creates a new CheckBox.
     *
     * @param id The checkbox ID.
     * @param style The checkbox style (optional).
     *
     * @return The new checkbox.
     * @script{create}
     */
    static UPtr<ComboBox> create(const char* id, Style* style = NULL);

    /**
     * Extends ScriptTarget::getTypeName() to return the type name of this class.
     *
     * Child controls should override this function to return the correct type name.
     *
     * @return The type name of this class: "ComboBox"
     * @see ScriptTarget::getTypeName()
     */
    const char* getTypeName() const;

    int getSelIndex() { return _selIndex; }
    void setSelIndex(int v);

protected:
    ComboBox();
    ~ComboBox();
    static Control* create(Style* style, Properties* properties = NULL);
    void initialize(const char* typeName, Style* style, Properties* properties);
private:
    ComboBox(const ComboBox& copy) = delete;
};


}


#endif