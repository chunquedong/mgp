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

#include "objects/Instanced.h"

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
    bool _useInstanced;
    Camera *_camera = NULL;

    struct InstanceKey {
        void* mesh;
        void* material;
        bool operator<(const InstanceKey& b) const {
            if (this->mesh != b.mesh) {
                return this->mesh < b.mesh;
            }
            return material < b.material;
        }
    };

    std::map<InstanceKey, std::vector<DrawCall*> > _groupByInstance;
    std::vector<InstanceKey> _orderedInstance;
    std::map<InstanceKey, UPtr<Instanced> > _instanceds;
    RenderInfo _renderInfo;

public:
    std::map<int, std::vector<DrawCall> > _renderQueues;
    std::vector<Light*> _lights;

    RenderDataManager();
    
    void fill(Scene* scene, Camera *camera, Rectangle *viewport, bool viewFrustumCulling = true);
    void fillDrawables(std::vector<Drawable*>& drawables, Camera *camera, Rectangle *viewport, bool viewFrustumCulling = true);
    void sort();
    void getRenderData(RenderData* view, int layer);
protected:
    bool buildRenderQueues(Node* node);
    void addInstanced(DrawCall* drawCall);
    void setInstanced(Instanced* instance_, std::vector<DrawCall*>& list);
    void addToQueue(DrawCall* drawCall);
    void filterInstanced();
    void clear();
    void endFill();
};
}

#endif