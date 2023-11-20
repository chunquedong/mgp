/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "CameraCtrl.h"

using namespace mgp;

EditorCameraCtrl::EditorCameraCtrl(): _prevX(0), _prevY(0), _pitch(0), _yaw(0), _camera(NULL)
{
}

void EditorCameraCtrl::update(float elapsedTime)
{
}

void EditorCameraCtrl::touchEvent(MotionEvent& evt)
{
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

        Vector3 position = _camera->getNode()->getTranslation();
        Vector3 originPos(0, 0, 1);
        originPos.scale(position.length());

        Matrix trans;
        Matrix::createTranslation(originPos, &trans);

        Matrix rotateY;
        Matrix::createRotationY(_yaw, &rotateY);

        Matrix rotateX;
        Matrix::createRotationX(_pitch, &rotateX);

        Matrix m = rotateY * rotateX * trans;

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
    switch (evt.type)
    {
    case MotionEvent::wheel:
        float amount = evt.wheelDelta / 4.0f;
        Vector3 v = _camera->getNode()->getForwardVectorWorld();
        v.normalize().scale(amount);
        _camera->getNode()->translate(v);
        return true;
    }
    return false;
}
