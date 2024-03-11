/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "RenderPath.h"
#include "math/Vector4.h"
#include "FrameBuffer.h"
#include "Shadow.h"
#include "RenderStage.h"
#include "platform/Toolkit.h"
#include "scene/MeshFactory.h"
#include "PostEffect.h"

using namespace mgp;


Model* RenderPath::_quadModel = NULL;

RenderPath::RenderPath(Renderer* renderer) : _renderer(renderer),
    _frameBuffer(NULL), _previousFrameBuffer(NULL), _width(0), _height(0) {
    _use_ssao = false;
    _use_bloom = false;
    _use_prez = false;
    _use_shadow = false;
    _use_fxaa = false;
    _use_hdr = false;
    _blend = false;
}

RenderPath::~RenderPath()
{
    finalize();
}

Model* RenderPath::fullscreenQuadModel() {
    if (_quadModel == NULL)
    {
        UPtr<Mesh> mesh = MeshFactory::createQuadFullscreen();
        _quadModel = Model::create(std::move(mesh)).take();
        Node* node = Node::create("QuadFullscreen").take();
        node->setDrawable(UPtr<Drawable>(_quadModel));
        //SAFE_RELEASE(mesh);
    }
    return _quadModel;
}

void RenderPath::releaseStatic() {
    if (_quadModel) {
        _quadModel->getNode()->release();
        //SAFE_RELEASE(_quadModel);
        _quadModel = NULL;
    }
}

void RenderPath::updateShadowMap(Scene* scene, Camera* camera) {
    std::map<Light*, Shadow*> curLights;
    for (Light* light : _renderDataManager._lights) {
        if (light->getLightType() != Light::Type::DIRECTIONAL) {
            continue;
        }

        auto itr = _shadowMapCache.find(light);
        Shadow* shadow;
        if (itr == _shadowMapCache.end()) {
            shadow = new Shadow();
            shadow->update(scene, _renderer, light, camera);
        }
        else {
            shadow = itr->second;
            shadow->addRef();

            //TODO
            shadow->update(scene, _renderer, light, camera);
        }

        curLights[light] = shadow;
    }
    
    //clear old cache
    for (auto itr = _shadowMapCache.begin(); itr != _shadowMapCache.end(); ++itr) {
        itr->second->release();
    }
    _shadowMapCache.clear();

    _shadowMapCache.swap(curLights);
}

void RenderPath::clearStages()
{
    for (int i = 0; i < _renderStages.size(); ++i) {
        RenderStage* p = _renderStages[i];
        delete p;
    }
    _renderStages.clear();
}

void RenderPath::clearBuffer() {
    for (auto it = _texturePool.begin(); it != _texturePool.end(); ++it) {
        it->second->release();
    }
    _texturePool.clear();
    for (auto it = _frameBufferPool.begin(); it != _frameBufferPool.end(); ++it) {
        it->second->release();
    }
    _frameBufferPool.clear();
    SAFE_RELEASE(_frameBuffer);
}


void RenderPath::initDeferred() {
    clearStages();
    GBuffer* gbuffer = new GBuffer();
    gbuffer->_renderPath = this;
    _renderStages.push_back(gbuffer);

    LightShading* light = new LightShading();
    light->_renderPath = this;
    _renderStages.push_back(light);

    Redraw* redraw = new Redraw();
    redraw->_renderPath = this;
    _renderStages.push_back(redraw);

    RestStage* p11 = new RestStage();
    p11->_renderPath = this;
    _renderStages.push_back(p11);

    RenderPass* post = new RenderPass();
    post->_renderPath = this;
    post->_inputTextureBuffers["u_texture"] = "main.0";
    post->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/passthrough.frag");
    _renderStages.push_back(post);
}

void RenderPath::addPostProcess() {
    if (_use_ssao) {
        SSAO* ssao = new SSAO(this);
        _renderStages.push_back(ssao);
    }

    if (_use_bloom) {
        Bloom* bloom = new Bloom(this);
        _renderStages.push_back(bloom);
    }
}

void RenderPath::initForward() {
    clearStages();

    ///////////////////////////////////////////////////////////////////
    //pre z pass
    if (_use_prez) {
        RenderPass* p0 = new RenderPass();
        p0->_renderPath = this;
        p0->_drawType = (int)Drawable::RenderLayer::Qpaque;
        p0->_material = Material::create("res/shaders/depth.vert", "res/shaders/null.frag");
        _renderStages.push_back(p0);
    }
    ///////////////////////////////////////////////////////////////////
    // Forward render

    RenderPass* p1 = new RenderPass();
    p1->_renderPath = this;
    p1->_drawType = (int)Drawable::RenderLayer::Qpaque;
    p1->_clearBuffer = _use_prez ? 0 : Renderer::CLEAR_COLOR_DEPTH_STENCIL;
    p1->_depthState = _use_prez ? 1 : 0;
    _renderStages.push_back(p1);

    RestStage* p11 = new RestStage();
    p11->_renderPath = this;
    _renderStages.push_back(p11);

    addPostProcess();

    if (_use_fxaa) {
        RenderPass* post2 = new RenderPass();
        post2->_renderPath = this;
        post2->_clearBuffer = 0;
        post2->_inputTextureBuffers["u_texture"] = "main.0";
        post2->_dstBufferName = "main";
        post2->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/fxaa.frag");
        post2->_material->getStateBlock()->setDepthTest(false);
        _renderStages.push_back(post2);
    }

    if (_use_hdr) {
        RenderPass* post3 = new RenderPass();
        post3->_renderPath = this;
        post3->_clearBuffer = 0;
        post3->_drawToScreen = true;
        post3->_inputTextureBuffers["u_texture"] = "main.0";
        post3->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/hdrToLdr.frag");
        post3->_material->getStateBlock()->setDepthTest(false);
        if (_blend) {
            post3->_material->getStateBlock()->setBlend(true);
        }
        _renderStages.push_back(post3);
    }
    else {
        RenderPass* post3 = new RenderPass();
        post3->_renderPath = this;
        post3->_clearBuffer = 0;
        post3->_drawToScreen = true;
        post3->_inputTextureBuffers["u_texture"] = "main.0";
        post3->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/passthrough.frag");
        post3->_material->getStateBlock()->setDepthTest(false);
        if (_blend) {
            post3->_material->getStateBlock()->setBlend(true);
        }
        _renderStages.push_back(post3);
    }

    RenderPass* p3 = new RenderPass();
    p3->_renderPath = this;
    p3->_drawType = (int)Drawable::RenderLayer::Overlay;
    p3->_clearBuffer = 0;
    p3->_drawToScreen = true;
    p3->_useScreenViewport = true;
    _renderStages.push_back(p3);
}

void RenderPath::resetViewport(Rectangle* viewport) {
    _renderer->setViewport((int)viewport->x, (int)viewport->y, (int)viewport->width, (int)viewport->height);
}

void RenderPath::render(Scene* scene, Camera* camera, Rectangle* viewport) {
    if (_renderStages.size() == 0) {
        initForward();
        for (RenderStage* p : _renderStages) {
            p->onResize(_width, _height);
        }
    }

    GP_ASSERT(scene);
    GP_ASSERT(camera);
    GP_ASSERT(viewport);
    GP_ASSERT(_frameBuffer);

    _renderDataManager.fill(scene, camera, viewport);
    if (_use_shadow) updateShadowMap(scene, camera);

    resetViewport(viewport);
    _renderDataManager.sort();

#if 0
    Shadow* shaow = _shadowMapCache.begin()->second;
    _renderer->clear(Renderer::CLEAR_COLOR_DEPTH_STENCIL);
    debugImage(shaow->getCascade(0).frameBuffer->getRenderTarget(0));
    return;
#endif

    _renderData.camera = camera;
    _renderData.viewport = *viewport;
    _renderData.wireframe = false;
    _renderData.lights = &_renderDataManager._lights;
    //_renderData._renderer = _renderer;
    //_renderData._renderPath = this;

    //_renderer->clear(Renderer::CLEAR_COLOR_DEPTH_STENCIL, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0);
    _previousFrameBuffer = _frameBuffer->bind();

    for (int i = 0; i < _renderStages.size(); ++i) {
        RenderStage* p = _renderStages[i];

        RenderPass* pass = dynamic_cast<RenderPass*>(p);
        if (pass && pass->_drawToScreen && _previousFrameBuffer) {
            _previousFrameBuffer->bind();
        }

        p->render();
    }
}

void RenderPath::renderDrawables(std::vector<Drawable*>& drawables, Camera* camera, Rectangle* viewport) {
    if (_renderStages.size() == 0) {
        initForward();
        for (RenderStage* p : _renderStages) {
            p->onResize(_width, _height);
        }
    }

    GP_ASSERT(camera);
    GP_ASSERT(viewport);
    GP_ASSERT(_frameBuffer);

    _renderDataManager.fillDrawables(drawables, camera, viewport);
    //if (_use_shadow) updateShadowMap(scene, camera);

    resetViewport(viewport);
    _renderDataManager.sort();

    _renderData.camera = camera;
    _renderData.viewport = *viewport;
    _renderData.wireframe = false;
    _renderData.lights = &_renderDataManager._lights;
    //_renderData._renderer = _renderer;
    //_renderData._renderPath = this;

    //_renderer->clear(Renderer::CLEAR_COLOR_DEPTH_STENCIL, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0);
    _previousFrameBuffer = _frameBuffer->bind();

    for (int i = 0; i < _renderStages.size(); ++i) {
        RenderStage* p = _renderStages[i];

        RenderPass* pass = dynamic_cast<RenderPass*>(p);
        if (pass && pass->_drawToScreen && _previousFrameBuffer) {
            _previousFrameBuffer->bind();
        }

        p->render();
    }
}

void RenderPath::createFramebuffer() {
    _frameBuffer = _renderer->createFrameBuffer("main", _width, _height, Texture::RGBA16F).take();
    //_frameBuffer->createDepthStencilTarget();
    Texture* depth = Texture::create(Texture::DEPTH24_STENCIL8, _width, _height, NULL).take();
    _frameBuffer->setRenderTarget(depth, 1);

    _frameBuffer->check();

    addFrameBuffer(_frameBuffer);
    SAFE_RELEASE(depth);
}

void RenderPath::onResize(int w, int h) {
    if (w == 0) w = 1;
    if (h == 0) h = 1;
    if (_width == w && _height == h) {
        return;
    }
    _width = w;
    _height = h;

    clearBuffer();
    createFramebuffer();
    
    for (RenderStage* p : _renderStages) {
        p->onResize(w, h);
    }
}

void RenderPath::finalize() {
    clearStages();
    clearBuffer();
    for (auto itr = _shadowMapCache.begin(); itr != _shadowMapCache.end(); ++itr) {
        itr->second->release();
    }
    _shadowMapCache.clear();
    _renderer = NULL;
}


void RenderPath::bindShadow(std::vector<Light*> *lights, DrawCall* drawCall, Camera* camera) {
    Uniform* uniform = drawCall->_material->getEffect()->getUniform("u_directionalLightShadowMap");
    if (!uniform) return;

    for (int i = 0; i < lights->size(); ++i) {
        Light* light = (*lights)[i];
        if ((light->getLightMask() & drawCall->_drawable->getLightMask()) == 0) {
            continue;
        }
        Shadow* shadow = _shadowMapCache[light];
        char buf[256];

        snprintf(buf, 256, "u_directionalLightShadowMap[%d]", i);
        drawCall->_material->getParameter(buf)->setSampler(shadow->getFrameBuffer()->getRenderTarget(0));

        //for (int j = shadow->getCascadeCount()-1; j >= 0; --j) {
        for (int j = 0; j < shadow->getCascadeCount(); ++j) {
            int pos = i * shadow->getCascadeCount() + j;
            //snprintf(buf, 256, "u_directionalLightShadowMap[%d]", pos);
            //drawCall->_material->getParameter(buf)->setValue(shadow->getCascade(j).frameBuffer->getRenderTarget(0));

            snprintf(buf, 256, "u_directionalLightSpaceMatrix[%d]", pos);
            Matrix worldViewProj;
            Matrix::multiply(shadow->getCascade(j).lightSpaceMatrix, camera->getInverseViewMatrix(), &worldViewProj);
            drawCall->_material->getParameter(buf)->setMatrix(worldViewProj);

            snprintf(buf, 256, "u_directionalLightCascadeDistance[%d]", pos);
            drawCall->_material->getParameter(buf)->setFloat(shadow->getCascade(j).distance);
        }
    }

}

void RenderPath::addFrameBuffer(FrameBuffer* frameBuffer) {
    _frameBufferPool[frameBuffer->getId()] = frameBuffer;
    frameBuffer->addRef();
    for (int i = 0; i < frameBuffer->getRenderTargetCount(); ++i) {
        std::string id = frameBuffer->getId();
        id += "." + std::to_string(i);
        Texture *t = frameBuffer->getRenderTarget(i);
        _texturePool[id] = t;
        t->addRef();
    }
}

FrameBuffer* RenderPath::getFrameBuffer(const std::string& name) {
    auto it = _frameBufferPool.find(name);
    if (it != _frameBufferPool.end()) return it->second;
    return NULL;
}
Texture* RenderPath::getTexture(const std::string& name) {
    auto it = _texturePool.find(name);
    if (it != _texturePool.end()) return it->second;
    return NULL;
}

RenderData* RenderPath::makeRenderDataSet() {
    _renderData._overridedMaterial = NULL;
    _renderData._overridedDepthState = 0;
    _renderData._drawList.clear();
    _renderData.lights = &_renderDataManager._lights;

    return &_renderData;
}

void RenderPath::commitRenderData() {
    RenderData* view = &_renderData;
    for (int i = 0; i < view->_drawList.size(); ++i) {
        DrawCall* drawCall = &view->_drawList[i];
        if (view->_overridedMaterial && drawCall->_instanceCount == 0) {
            drawCall->_material = view->_overridedMaterial;
        }

        StateBlock stateBlock;
        if (view->_overridedDepthState == 1) {
            drawCall->_material->getStateBlock()->cloneInto(&stateBlock);
            drawCall->_material->getStateBlock()->setDepthWrite(false);
            drawCall->_material->getStateBlock()->setDepthFunction(StateBlock::DEPTH_EQUAL);
        }

        drawCall->_wireframe = view->wireframe;

        int instanced = drawCall->_instanceCount > 0 ? 1 : 0;
        drawCall->_material->setParams(view->lights, view->camera, &view->viewport, drawCall->_drawable, instanced);
        if (view->_overridedMaterial == NULL) {
            this->bindShadow(view->lights, drawCall, view->camera);
        }

        _renderer->draw(drawCall);

        if (view->_overridedDepthState == 1) {
            stateBlock.cloneInto(drawCall->_material->getStateBlock());
        }
    }
}

