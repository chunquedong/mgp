#include "ComboBox.h"
#include "base/Base.h"
#include "platform/Toolkit.h"
using namespace mgp;



ComboBox::ComboBox() : _image(NULL)
{
    _canFocus = true;
    setPadding(4, 4, 4, 4);
}

ComboBox::~ComboBox()
{

}

UPtr<ComboBox> ComboBox::create(const char* id, Style* style)
{
    ComboBox* cb = new ComboBox();
    cb->_id = id ? id : "";
    cb->initialize("CheckBox", style, NULL);
    return UPtr<ComboBox>(cb);
}

Control* ComboBox::create(Style* style, Properties* properties)
{
    ComboBox* cb = new ComboBox();
    cb->initialize("CheckBox", style, properties);
    return cb;
}

void ComboBox::initialize(const char* typeName, Style* style, Properties* properties)
{
    Button::initialize(typeName, style, properties);

    if (properties)
    {
        _selIndex = properties->getBool("selIndex");
    }
}

const char* ComboBox::getTypeName() const
{
    return "ComboBox";
}

void ComboBox::setSelIndex(int v) {
    _selIndex = v;
}
