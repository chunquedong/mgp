#ifndef INSTANCED_H_
#define INSTANCED_H_
#include "scene/Model.h"

namespace mgp
{

/**
* Instancing is a technique where we draw many (equal mesh data) objects at once with a single render call.
*/
class Instanced : public Drawable {
    std::vector<float> _instanceMatrix;
    int _instanceCount;
    BufferHandle _instanceVbo;
    UPtr<Drawable> _model;
public:
    Instanced();
    ~Instanced();

    void setModel(UPtr<Drawable> model);
    Drawable* getModel() { return _model.get(); }

    void setInstanceMatrix(Matrix* data, int count);
    void clear();
    void add(const Matrix& matrix);
    void finish();
    void setDrawCall(DrawCall* drawCall);
    unsigned int draw(RenderInfo *view) override;
};

}

#endif