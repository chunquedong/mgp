
#ifndef POST_EFFECT_H_
#define POST_EFFECT_H_

#include "render/RenderStage.h"

namespace mgp {

struct Bloom : public RenderStageGroup {
    Bloom(RenderPath* _renderPath);
};

struct SSAO : public RenderStageGroup {
    SSAO(RenderPath* _renderPath);
};

}

#endif //POST_EFFECT_H_