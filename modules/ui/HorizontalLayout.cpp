#include "base/Base.h"
#include "HorizontalLayout.h"

namespace mgp
{

HorizontalLayout::HorizontalLayout() : _spacing(0)
{
}

HorizontalLayout::~HorizontalLayout()
{
}

UPtr<HorizontalLayout> HorizontalLayout::create()
{
    return UPtr<HorizontalLayout>(new HorizontalLayout());
}

Layout::Type HorizontalLayout::getType()
{
    return Layout::LAYOUT_HORIZONTAL;
}

int HorizontalLayout::getSpacing() const
{
    return _spacing;
}

void HorizontalLayout::setSpacing(int spacing)
{
    _spacing = spacing;
}

void HorizontalLayout::update(const Container* container)
{
    GP_ASSERT(container);

    // Need border, padding.
    //Border border = container->getBorder(container->getState());
    //Padding padding = container->getPadding();

    float xPosition = 0;

    const std::vector<Control*>& controls = container->getControls();

    int i, end, iter;
    i = 0;
    end = (int)controls.size();
    iter = 1;

    while (i != end)
    {
        Control* control = controls.at(i);
        GP_ASSERT(control);

        if (control->isVisible())
        {
            const Rectangle& bounds = control->getBounds();
            const Margin& margin = control->getMargin();

            xPosition += margin.left;

            control->setXInternal(xPosition);

            xPosition += bounds.width + margin.right + _spacing;
        }

        i += iter;
    }
}

float HorizontalLayout::prefContentWidth(const Container* container) {
    // Size ourself to tightly fit the height of our children
    float width = 0;
    for (size_t i = 0, count = container->getControls().size(); i < count; ++i)
    {
        Control* ctrl = container->getControls()[i];
        if (ctrl->isVisible() && !ctrl->isWidthPercentage())
        {
            float h = ctrl->getMeasureBufferedWidth();

            width += h + _spacing;
        }
    }
    return width;
}

}