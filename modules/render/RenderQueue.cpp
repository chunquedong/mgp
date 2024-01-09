/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "RenderQueue.h"
//#include "objects/CubeMap.h"
#include <algorithm>
#include <float.h>
#include "objects/Instanced.h"

using namespace mgp;

void RenderQueue::fill(Scene* scene, Camera *camera, Rectangle *viewport, bool viewFrustumCulling) {
    _camera = camera;
    _viewFrustumCulling = viewFrustumCulling;

    for (auto it = _instanceds.begin(); it != _instanceds.end(); ++it) {
        Instanced* instance = dynamic_cast<Instanced*>(it->second->getDrawable());
        instance->clear();
    }

    //clear
    for (unsigned int i = 0; i < Drawable::RenderLayer::Count; ++i)
    {
        std::vector<Drawable*>& queue = _renderQueues[i];
        queue.clear();
    }
    _lights.clear();
    
    // Visit all the nodes in the scene for drawing
    scene->visit(this, &RenderQueue::buildRenderQueues);

    for (auto it = _instanceds.begin(); it != _instanceds.end(); ++it) {
        Instanced* instance = dynamic_cast<Instanced*>(it->second->getDrawable());
        instance->finish();
        std::vector<Drawable*>* queue = &_renderQueues[(int)instance->getRenderPass()];
        queue->push_back(instance);
    }
}

bool RenderQueue::addInstanced(Drawable* drawble) {
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

void RenderQueue::fillDrawables(std::vector<Drawable*>& drawables, Camera *camera, Rectangle *viewport, bool viewFrustumCulling) {
    _camera = camera;
    _viewFrustumCulling = viewFrustumCulling;

    //clear
    for (unsigned int i = 0; i < Drawable::RenderLayer::Count; ++i)
    {
        std::vector<Drawable*>& queue = _renderQueues[i];
        queue.clear();
    }
    _lights.clear();
    
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

            // Determine which render queue to insert the node into
            std::vector<Drawable*>* queue = &_renderQueues[(int)drawable->getRenderPass()];
            queue->push_back(drawable);
        }
    }
}

bool RenderQueue::buildRenderQueues(Node* node) {
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

        // Determine which render queue to insert the node into
        std::vector<Drawable*>* queue = &_renderQueues[(int)drawable->getRenderPass()];
        queue->push_back(drawable);
    }
    Light *light = node->getLight();
    if (light) {
        _lights.push_back(light);
    }
    return true;
}

static uint64_t getMaterialId(Vector3& cameraPosition, Drawable* a) {

    uint64_t materialId = 0;
    Material* material = a->getMainMaterial();
    if (material) {
        ShaderProgram* program = material->getEffect();
        if (program) {
            materialId = *((uint64_t*)program);
        }
    }
    return materialId;
}

static double getDistance(Vector3& cameraPosition, Drawable* a) {
    return a->getDistance(cameraPosition);
}

static uint64_t getSortScore(Vector3& cameraPosition, Drawable* a) {
    uint64_t aid = getMaterialId(cameraPosition, a);
    uint64_t adis = (uint64_t)getDistance(cameraPosition, a);
    uint64_t mask = 0xFFFF;
    uint64_t score = ((adis & (~mask))) | (aid & mask);
    return score;
}

void RenderQueue::sort() {
    Vector3 cameraPosition = _camera->getNode()->getTranslationWorld();
    std::stable_sort(_renderQueues[Drawable::Qpaque].begin(), _renderQueues[Drawable::Qpaque].end(), [&](Drawable* a, Drawable* b)
        {
            uint64_t as = getSortScore(cameraPosition, a);
            uint64_t bs = getSortScore(cameraPosition, b);
            return as < bs;
        }
    );
    std::stable_sort(_renderQueues[Drawable::Transparent].begin(), _renderQueues[Drawable::Transparent].end(), [&](Drawable* a, Drawable* b)
        {
            double as = getDistance(cameraPosition, a);
            double bs = getDistance(cameraPosition, b);
            return bs < as;
        }
    );
}

void RenderQueue::beginDrawScene(RenderInfo* view, Drawable::RenderLayer layer) {
    std::vector<Drawable*>& queue = _renderQueues[layer];
    for (size_t j = 0, ncount = queue.size(); j < ncount; ++j)
    {
        Drawable *drawble = queue[j];
        drawble->draw(view);
    }
}