#ifndef FIRSTPERSONCAMERA_H_
#define FIRSTPERSONCAMERA_H_

#include "scene/Scene.h"
#include "platform/Keyboard.h"
#include "platform/Mouse.h"
#include "CameraCtrl.h"

namespace mgp {

/**
 * FirstPersonCamera controls a camera like a first person shooter game.
 */
class FPCameraCtrl : public CameraCtrl
{
public:

    /**
     * Constructor.
     */
    FPCameraCtrl();

    /**
     * Destructor.
     */
    ~FPCameraCtrl();

    void setCamera(Camera* camera);

    /**
     * Gets root node. May be NULL if not initialized.
     * 
     * @return Root node or NULL.
     */
    Node* getRootNode();

    /**
     * Gets the camera. May be NULL.
     * 
     * @return Camera or NULL.
     */
    Camera* getCamera();

    /**
     * Sets the position of the camera.
     * 
     * @param position The position to move to.
     */
    void setPosition(const Vector3& position);

    /**
     * Gets the position of the camera.
     * 
     * @return the current camera position
     */
    const Vector3& getPosition();

    /**
     * Moves the camera forward in the direction that it is pointing. (Fly mode)
     */
    void moveForward(float amount);

    /**
     * Moves the camera in the opposite direction that it is pointing.
     */
    void moveBackward(float amount);

    /**
     * Strafes that camera left, which is perpendicular to the direction it is facing.
     */
    void moveLeft(float amount);

    /**
     * Strafes that camera right, which is perpendicular to the direction it is facing.
     */
    void moveRight(float amount);

    void moveUp(float amount);

    void moveDown(float amount);

    /**
     * Rotates the camera in place in order to change the direction it is looking.
     * 
     * @param yaw Rotates the camera around the yaw axis in radians. Positive looks right, negative looks left.
     * @param pitch Rotates the camera around the ptich axis in radians. Positive looks up, negative looks down.
     */
    void rotate(float yaw, float pitch);


public:
    void update(float elapsedTime);

    void touchEvent(MotionEvent& evt);

    bool keyEvent(Keyboard evt);

    bool mouseEvent(Mouse evt);
private:

    Node* _pitchNode;
    Node* _rootNode;

    unsigned int _moveFlags = 0;
    int _prevX = 0;
    int _prevY = 0;
};

}
#endif
