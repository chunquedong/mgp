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
        UPtr<EditorCameraCtrl> cameraCtrl = UPtr<EditorCameraCtrl>(new EditorCameraCtrl());
        cameraCtrl->setCamera(_camera.get());
        setCameraCtrl(cameraCtrl.dynamicCastTo<CameraCtrl>());
    }
}

void SceneView::initCamera(bool firstPerson, float nearPlane, float farPlane, float fov)
{
    float aspectRatio = 1;
    if (_viewport.height > 0) {
        aspectRatio = _viewport.width / _viewport.height;
    }
    if (firstPerson) {
        _fpCamera.initialize(aspectRatio, nearPlane, farPlane, fov);
        _fpCamera.setPosition(Vector3(0, 0, 10));
        //_fpCamera.rotate(0.0f, -MATH_DEG_TO_RAD(10));
        if (_scene.get()) {
            _scene->addNode(uniqueFromInstant(_fpCamera.getRootNode()));
            //_scene->setActiveCamera(_fpCamera.getCamera());
        }

        _useFirstPersonCamera = true;
        UPtr<FirstPersonCtrl> cameraCtrl = UPtr<FirstPersonCtrl>(new FirstPersonCtrl());
        cameraCtrl->setFpCamera(&_fpCamera);
        setCameraCtrl(cameraCtrl.dynamicCastTo<CameraCtrl>());

        UPtr<Camera> camera = UPtr<Camera>(_fpCamera.getCamera());
        setCamera(camera.get());
        camera.take();
    }
    else {
        UPtr<Camera> camera = Camera::createPerspective(fov, aspectRatio, nearPlane, farPlane);
        if (_scene.get()) {
            Node* cameraNode = _scene->addNode("camera");
            // Attach the camera to a node. This determines the position of the camera.
            cameraNode->setCamera(uniqueFromInstant(camera.get()));
            // Make this the active camera of the scene.
            //_scene->setActiveCamera(camera);
            // Move the camera to look at the origin.
            cameraNode->translate(0, 0, 10);

            UPtr<EditorCameraCtrl> cameraCtrl = UPtr<EditorCameraCtrl>(new EditorCameraCtrl());
            cameraCtrl->setCamera(camera.get());
            setCameraCtrl(cameraCtrl.dynamicCastTo<CameraCtrl>());
        }
        _useFirstPersonCamera = false;
        setCamera(camera.get());
        //SAFE_RELEASE(camera);
    }
}