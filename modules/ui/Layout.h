#ifndef LAYOUT_H_
#define LAYOUT_H_

#include "base/Ref.h"
#include "platform/Mouse.h"
#include "math/Vector2.h"

namespace mgp
{

class Container;
class Control;

/**
 * Defines the layout for containers.
 *
 * Implementations are responsible for positioning, resizing and
 * calling update on all the controls within a container.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-UI_Forms
 */
class Layout : public Refable
{
    //friend class Container;

public:

    /**
     * Layout types available to containers.
     */
    enum Type
    {
        /**
         * Flow layout: Controls are placed next to one another horizontally
         * until the right-most edge of the container is reached, at which point
         * a new row is started.
         */
        LAYOUT_FLOW,

        /**
         * Vertical layout: Controls are placed next to one another vertically until
         * the bottom-most edge of the container is reached.
         */
        LAYOUT_VERTICAL,

        /**
         * Absolute layout: Controls are not modified at all by this layout.
         * They must be positioned and sized manually.
         */
        LAYOUT_ABSOLUTE,

        /**
         * Horizontal layout
         */
        LAYOUT_HORIZONTAL,
    };

    /**
     * Get the type of this layout.
     *
     * @return The type of this layout.
     */
    virtual Type getType() = 0;

    /**
     * Position, resize, and update the controls within a container.
     *
     * @param container The container to update.
     */
    virtual void update(const Container* container) = 0;

    
    virtual float prefContentWidth(const Container* container);
    virtual float prefContentHeight(const Container* container);
};

}

#endif