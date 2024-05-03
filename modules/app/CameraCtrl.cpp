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
    if (_dirty) {
        _dirty = false;

        Vector3 position = _camera->getNode()->getTranslationWorld();
        double distance = position.distance(_rotateCenter);
        Vector3 originPos(0, 0, 1);
        originPos.scale(distance);

        Matrix trans;
        Matrix::createTranslation(originPos, &trans);

        Matrix rotateY;
        Matrix::createRotationY(_yaw, &rotateY);

        Matrix rotateX;
        Matrix::createRotationX(_pitch, &rotateX);

        Matrix centerTrans;
        Matrix::createTranslation(_rotateCenter, &centerTrans);

        Matrix m = centerTrans * rotateY * rotateX * trans;

        _camera->getNode()->setMatrix(m);
    }
}

void EditorCameraCtrl::setRotateCenter(const Vector3& c) {
    _rotateCenter = c;
    _dirty = true;
}

void EditorCameraCtrl::setRotate(float pitch, float yaw) {
    _pitch = pitch; _yaw = yaw;
    _dirty = true;
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
        _surfaceDistance = _camera->getNode()->getTranslationWorld().distance(_rotateCenter);
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
        _isPressed = true;
        break;
    case MotionEvent::release:
        _prevX = 0;
        _prevY = 0;
        _isPressed = false;
        break;
    case MotionEvent::touchMove:
        if (_isPressed) {
            int deltaX = evt.x - _prevX;
            int deltaY = evt.y - _prevY;
            _prevX = evt.x;
            _prevY = evt.y;

            if (evt.button == MotionEvent::right || evt.button == MotionEvent::middle) {
                updateSurfaceDistance();
                Rectangle& viewport = *sceneView->getViewport();
                double fovDivisor = tan(MATH_DEG_TO_RAD(_camera->getFieldOfView()) / 2) / (viewport.height / 2);
                double scale = fovDivisor * _surfaceDistance;

                Vector3 left = -_camera->getNode()->getRightVectorWorld();
                left.normalize().scale(deltaX * scale);
                Vector3 up = _camera->getNode()->getUpVectorWorld();
                up.normalize().scale(deltaY * scale);
                _rotateCenter += left + up;
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
            _dirty = true;
        }
        break;
    };
}

bool EditorCameraCtrl::keyEvent(Keyboard evt)
{
    return false;
}

bool EditorCameraCtrl::mouseEvent(Mouse evt)
{
    if (!sceneView) return false;
    switch (evt.type)
    {
    case MotionEvent::wheel:
        updateSurfaceDistance();
        float wheelDelta = evt.wheelDelta;
        if (_reverseZoom) {
            wheelDelta = -wheelDelta;
        }
        float amount = (wheelDelta * 0.1f) * (_surfaceDistance-0.1);
        if (amount < 0 && amount > -10E-4) {
            amount = -10E-4;
        }
        Vector3 v = _camera->getNode()->getForwardVectorWorld();
        v.normalize().scale(amount);
        _camera->getNode()->translate(v);
        return true;
    }

    touchEvent(evt);
    return true;
}
