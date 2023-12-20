#ifndef HORIZONTAL_LAYOUT_H_
#define HORIZONTAL_LAYOUT_H_

#include "Layout.h"
#include "Container.h"

namespace mgp
{

/**
 * Defines a vertical layout.
 *
 * Controls are placed next to one another vertically until
 * the bottom-most edge of the container is reached.
 */
class HorizontalLayout : public Layout
{
    //friend class Form;
    //friend class Container;

public:

    /**
     * Get the type of this Layout.
     *
     * @return Layout::LAYOUT_VERTICAL
     */
    Layout::Type getType();

    /**
     * Returns the vertical spacing between controls in the layout.
     *
     * @return The vertical spacing between controls.
     */
    int getSpacing() const;

    /**
     * Sets the vertical spacing to add between controls in the layout.
     *
     * @param spacing The vertical spacing between controls.
     */
    void setSpacing(int spacing);

    /**
     * Create a HorizontalLayout.
     *
     * @return a HorizontalLayout object.
     */
    static UPtr<HorizontalLayout> create();
protected:

    /**
     * Constructor.
     */
    HorizontalLayout();

    /**
     * Destructor.
     */
    virtual ~HorizontalLayout();

    /**
     * Update the controls contained by the specified container.
     *
     * Controls are placed next to one another vertically until
     * the bottom-most edge of the container is reached.
     *
     * @param container The container to update.
     */
    void update(const Container* container) override;

    float prefContentWidth(const Container* container) override;

    /**
     * Spacing between controls in the layout.
     */
    int _spacing;

private:

    /**
     * Constructor.
     */
    HorizontalLayout(const HorizontalLayout& copy);


};

}

#endif