#include "base/Base.h"
#include "Layout.h"
#include "Control.h"
#include "Container.h"
#include "platform/Toolkit.h"

namespace mgp
{

float Layout::prefContentWidth(const Container* container) {
    // Size ourself to tightly fit the width of our children
    float width = 0;
    for (size_t i = 0, count = container->getControls().size(); i < count; ++i)
    {
        Control* ctrl = container->getControls()[i];
        if (ctrl->isVisible() && !ctrl->isWidthPercentage())
        {
            float w = ctrl->getMeasureBufferedWidth();
            if (width < w)
                width = w;
        }
    }
    return width;
}

float Layout::prefContentHeight(const Container* container) {
    // Size ourself to tightly fit the height of our children
    float height = 0;
    for (size_t i = 0, count = container->getControls().size(); i < count; ++i)
    {
        Control* ctrl = container->getControls()[i];
        if (ctrl->isVisible() && !ctrl->isHeightPercentage())
        {
            float h = ctrl->getMeasureBufferedHeight();
            if (height < h)
                height = h;
        }
    }
    return height;
}
}