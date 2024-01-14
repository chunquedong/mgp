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
#include "objects/Instanced.h"

using namespace mgp;

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
        Instanced* instance = dynamic_cast<Instanced*>(it->second->getDrawable());
        instance->clear();
    }

    //clear
    for (unsigned int i = 0; i < Drawable::RenderLayer::Count; ++i)
    {
        auto& queue = _renderQueues[i];
        queue.clear();
    }
    _lights.clear();
}

void RenderDataManager::endFill() {

    for (auto it = _instanceds.begin(); it != _instanceds.end(); ++it) {
        Instanced* instance = dynamic_cast<Instanced*>(it->second->getDrawable());
        instance->finish();
        instance->draw(&_renderInfo);
    }

    Vector3 cameraPosition = _camera->getNode()->getTranslationWorld();
    for (size_t j = 0, ncount = _renderInfo._drawList.size(); j < ncount; ++j)
    {
        DrawCall* drawCall = &_renderInfo._drawList[j];
        drawCall->_distanceToCamera = drawCall->_drawable->getDistance(cameraPosition);
        _renderQueues[drawCall->_renderLayer].emplace_back(*drawCall);
    }
}

bool RenderDataManager::addInstanced(Drawable* drawble) {
    DrawableGroup* group = dynamic_cast<DrawableGroup*>(drawble);
    if (group) {
        bool rc = false;
        for (UPtr<Drawable>& draw : group->getDrawables()) {
            if (addInstanced(draw.get())) {
                rc = true;
            }
        }
        return rc;
    }

    if (drawble->getInstanceKey() && drawble->getNode()) {
        //Model* model = dynamic_cast<Model*>(drawble);
        //if (!model) return false;
        auto found = _instanceds.find(drawble->getInstanceKey());
        Instanced* instance_;
        if (found == _instanceds.end()) {
            UPtr<Instanced> instanced(new Instanced());
            NodeCloneContext ctx;
            UPtr<Drawable> temp = drawble->clone(ctx);
            //temp->
            instanced->setModel(std::move(temp));
            instance_ = instanced.get();
            instanced->setRenderPass(drawble->getRenderPass());

            UPtr<Node> node = Node::create("instanced");
            node->setDrawable(std::move(instanced));
            _instanceds[drawble->getInstanceKey()] = std::move(node);
        }
        else {
            instance_ = dynamic_cast<Instanced*>(found->second->getDrawable());
        }

        Matrix worldViewProj;
        Matrix::multiply(_camera->getViewProjectionMatrix(), drawble->getNode()->getWorldMatrix(), &worldViewProj);
        instance_->add(worldViewProj);

        return true;
    }
    return false;
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

            if (addInstanced(drawable)) {
                continue;
            }

            drawable->draw(&_renderInfo);
        }
    }
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

        if (addInstanced(drawable)) {
            return true;
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