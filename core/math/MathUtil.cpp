#include "base/Base.h"
#include "MathUtil.h"

namespace mgp
{

void MathUtil::smooth(Float* x, Float target, Float elapsedTime, Float responseTime)
{
    GP_ASSERT(x);

    if (elapsedTime > 0)
    {
        *x += (target - *x) * elapsedTime / (elapsedTime + responseTime);
    }
}

void MathUtil::smooth(Float* x, Float target, Float elapsedTime, Float riseTime, Float fallTime)
{
    GP_ASSERT(x);
    
    if (elapsedTime > 0)
    {
        Float delta = target - *x;
        *x += delta * elapsedTime / (elapsedTime + (delta > 0 ? riseTime : fallTime));
    }
}

}
