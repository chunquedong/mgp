/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef RENDE_DATA_MANAGER_H
#define RENDE_DATA_MANAGER_H

#include "scene/Renderer.h"
#include "scene/Scene.h"
#include "scene/Camera.h"

namespace mgp
{

class RenderData {
public:
    std::vector<DrawCall> _drawList;

    std::vector<Light*>* lights = NULL;
    Camera* camera = NULL;
    Rectangle viewport;

    Material* _overridedMaterial = NULL;
    int _overridedDepthState = 0;

    bool wireframe = false;
    bool isDepthPass = false;
};

class RenderDataManager {
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
    void getRenderData(RenderData* view, Drawable::RenderLayer layer);
protected:
    bool buildRenderQueues(Node* node);
    bool addInstanced(Drawable* draw);
    void clear();
    void endFill();
};
}

#endif