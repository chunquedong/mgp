/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "Shadow.h"
#include "scene/Renderer.h"
#include "scene/Scene.h"
#include "RenderDataManager.h"
#include "material/Material.h"
#include "RenderPath.h"
#include "scene/Drawable.h"

using namespace mgp;

Shadow::Shadow(): _material(NULL), _cascadeCount(2), _cascadeTextureSize(1024) {
    _material = Material::create("res/shaders/depth.vert", "res/shaders/null.frag").take();
    //_material = Material::create("res/shaders/min.vert", "res/shaders/min.frag");
    //_material->getParameter("u_diffuseColor")->setVector4(Vector4(1.0, 0.0, 0.0, 1.0));
    _material->getStateBlock()->setDepthTest(true);
}

Shadow::~Shadow() {
    _cascades.clear();
    SAFE_RELEASE(_frameBuffer);
    SAFE_RELEASE(_material);
}

static void getLightSpaceMatrix(const Matrix &invertView, const Vector3& lightDir, 
    float fov, float ratio, const float nearPlane, const float farPlane, 
    Matrix& lightViewRotate, Matrix &lightProjection)
{
    Matrix proj;
    Matrix::createPerspective(fov, ratio, nearPlane, farPlane, &proj);
    Frustum frustm(proj);
    Vector3 corners[8];
    frustm.getCorners(corners);

    Vector3 center;
    for (Vector3& v : corners) {
        invertView.transformPoint(&v);
        center += v;
    }
    center.x /= 8;
    center.y /= 8;
    center.z /= 8;

    Matrix lightView;
    Matrix::createLookAt(center + lightDir, center, Vector3(0.0f, 1.0f, 0.0f), &lightView, true);

    Float minX = std::numeric_limits<Float>::max();
    Float maxX = std::numeric_limits<Float>::lowest();
    Float minY = std::numeric_limits<Float>::max();
    Float maxY = std::numeric_limits<Float>::lowest();
    Float minZ = std::numeric_limits<Float>::max();
    Float maxZ = std::numeric_limits<Float>::lowest();
    for (const auto& v : corners)
    {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Tune this parameter according to the scene
    Float zMult = 1.2f;
    Float zWidth = (maxZ - minZ) * zMult;
    Float avgZ = (maxZ + minZ) / 2.0;
    minZ = avgZ - zWidth / 2.0;
    maxZ = avgZ + zWidth / 2.0;

    //Matrix lightProjection;
    Matrix::createOrthographicOffCenter(minX, maxX, minY, maxY, -maxZ, -minZ, &lightProjection);

    Quaternion rotate;
    lightView.getRotation(&rotate);
    lightViewRotate.rotate(rotate);
}

void Shadow::initCascadeDistance(Camera* curCamera) {
    _cascades.resize(_cascadeCount);
    float disR = 1;
    float len = curCamera->getFarPlane() - curCamera->getNearPlane();
    float near = curCamera->getNearPlane();
    for (int i = _cascadeCount-1; i >= 0; --i) {
        len = len * 0.2;
        float dis = near + len;
        if (i == 0) {
            dis = near;
        }
        _cascades[i].distance = dis;
    }
}

void Shadow::draw(Scene* scene, Renderer* renderer, Matrix& lightView, Matrix& lightProjection, CascadeInfo& cascade, int index) {
    int width = _cascadeTextureSize;
    int height = _cascadeTextureSize;

    UPtr<Camera> camera = Camera::createOrthographic(20, 20, 1, 1, 100);
    Camera* _camera = camera.get();
    camera->setProjectionMatrix(lightProjection);
    UPtr<Node> cameraNode = Node::create("shadowCamera");
    cameraNode->setCamera(std::move(camera));

    Matrix nodeMatrix = lightView;
    nodeMatrix.invert();
    //cameraNode->setMatrix(light->getNode()->getWorldMatrix());
    cameraNode->setMatrix(nodeMatrix);

    Rectangle viewport(0, index*height, width, height);
    renderer->setViewport(viewport.x, viewport.y, viewport.width, viewport.height);

    RenderData view;
    view.camera = cameraNode->getCamera();
    view.viewport = viewport;
    view.wireframe = false;
    view.lights = NULL;
    //view._renderer = renderer;

    RenderDataManager renderQueue;
    renderQueue.fill(scene, _camera, &viewport);

    view._overridedMaterial = _material;
    view.isDepthPass = true;
    renderQueue.getRenderData(&view, Drawable::RenderLayer::Qpaque);

    for (int i = 0; i < view._drawList.size(); ++i) {
        DrawCall* drawCall = &view._drawList[i];
        drawCall->_material = _material;
        drawCall->_wireframe = false;
        int instanced = drawCall->_instanceCount > 0 ? 1 : 0;
        drawCall->_material->setParams(view.lights, view.camera, &view.viewport, drawCall->_drawable, instanced);
        renderer->draw(drawCall);
    }

    cascade.lightSpaceMatrix = _camera->getViewProjectionMatrix();
}

void Shadow::update(Scene* scene, Renderer *renderer, Light* light, Camera* curCamera) {
    initCascadeDistance(curCamera);

    int width = _cascadeTextureSize;
    int height = _cascadeTextureSize;
    if (_frameBuffer == NULL) {
        //SAFE_RELEASE(cascade.frameBuffer);
        //cascade.frameBuffer = renderer->createFrameBuffer("shadow", width, height, Image::RGBA);
        //cascade.frameBuffer->createDepthStencilTarget();
        _frameBuffer = renderer->createFrameBuffer("shadow", width, height * _cascadeCount, Image::DEPTH).take();
        _frameBuffer->disableDrawBuffer();
        _frameBuffer->check();
    }
    FrameBuffer* preFrameBuffer = _frameBuffer->bind();
    renderer->clear(Renderer::CLEAR_DEPTH);

    Vector3 lightDir = light->getNode()->getForwardVectorWorld();
    lightDir = -lightDir;
    const Matrix& inverseView = curCamera->getInverseViewMatrix();
    for (int i = 0; i < _cascadeCount; ++i) {
        float near = _cascades[i].distance;
        float far = (i + 1 < _cascadeCount) ? _cascades[i + 1].distance : curCamera->getFarPlane();

        Matrix lightView;
        Matrix lightProjection;
        getLightSpaceMatrix(inverseView, lightDir, 
            curCamera->getFieldOfView(), curCamera->getAspectRatio(), near, far, 
            lightView, lightProjection);

        draw(scene, renderer, lightView, lightProjection, _cascades[i], i);
    }

    preFrameBuffer->bind();
}
