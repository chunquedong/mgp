/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef SCENEVIEW_H_
#define SCENEVIEW_H_

#include "base/Base.h"
#include "scene/Renderer.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "scene/Drawable.h"
#include "render/RenderQueue.h"
#include "render/RenderPath.h"
#include "FirstPersonCamera.h"
#include "CameraCtrl.h"

namespace mgp {


class SceneView {
    UPtr<Scene> _scene;
    UPtr<Camera> _camera;
    Rectangle _viewport;
    UPtr<RenderPath> _renderPath;


    bool _useFirstPersonCamera = false;
    FirstPersonCamera _fpCamera;
    UPtr<CameraCtrl> _cameraCtrl;

public:
    SceneView();
    ~SceneView();

    void update(float elapsedTime);
    void render();

    /**
    * Add a default Camera
    */
    void initCamera(bool firstPerson, float nearPlane = 1.0f, float farPlane = 1000.0f, float fov = 45.0f);

    void finalize();

    Rectangle* getViewport() { return &_viewport; }
    void setViewport(Rectangle* rect);
    Scene* getScene() { return _scene.get(); }
    void setScene(UPtr<Scene> s) { _scene = std::move(s); }
    Camera* getCamera() { return _camera.get(); }
    void setCamera(Camera* s, bool initCameraCtrl = true);
    RenderPath* getRenderPath() { return _renderPath.get(); }
    void setRenderPath(UPtr<RenderPath> s) { _renderPath = std::move(s); }

    CameraCtrl* getCameraCtrl() { return _cameraCtrl.get(); };
    void setCameraCtrl(UPtr<CameraCtrl> c);
};


}

#endif