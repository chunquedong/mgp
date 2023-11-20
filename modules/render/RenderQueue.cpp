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

using namespace mgp;

void RenderQueue::fill(Scene* scene, Camera *camera, Rectangle *viewport, bool viewFrustumCulling) {
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
    scene->visit(this, &RenderQueue::buildRenderQueues);
}

bool RenderQueue::buildRenderQueues(Node* node) {
    Drawable* drawable = node->getDrawable();
    if (drawable && drawable->isVisiable())
    {
        // Perform view-frustum culling for this node
        if (dynamic_cast<Model*>(drawable)) {
            if (_viewFrustumCulling && !node->getBoundingSphere().intersects(_camera->getFrustum()))
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