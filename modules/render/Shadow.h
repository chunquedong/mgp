/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef SHADOW_H
#define SHADOW_H

#include "FrameBuffer.h"
#include "scene/Camera.h"
#include "scene/Light.h"

namespace mgp
{
class Scene;
class Renderer;
class Material;

class Shadow : public Refable {

    Material* _material;
    int _cascadeCount;
    int _cascadeTextureSize;
public:
    struct CascadeInfo {
        float distance;
        Matrix lightSpaceMatrix;
    };
private:
    FrameBuffer* _frameBuffer = NULL;
    std::vector<CascadeInfo> _cascades;

public:
    Shadow();
    ~Shadow();
    void update(Scene* scene, Renderer *renderer, Light* light, Camera* curCamera);
    CascadeInfo& getCascade(int i) { return _cascades[i]; }
    int getCascadeCount() { return _cascadeCount; }
    FrameBuffer* getFrameBuffer() { return _frameBuffer; }
private:
    void initCascadeDistance(Camera* curCamera);
    void draw(Scene* scene, Renderer* renderer, Matrix& lightView, Matrix& lightProjection, CascadeInfo& cascade, int index);
};
}

#endif