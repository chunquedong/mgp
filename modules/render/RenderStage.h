/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef RENDERSTAGE_H_
#define RENDERSTAGE_H_

#include "base/Base.h"
#include "scene/Renderer.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "scene/Drawable.h"

namespace mgp {

class RenderPath;
class FrameBuffer;
class RenderData;

/**
* Abstract Render Pass
*/
struct RenderStage {
    virtual void render() = 0;
    virtual void onResize(int w, int h) {};
    virtual ~RenderStage() {}
};

struct RenderStageGroup : public RenderStage {
    std::vector<RenderStage*> _passGroup;
    void render() override;
    void onResize(int w, int h) override;
    ~RenderStageGroup() override;
};

/**
* Render pass
*/
struct RenderPass : public RenderStage {
    RenderPath* _renderPath = NULL;
    std::string _dstBufferName;

    /**
    * Override Material to Draw
    */
    UPtr<Material> _material;

    std::map<std::string, std::string> _inputTextureBuffers;
    
    /**
    * clear buffer before render
    */
    int _clearBuffer = Renderer::CLEAR_COLOR_DEPTH_STENCIL;

    /**
    * RenderLayer type. -1 is draw full screen quad.
    */
    int _drawType = -1;

    /**
    * disable depth write and depth funtion to EQUALS
    */
    bool _depthState = 0;

    /**
    * enable light
    */
    bool _lightEnabled = true;

    /**
    * create new frame buffer as dstBuffer
    */
    float _newDstBufferSize = 0;
    Image::Format _newDstBufferFormat = Image::UNKNOWN;

    bool _useScreenViewport = false;
    bool _drawToScreen = false;
protected:
    UPtr<FrameBuffer> _dstFrameBuffer;

public:
    RenderPass();
    virtual ~RenderPass();
    void setMaterial(UPtr<Material> material);

    virtual void render();
    virtual void onResize(int w, int h);

protected:
    virtual void beforeRender(RenderData* view);
    virtual void afterRender(RenderData* view);
};

struct RestStage : public RenderStage {
    RenderPath* _renderPath = NULL;
    virtual void render();
};

struct GBuffer : public RenderPass {
    GBuffer();
    void onResize(int w, int h) override;
};

struct LightShading : public RenderPass {
    LightShading();
    void beforeRender(RenderData* view) override;
    void afterRender(RenderData* view) override;
};

struct Redraw : public RenderPass {
    Redraw();
    void beforeRender(RenderData* view) override;
    void onResize(int w, int h) override;
};


}
#endif