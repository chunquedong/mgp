/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef RENDERQUEUE_H
#define RENDERQUEUE_H

#include "scene/Renderer.h"
#include "scene/Scene.h"
#include "scene/Camera.h"

namespace mgp
{

class RenderQueue {
    bool _viewFrustumCulling;
    Camera *_camera;
    std::map<void*, UPtr<Node> > _instanceds;
    RenderInfo _renderInfo;
public:
    std::vector<DrawCall> _renderQueues[Drawable::RenderLayer::Count];
    std::vector<Light*> _lights;
    
    void fill(Scene* scene, Camera *camera, Rectangle *viewport, bool viewFrustumCulling = true);
    void fillDrawables(std::vector<Drawable*>& drawables, Camera *camera, Rectangle *viewport, bool viewFrustumCulling = true);
    void sort();
    void getSceneData(RenderInfo* view, Drawable::RenderLayer layer);
protected:
    bool buildRenderQueues(Node* node);
    bool addInstanced(Drawable* draw);
    void clear();
    void endFill();
};
}

#endif