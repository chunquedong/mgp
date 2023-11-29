#ifndef BUTTON_H_
#define BUTTON_H_

#include "Container.h"
#include "Label.h"
#include "Theme.h"
#include "base/Properties.h"

namespace mgp
{

/**
 * Defines a button control. 
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-UI_Forms
 */
class Button : public Label
{
    //friend class Container;
    //friend class Gamepad;
    //friend class ControlFactory;

public:

    /**
     * Creates a new Button.
     *
     * @param id The button ID.
     * @param style The button style (optional).
     *
     * @return The new button.
     * @script{create}
     */
    static UPtr<Button> create(const char* id, Style* style = NULL);

    /**
     * Create a button with a given style and properties.
     *
     * @param style The style to apply to this button.
     * @param properties A properties object containing a definition of the button (optional).
     *
     * @return The new button.
     */
    static Control* create(Style* style, Properties* properties = NULL);
protected:

    /**
     * Constructor.
     */
    Button();

    /**
     * Destructor.
     */
    virtual ~Button();


    /**
     * @see Control::initialize
     */
    void initialize(const char* typeName, Style* style, Properties* properties);

    /**
     * Extends ScriptTarget::getTypeName() to return the type name of this class.
     *
     * Child controls should override this function to return the correct type name.
     *
     * @return The type name of this class: "Button"
     * @see ScriptTarget::getTypeName()
     */
    const char* getTypeName() const;

    /**
     * Gets the data binding index for this control.
     *
     * @return The data binding index for control. 
     */
    const unsigned int getDataBinding() const;

    /**
     * Sets the data binding provider for this control.
     *
     * @param dataBinding The data binding index for control. 
     */
    void setDataBinding(unsigned int dataBinding);

private:

    /**
     * Constructor.
     */
    Button(const Button& copy);

    unsigned int _dataBinding;

};

}

#endif
