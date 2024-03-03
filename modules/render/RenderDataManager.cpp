/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "RenderDataManager.h"
//#include "objects/CubeMap.h"
#include <algorithm>
#include <float.h>


using namespace mgp;

RenderDataManager::RenderDataManager(): _viewFrustumCulling(true), _useInstanced(true) {

}

void RenderDataManager::fill(Scene* scene, Camera *camera, Rectangle *viewport, bool viewFrustumCulling) {
    _camera = camera;
    _viewFrustumCulling = viewFrustumCulling;
    _renderInfo.camera = camera;
    _renderInfo.viewport = *viewport;
    
    clear();
    
    // Visit all the nodes in the scene for drawing
    scene->visit(this, &RenderDataManager::buildRenderQueues);

    endFill();
}

void RenderDataManager::clear() {
    _renderInfo._drawList.clear();

    for (auto it = _instanceds.begin(); it != _instanceds.end(); ++it) {
        Instanced* instance = it->second.get();
        instance->clear();
    }
    _groupByInstance.clear();
    _orderedInstance.clear();

    //clear
    for (unsigned int i = 0; i < Drawable::RenderLayer::Count; ++i)
    {
        auto& queue = _renderQueues[i];
        queue.clear();
    }
    _lights.clear();
}

void RenderDataManager::endFill() {
    for (size_t j = 0, ncount = _renderInfo._drawList.size(); j < ncount; ++j)
    {
        DrawCall* drawCall = &_renderInfo._drawList[j];
        addInstanced(drawCall);
    }

    filterInstanced();

    //init _distanceToCamera
    Vector3 cameraPosition = _camera->getNode()->getTranslationWorld();
    for (unsigned int i = 0; i < Drawable::RenderLayer::Count; ++i)
    {
        auto& queue = _renderQueues[i];
        for (auto it = queue.begin(); it != queue.end(); ++it) {
            if (it->_drawable) {
                it->_distanceToCamera = it->_drawable->getDistance(cameraPosition);
            }
        }
    }
}

void RenderDataManager::setInstanced(Instanced* instance_, std::vector<DrawCall*>& list) {
    int count = 0;
    for (int i = 0; i < list.size(); ++i) {
        DrawCall* drawCall = list[i];
        Drawable* drawable = drawCall->_drawable;
        if (drawable && drawable->getNode()) {
            Matrix worldViewProj;
            Matrix::multiply(_camera->getViewProjectionMatrix(), drawable->getNode()->getWorldMatrix(), &worldViewProj);
            instance_->add(worldViewProj);
            ++count;
        }
        else {
            _renderQueues[drawCall->_renderLayer].emplace_back(*drawCall);
        }
    }

    if (count) {
        instance_->finish();
        DrawCall* drawCall = list[0];

        Material* m = drawCall->_material;
        std::string define = m->getShaderDefines();
        if (define.find("INSTANCED") == std::string::npos) {
            define += ";INSTANCED;NO_MVP";
            m->setShaderDefines(define);
        }

        instance_->setDrawCall(drawCall);
        _renderQueues[drawCall->_renderLayer].emplace_back(*drawCall);
    }
}

void RenderDataManager::filterInstanced() {
    for (auto it = _orderedInstance.begin(); it != _orderedInstance.end(); ++it) {
        const InstanceKey& key = *it;
        std::vector<DrawCall*>& list = _groupByInstance[key];
        if (list.size() > 1) {
            if (_useInstanced) {
                auto found = _instanceds.find(key);
                Instanced* instance_;
                if (found == _instanceds.end()) {
                    UPtr<Instanced> instanced(new Instanced());
                    instance_ = instanced.get();
                    _instanceds[key] = std::move(instanced);
                }
                else {
                    instance_ = found->second.get();
                    //instance_->clear();
                }

                setInstanced(instance_, list);
            }
            else {
                for (int i = 0; i < list.size(); ++i) {
                    DrawCall* drawCall = list[i];
                    _renderQueues[drawCall->_renderLayer].emplace_back(*drawCall);
                }
            }
        }
        else {
            DrawCall* drawCall = list[0];
            _renderQueues[drawCall->_renderLayer].emplace_back(*drawCall);
        }
    }
}

void RenderDataManager::addInstanced(DrawCall* drawCall) {
    InstanceKey key = {
        drawCall->_mesh, drawCall->_material
    };
    if (_groupByInstance.find(key) == _groupByInstance.end()) {
        _orderedInstance.push_back(key);
    }
    _groupByInstance[key].push_back(drawCall);
}

void RenderDataManager::fillDrawables(std::vector<Drawable*>& drawables, Camera *camera, Rectangle *viewport, bool viewFrustumCulling) {
    _camera = camera;
    _viewFrustumCulling = viewFrustumCulling;

    clear();
    
    // Visit all the nodes in the scene for drawing
    for (Drawable* drawable : drawables) {
        if (drawable && drawable->isVisiable())
        {
            // Perform view-frustum culling for this node
            if (dynamic_cast<Model*>(drawable)) {
                if (_viewFrustumCulling && drawable->getNode() && !drawable->getNode()->getBoundingSphere().intersects(_camera->getFrustum())) {
                    continue;
                }
            }

            drawable->draw(&_renderInfo);
        }
    }

    endFill();
}

bool RenderDataManager::buildRenderQueues(Node* node) {
    Drawable* drawable = node->getDrawable();
    if (drawable && drawable->isVisiable())
    {
        // Perform view-frustum culling for this node
        if (dynamic_cast<Model*>(drawable)) {
            if (_viewFrustumCulling && !node->getBoundingSphere().intersects(_camera->getFrustum())) {
                return true;
            }
        }

        drawable->draw(&_renderInfo);
    }
    Light *light = node->getLight();
    if (light) {
        _lights.push_back(light);
    }
    return true;
}

static uint64_t getMaterialId(const DrawCall* a) {

    uint64_t materialId = 0;
    Material* material = a->_material;
    if (material) {
        ShaderProgram* program = material->getEffect();
        if (program) {
            materialId = *((uint64_t*)program);
        }
    }
    return materialId;
}

static double getDistance(const DrawCall* a) {
    return a->_distanceToCamera;
}

static uint64_t getSortScore(const DrawCall* a) {
    uint64_t aid = getMaterialId(a);
    uint64_t adis = (uint64_t)getDistance(a);
    uint64_t mask = 0xFFFF;
    uint64_t score = ((adis & (~mask))) | (aid & mask);
    return score;
}

void RenderDataManager::sort() {

    std::stable_sort(_renderQueues[Drawable::Qpaque].begin(), _renderQueues[Drawable::Qpaque].end(), [&](const DrawCall& a, const DrawCall& b)
        {
            uint64_t as = getSortScore(&a);
            uint64_t bs = getSortScore(&b);
            return as < bs;
        }
    );
    std::stable_sort(_renderQueues[Drawable::Transparent].begin(), _renderQueues[Drawable::Transparent].end(), [&](const DrawCall& a, const DrawCall& b)
        {
            double as = getDistance(&a);
            double bs = getDistance(&b);
            return bs < as;
        }
    );
}

void RenderDataManager::getRenderData(RenderData* view, Drawable::RenderLayer layer) {
    view->_drawList.insert(view->_drawList.begin(), _renderQueues[layer].begin(), _renderQueues[layer].end());
}