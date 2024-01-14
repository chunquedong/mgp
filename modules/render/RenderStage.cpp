/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "RenderStage.h"
#include "FrameBuffer.h"
#include "RenderPath.h"
#include <random>
#include "platform/Toolkit.h"

using namespace mgp;

void RenderStageGroup::render() {
    for (auto p : _passGroup) {
        p->render();
    }
}
void RenderStageGroup::onResize(int w, int h) {
    for (auto p : _passGroup) {
        p->onResize(w, h);
    }
}
RenderStageGroup::~RenderStageGroup() {
    for (auto p : _passGroup) {
        SAFE_DELETE(p);
    }
    _passGroup.clear();
}

RenderPass::RenderPass()
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::setMaterial(UPtr<Material> material)
{
    _material = std::move(material);
}

void RenderPass::render() {
    FrameBuffer* preBuffer = NULL;
    if (_dstFrameBuffer.get()) {
        preBuffer = _dstFrameBuffer->bind();
    }

    Model* _quadModel = RenderPath::fullscreenQuadModel();

    
    RenderInfo* view = _renderPath->getRenderView();
    if (_material.get()) {
        view->_overridedMaterial = _material.get();
    }
    if (_depthState) {
        view->_overridedDepthState = 1;
    }
    if (!_lightEnabled) {
        view->lights = NULL;
    }

    if (_material.get()) {
        for (auto it = _inputTextureBuffers.begin(); it != _inputTextureBuffers.end(); ++it) {
            Texture* texture = _renderPath->getTexture(it->second);
            GP_ASSERT(texture);
            texture->setWrapMode(Texture::CLAMP, Texture::CLAMP, Texture::CLAMP);
            _material->getParameter(it->first.c_str())->setSampler(texture);
        }
    }

    if (true) {
        if (_dstFrameBuffer.get()) {
            _renderPath->getRenderer()->setViewport(0, 0, _dstFrameBuffer->getWidth(), _dstFrameBuffer->getHeight());
        }
        else if (_useScreenViewport) {
            int vpx = 0;
            int vpy = 0;
            int vpw = Toolkit::cur()->getWidth();
            int vph = Toolkit::cur()->getHeight();
            _renderPath->getRenderer()->setViewport(vpx, vpy, vpw, vph);
        }
        else if (_drawToScreen) {
            int vpx = (int)view->viewport.x;
            int vpy = (int)view->viewport.y;
            int vpw = (int)view->viewport.width;
            int vph = (int)view->viewport.height;
            _renderPath->getRenderer()->setViewport(vpx, vpy, vpw, vph);
        }
        else {
            int vpx = 0;// (int)view->viewport.x;
            int vpy = 0;//(int)view->viewport.y;
            int vpw = (int)view->viewport.width;
            int vph = (int)view->viewport.height;
            _renderPath->getRenderer()->setViewport(vpx, vpy, vpw, vph);
        }
    }
    if (_clearBuffer) {
        _renderPath->getRenderer()->clear((Renderer::ClearFlags)_clearBuffer);
    }

    beforeRender(view);

    if (_drawType == -1) {
        _material->getStateBlock()->setDepthTest(false);
        _quadModel->setMaterial(uniqueFromInstant(_material.get()));
        _quadModel->draw(view);
    }
    else {
        _renderPath->getRenderQueue()->getSceneData(view, (Drawable::RenderLayer)_drawType);
    }

    _renderPath->applyDraw(view);

    afterRender(view);

    if (preBuffer) preBuffer->bind();
}

void RenderPass::onResize(int w, int h) {
    if (_dstBufferName.size() > 0) {
        if (_newDstBufferSize == 0) {
            _dstFrameBuffer = uniqueFromInstant(_renderPath->getFrameBuffer(_dstBufferName.c_str()));
            GP_ASSERT(_dstFrameBuffer.get());
        }
        else {
            //SAFE_RELEASE(_dstFrameBuffer);
            GP_ASSERT(_newDstBufferFormat != Texture::UNKNOWN);
            _dstFrameBuffer = _renderPath->getRenderer()->createFrameBuffer(_dstBufferName.c_str(), 
                    w* _newDstBufferSize, h* _newDstBufferSize, _newDstBufferFormat);
            _dstFrameBuffer->check();
            _renderPath->addFrameBuffer(_dstFrameBuffer.get());
        }
    }
}

void RenderPass::beforeRender(RenderInfo* view) {
}
void RenderPass::afterRender(RenderInfo* view) {
}


void RestStage::render() {
    RenderInfo* view = _renderPath->getRenderView();
    _renderPath->getRenderQueue()->getSceneData(view, Drawable::RenderLayer::Custom);
    _renderPath->getRenderQueue()->getSceneData(view, Drawable::RenderLayer::Transparent);
    //_renderPath->getRenderQueue()->beginDrawScene(view, Drawable::RenderLayer::Overlay);
    _renderPath->applyDraw(view);
}

///////////////////////////////////////////////////////////////////
// Deferred

GBuffer::GBuffer() {
    _material = Material::create("res/shaders/deferred/gbuffer.vert", "res/shaders/deferred/gbuffer.frag");
    _drawType = Drawable::RenderLayer::Qpaque;
    this->_newDstBufferSize = 1.0;
    this->_newDstBufferFormat = Texture::RGBA16F;
    this->_dstBufferName = "gbuffer";
}

void GBuffer::onResize(int w, int h) {

    //SAFE_RELEASE(_dstFrameBuffer);

    _dstFrameBuffer = _renderPath->getRenderer()->createFrameBuffer(_dstBufferName.c_str(),
        w * _newDstBufferSize, h * _newDstBufferSize, _newDstBufferFormat);

    /*RenderTarget* normal = RenderTarget::create("normal", w, h, Texture::RGBA16F);
    _dstBuffer->setRenderTarget(normal, 1);*/

    UPtr<Texture> depth = Texture::create(Texture::DEPTH24_STENCIL8, w, h, NULL);
    _dstFrameBuffer->setRenderTarget(depth.get(), 1);

    _dstFrameBuffer->check();

    _renderPath->addFrameBuffer(_dstFrameBuffer.get());

    //SAFE_RELEASE(depth);
}

LightShading::LightShading()
{
    _material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/deferred/light.frag");
    _drawType = -1;
    _inputTextureBuffers["u_texture"] = "gbuffer.0";
    _newDstBufferSize = 1;
    _newDstBufferFormat = Texture::RGBA16F;
    _dstBufferName = "lbuffer";
}

void LightShading::beforeRender(RenderInfo* view) {
    Model* _quadModel = RenderPath::fullscreenQuadModel();
    _quadModel->setLightMask(1);
}

void LightShading::afterRender(RenderInfo* view) {
    Model* _quadModel = RenderPath::fullscreenQuadModel();
    _quadModel->setLightMask(0);
}

Redraw::Redraw() {
    _drawType = Drawable::RenderLayer::Qpaque;
    _dstBufferName = "main";
    _clearBuffer = Renderer::CLEAR_COLOR;
    _depthState = 1;
}

void Redraw::onResize(int w, int h) {
    RenderPass::onResize(w, h);
    Texture* depth = _renderPath->getTexture("gbuffer.1");
    GP_ASSERT(depth);
    _dstFrameBuffer->setRenderTarget(depth, 1);
    _dstFrameBuffer->check();
}

void Redraw::beforeRender(RenderInfo* view) {
    Texture* texture = _renderPath->getTexture("lbuffer.0");
    std::vector<DrawCall>& queue = _renderPath->getRenderQueue()->_renderQueues[_drawType];
    for (size_t j = 0, ncount = queue.size(); j < ncount; ++j)
    {
        DrawCall* drawble = &queue[j];
        if (drawble->_material) {
            drawble->_material->getParameter("u_lightAcc")->setSampler(texture);
        }
    }
}

