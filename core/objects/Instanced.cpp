#include "Instanced.h"
#include "scene/Renderer.h"

using namespace mgp;

Instanced::Instanced(): _instanceMatrix(NULL), _instanceCount(0), _instanceVbo(0) {

}

Instanced::~Instanced() {
    if (_instanceVbo) {
        Renderer::cur()->deleteBuffer(_instanceVbo);
        _instanceVbo = 0;
    }
}

void Instanced::setModel(UPtr<Drawable> model) {
    _model = std::move(model);
    if (_model.get()) {
        this->setLightMask(_model->getLightMask());
    }
}

void Instanced::setInstanceMatrix(Matrix* data, int count) {
    _instanceMatrix.resize(16 * count);
    _instanceCount = count;
    for (int i = 0; i < count; ++i) {
        data[i].toArray(_instanceMatrix.data() + 16 * i);
    }
    if (!_instanceVbo) {
        _instanceVbo = Renderer::cur()->createBuffer(0);
    }
    Renderer::cur()->setBufferData(_instanceVbo, 0, 0, (const char*)_instanceMatrix.data(), _instanceCount * 16 * sizeof(float), 0);
}

void Instanced::clear() {
    _instanceMatrix.clear();
    _instanceCount = 0;
}
void Instanced::add(const Matrix& matrix) {
    int pos = _instanceMatrix.size();
    _instanceMatrix.resize(pos + 16);
    matrix.toArray(_instanceMatrix.data() + pos);
    ++_instanceCount;
}
void Instanced::finish() {
    if (!_instanceVbo) {
        _instanceVbo = Renderer::cur()->createBuffer(0);
    }
    Renderer::cur()->setBufferData(_instanceVbo, 0, 0, (const char*)_instanceMatrix.data(), _instanceCount * 16 * sizeof(float), 0);
}

void Instanced::setDrawCall(DrawCall* drawCall) {
    if (drawCall->_drawable) {
        this->setLightMask(drawCall->_drawable->getLightMask());
    }
    drawCall->_instanceVbo = _instanceVbo;
    drawCall->_instanceCount = _instanceCount;
    drawCall->_drawable = this;
}

unsigned int Instanced::draw(RenderInfo *view) {
    int pos = view->_drawList.size();

    int res = _model->draw(view);

    for (; pos < view->_drawList.size(); ++pos) {
        DrawCall& drawCall = view->_drawList[pos];
        drawCall._instanceVbo = _instanceVbo;
        drawCall._instanceCount = _instanceCount;
        drawCall._drawable = this;
    }
    return res;
}