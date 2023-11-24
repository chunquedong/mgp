/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "CameraCtrl.h"
#include "SceneView.h"

using namespace mgp;

EditorCameraCtrl::EditorCameraCtrl(): _prevX(0), _prevY(0), _pitch(0), _yaw(0), _camera(NULL), sceneView(NULL), _surfaceDistance(-1)
{
}

void EditorCameraCtrl::update(float elapsedTime)
{
}

bool EditorCameraCtrl::updateSurfaceDistance() {
    if (!sceneView) return false;
    Rectangle& viewport = *sceneView->getViewport();

    Ray ray;
    _camera->pickRay(viewport, viewport.width/2, viewport.height/2, &ray);
    std::vector<Drawable*> drawables;
    sceneView->getScene()->getRootNode()->getAllDrawable(drawables);

    RayQuery query;
    //query.pickMask = 2;
    query.ray = ray;
    for (Drawable* drawable : drawables) {
        //Vector3 target;
        drawable->raycast(query);
    }
    if (query.minDistance != Ray::INTERSECTS_NONE) {
        _surfaceDistance = query.minDistance;// query.target.distance(_camera->getNode()->getTranslationWorld());
        return true;
    }
    if (_surfaceDistance == -1) {
        _surfaceDistance = (_camera->getFarPlane() + _camera->getNearPlane()) / 2.0;
    }
    return false;
}

void EditorCameraCtrl::touchEvent(MotionEvent& evt)
{
    if (!sceneView) return;
    switch (evt.type)
    {
    case MotionEvent::press:
        _prevX = evt.x;
        _prevY = evt.y;
        break;
    case MotionEvent::release:
        _prevX = 0;
        _prevY = 0;
        break;
    case MotionEvent::touchMove:
    {
        int deltaX = evt.x - _prevX;
        int deltaY = evt.y - _prevY;
        _prevX = evt.x;
        _prevY = evt.y;

        if (evt.button == MotionEvent::right) {
            updateSurfaceDistance();
            Rectangle& viewport = *sceneView->getViewport();
            double fovDivisor = tan(MATH_DEG_TO_RAD(_camera->getFieldOfView()) / 2) / (viewport.height / 2);
            double scale = fovDivisor * _surfaceDistance;
            rotateCenter.x -= deltaX * scale;
            rotateCenter.y += deltaY * scale;
        }
        else {

            float dpitch = -MATH_DEG_TO_RAD(deltaY * 0.5f);
            float dyaw = -MATH_DEG_TO_RAD(deltaX * 0.5f);

            if (_pitch > 2 * MATH_PI) {
                _pitch -= 2 * MATH_PI;
            }
            else if (_pitch < 0) {
                _pitch += 2 * MATH_PI;
            }
            if (_pitch > MATH_DEG_TO_RAD(90) && _pitch < MATH_DEG_TO_RAD(270)) {
                dyaw = -dyaw;
            }

            _pitch += dpitch;
            _yaw += dyaw;
        }

        Vector3 position = _camera->getNode()->getTranslationWorld();
        double distance = position.distance(rotateCenter);
        Vector3 originPos(0, 0, 1);
        originPos.scale(distance);

        Matrix trans;
        Matrix::createTranslation(originPos, &trans);

        Matrix rotateY;
        Matrix::createRotationY(_yaw, &rotateY);

        Matrix rotateX;
        Matrix::createRotationX(_pitch, &rotateX);

        Matrix centerTrans;
        Matrix::createTranslation(rotateCenter, &centerTrans);

        Matrix m = centerTrans * rotateY * rotateX * trans;

        _camera->getNode()->setMatrix(m);
        break;
    }
    };
}

void EditorCameraCtrl::keyEvent(Keyboard evt)
{
}

bool EditorCameraCtrl::mouseEvent(Mouse evt)
{
    if (!sceneView) return false;
    switch (evt.type)
    {
    case MotionEvent::wheel:
        updateSurfaceDistance();
        float amount = (evt.wheelDelta / 8.0f) * (_surfaceDistance-0.1);
        if (amount < 0 && amount > -10E-4) {
            amount = -10E-4;
        }
        Vector3 v = _camera->getNode()->getForwardVectorWorld();
        v.normalize().scale(amount);
        _camera->getNode()->translate(v);
        return true;
    }
    return false;
}
