/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "RenderDataManager.h"
//#include "objects/CubeMap.h"
#include "base/StringUtil.h"

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
    for (auto it = _renderQueues.begin(); it != _renderQueues.end(); ++it)
    {
        auto& queue = it->second;
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
    for (auto it = _renderQueues.begin(); it != _renderQueues.end(); ++it)
    {
        auto& queue = it->second;
        for (auto it = queue.begin(); it != queue.end(); ++it) {
            if (it->_drawable) {
                it->_distanceToCamera = it->_drawable->getDistance(cameraPosition);
            }
        }
    }
}

void RenderDataManager::addToQueue(DrawCall* drawCall) {
    _renderQueues[drawCall->_renderLayer].emplace_back(*drawCall);
}

void RenderDataManager::setInstanced(Instanced* instance_, std::vector<DrawCall*>& list) {
    int count = 0;
    for (int i = 0; i < list.size(); ++i) {
        DrawCall* drawCall = list[i];
        Drawable* drawable = drawCall->_drawable;
        if (drawable && drawable->getNode()) {
            Matrix worldViewProj;
            Matrix::multiply(_camera->getViewMatrix(), drawable->getNode()->getWorldMatrix(), &worldViewProj);
            instance_->add(worldViewProj);
            ++count;
        }
        else {
            addToQueue(drawCall);
        }
    }

    if (count) {
        instance_->finish();
        DrawCall* drawCall = list[0];
        instance_->setDrawCall(drawCall);
        addToQueue(drawCall);
    }
}

void RenderDataManager::filterInstanced() {
    for (auto it = _orderedInstance.begin(); it != _orderedInstance.end(); ++it) {
        const InstanceKey& key = *it;
        std::vector<DrawCall*>& list = _groupByInstance[key];
        if (list.size() > 1) {
            bool hasSkin = false;
            if (Model* model = dynamic_cast<Model*>(list[0]->_drawable)) {
                hasSkin = model->getSkin() != NULL;
            }
            if (_useInstanced && !hasSkin) {
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
                    addToQueue(drawCall);
                }
            }
        }
        else {
            DrawCall* drawCall = list[0];
            addToQueue(drawCall);
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
    _renderInfo.camera = camera;
    _renderInfo.viewport = *viewport;
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

void RenderDataManager::getRenderData(RenderData* view, int layer) {
    auto it = _renderQueues.find(layer);
    if (it != _renderQueues.end()) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            view->_drawList.push_back(*it2);
        }
    }
}