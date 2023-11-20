#include "BoundingBox.h"

namespace mgp
{

inline BoundingBox& BoundingBox::operator*=(const Matrix& matrix)
{
    transform(matrix);
    return *this;
}

inline const BoundingBox operator*(const Matrix& matrix, const BoundingBox& box)
{
    BoundingBox b(box);
    b.transform(matrix);
    return b;
}

}
