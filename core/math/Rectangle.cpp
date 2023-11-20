#include "base/Base.h"
#include "Rectangle.h"

namespace mgp
{

Rectangle::Rectangle()
    : x(0), y(0), width(0), height(0)
{
}

Rectangle::Rectangle(Float width, Float height) :
    x(0), y(0), width(width), height(height)
{
}

Rectangle::Rectangle(Float x, Float y, Float width, Float height) :
    x(x), y(y), width(width), height(height)
{
}

Rectangle::Rectangle(const Rectangle& copy)
{
    set(copy);
}

Rectangle::~Rectangle()
{
}

const Rectangle& Rectangle::empty()
{
    static Rectangle empty;
    return empty;
}

bool Rectangle::isEmpty() const
{
    return (x == 0 && y == 0 && width == 0 && height == 0);
}

void Rectangle::set(const Rectangle& r)
{
    set(r.x, r.y, r.width, r.height);
}

void Rectangle::set(Float x, Float y, Float width, Float height)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
}

void Rectangle::setPosition(Float x, Float y)
{
    this->x = x;
    this->y = y;
}

Float Rectangle::left() const
{
    return x;
}

Float Rectangle::top() const
{
    return y;
}

Float Rectangle::right() const
{
    return x + width;
}

Float Rectangle::bottom() const
{
    return y + height;
}

bool Rectangle::contains(Float x, Float y) const
{
    return (x >= this->x && x <= (this->x + width) && y >= this->y && y <= (this->y + height));
}

bool Rectangle::contains(Float x, Float y, Float width, Float height) const
{
    return (contains(x, y) && contains(x + width, y + height));
}

bool Rectangle::contains(const Rectangle& r) const
{
    return contains(r.x, r.y, r.width, r.height);
}

bool Rectangle::intersects(Float x, Float y, Float width, Float height) const
{
    Float t;
    if ((t = x - this->x) > this->width || -t > width)
        return false;
    if ((t = y - this->y) > this->height || -t > height)
        return false;
    return true;
}

bool Rectangle::intersects(const Rectangle& r) const
{
    return intersects(r.x, r.y, r.width, r.height);
}

bool Rectangle::intersect(const Rectangle& r1, const Rectangle& r2, Rectangle* dst)
{
    GP_ASSERT(dst);

    Float xmin = max(r1.x, r2.x);
    Float xmax = min(r1.right(), r2.right());
    if (xmax > xmin)
    {
        Float ymin = max(r1.y, r2.y);
        Float ymax = min(r1.bottom(), r2.bottom());
        if (ymax > ymin)
        {
            dst->set(xmin, ymin, xmax - xmin, ymax - ymin);
            return true;
        }
    }

    dst->set(0, 0, 0, 0);
    return false;
}

void Rectangle::combine(const Rectangle& r1, const Rectangle& r2, Rectangle* dst)
{
    GP_ASSERT(dst);

    dst->x = min(r1.x, r2.x);
    dst->y = min(r1.y, r2.y);
    dst->width = max(r1.x + r1.width, r2.x + r2.width) - dst->x;
    dst->height = max(r1.y + r1.height, r2.y + r2.height) - dst->y;
}

void Rectangle::inflate(Float horizontalAmount, Float verticalAmount)
{
    x -= horizontalAmount;
    y -= verticalAmount;
    width += horizontalAmount * 2;
    height += verticalAmount * 2;
}

Rectangle& Rectangle::operator = (const Rectangle& r)
{
    x = r.x;
    y = r.y;
    width = r.width;
    height = r.height;
    return *this;
}

bool Rectangle::operator == (const Rectangle& r) const
{
    return (x == r.x && width == r.width && y == r.y && height == r.height);
}

bool Rectangle::operator != (const Rectangle& r) const
{
    return (x != r.x || width != r.width || y != r.y || height != r.height);
}

}
