/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "SceneView.h"

using namespace mgp;

SceneView::SceneView() {
}
SceneView::~SceneView() {
    finalize();
}

void SceneView::finalize() {
    _renderPath.clear();
    _scene.clear();
    _camera.clear();
    _cameraCtrl.clear();
}

void SceneView::update(float elapsedTime) {
    if (_scene.get()) _scene->update(elapsedTime);
    if (_cameraCtrl.get()) {
        _cameraCtrl->update(elapsedTime);
    }
}

void SceneView::render() {
    if (!_camera.get() || !_scene.get()) return;
    _scene->setActiveCamera(_camera.get());
    _renderPath->render(_scene.get(), _camera.get(), &_viewport);
}

void SceneView::setViewport(Rectangle* rect) {
    _viewport = *rect;

    int w = rect->width;
    int h = rect->height;
    if (_camera.get()) {
        _camera->setAspectRatio((float)w / (float)h);
    }
    _renderPath->onResize(w, h);
}

void SceneView::setCamera(Camera* c, bool initCameraCtrl) {
    _camera = uniqueFromInstant(c);
    if (!_cameraCtrl.get() && initCameraCtrl) {
        UPtr<EditorCameraCtrl> cameraCtrl(new EditorCameraCtrl());
        cameraCtrl->setCamera(_camera.get());
        cameraCtrl->sceneView = this;
        setCameraCtrl(cameraCtrl.dynamicCastTo<CameraCtrl>());
    }
}

void SceneView::setCameraCtrl(UPtr<CameraCtrl> c) {
    _cameraCtrl = std::move(c);
}

void SceneView::initCamera(bool firstPerson, float nearPlane, float farPlane, float fov)
{
    float aspectRatio = 1;
    if (_viewport.height > 0) {
        aspectRatio = _viewport.width / _viewport.height;
    }

    UPtr<Camera> camera = Camera::createPerspective(fov, aspectRatio, nearPlane, farPlane);

    if (firstPerson) {
        UPtr<FPCameraCtrl> cameraCtrl(new FPCameraCtrl());
        cameraCtrl->setCamera(camera.get());
        cameraCtrl->setPosition(Vector3(0, 0, 10));

        //_fpCamera.rotate(0.0f, -MATH_DEG_TO_RAD(10));
        if (_scene.get()) {
            _scene->addNode(uniqueFromInstant(cameraCtrl->getRootNode()));
            //_scene->setActiveCamera(_fpCamera.getCamera());
        }

        setCameraCtrl(cameraCtrl.dynamicCastTo<CameraCtrl>());
    }
    else {
        UPtr<EditorCameraCtrl> cameraCtrl(new EditorCameraCtrl());
        cameraCtrl->setCamera(camera.get());
        cameraCtrl->sceneView = this;

        if (_scene.get()) {
            Node* cameraNode = _scene->addNode("__camera");
            cameraNode->setSerializable(false);
            // Attach the camera to a node. This determines the position of the camera.
            cameraNode->setCamera(uniqueFromInstant(camera.get()));
            // Make this the active camera of the scene.
            //_scene->setActiveCamera(camera);
            // Move the camera to look at the origin.
            cameraNode->translate(0, 0, 10);
        }

        setCameraCtrl(cameraCtrl.dynamicCastTo<CameraCtrl>());
    }
    _useFirstPersonCamera = firstPerson;
    setCamera(camera.get());
}