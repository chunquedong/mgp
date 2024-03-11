#include "FirstPersonCamera.h"

using namespace mgp;

FPCameraCtrl::FPCameraCtrl()
    : _pitchNode(NULL), _rootNode(NULL)
{
    
}

FPCameraCtrl::~FPCameraCtrl()
{
    SAFE_RELEASE(_pitchNode);
    SAFE_RELEASE(_rootNode);
}

void FPCameraCtrl::setCamera(Camera* camera) {
    SAFE_RELEASE(_pitchNode);
    SAFE_RELEASE(_rootNode);
    _rootNode = Node::create("FirstPersonCtrl_root").take();
    _pitchNode = Node::create("FirstPersonCtrl_pitch").take();
    _rootNode->addChild(uniqueFromInstant(_pitchNode));
    _rootNode->setSerializable(false);

    _pitchNode->setCamera(uniqueFromInstant(camera));
}

Node* FPCameraCtrl::getRootNode()
{
    return _rootNode;
}

Camera* FPCameraCtrl::getCamera()
{
    if (_pitchNode)
        return _pitchNode->getCamera();
    return NULL;
}

void FPCameraCtrl::setPosition(const Vector3& position)
{
    _rootNode->setTranslation(position);
}

const Vector3& FPCameraCtrl::getPosition()
{
    return _rootNode->getTranslation();
}

void FPCameraCtrl::moveForward(float amount)
{
    Vector3 v = _pitchNode->getForwardVectorWorld();
    v.normalize().scale(amount);
    _rootNode->translate(v);
}

void FPCameraCtrl::moveBackward(float amount)
{
    moveForward(-amount);
}

void FPCameraCtrl::moveLeft(float amount)
{
    _rootNode->translateLeft(amount);
}

void FPCameraCtrl::moveRight(float amount)
{
    _rootNode->translateLeft(-amount);
}

void FPCameraCtrl::moveUp(float amount)
{
    _rootNode->translateUp(amount);
}

void FPCameraCtrl::moveDown(float amount)
{
    _rootNode->translateUp(-amount);
}

void FPCameraCtrl::rotate(float yaw, float pitch)
{
    _rootNode->rotateY(-yaw);
    _pitchNode->rotateX(pitch);
}


static const unsigned int MOVE_FORWARD = 1;
static const unsigned int MOVE_BACKWARD = 2;
static const unsigned int MOVE_LEFT = 4;
static const unsigned int MOVE_RIGHT = 8;
static const unsigned int MOVE_UP = 16;
static const unsigned int MOVE_DOWN = 32;

static const float MOVE_SPEED = 8.0f;
static const float UP_DOWN_SPEED = 8.0f;

void FPCameraCtrl::update(float elapsedTime)
{
    float time = (float)elapsedTime / 1000.0f;

    Vector2 mouseMove;
    if (_moveFlags != 0)
    {
        // Forward motion
        if (_moveFlags & MOVE_FORWARD)
        {
            mouseMove.y = 1;
        }
        else if (_moveFlags & MOVE_BACKWARD)
        {
            mouseMove.y = -1;
        }
        // Strafing
        if (_moveFlags & MOVE_LEFT)
        {
            mouseMove.x = 1;
        }
        else if (_moveFlags & MOVE_RIGHT)
        {
            mouseMove.x = -1;
        }
        mouseMove.normalize();

        // Up and down
        if (_moveFlags & MOVE_UP)
        {
            this->moveUp(time * UP_DOWN_SPEED);
        }
        else if (_moveFlags & MOVE_DOWN)
        {
            this->moveDown(time * UP_DOWN_SPEED);
        }
    }

    if (!mouseMove.isZero())
    {
        mouseMove.scale(time * MOVE_SPEED);
        this->moveForward(mouseMove.y);
        this->moveLeft(mouseMove.x);
    }
}


void FPCameraCtrl::touchEvent(MotionEvent& evt)
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
        float pitch = -MATH_DEG_TO_RAD(deltaY * 0.5f);
        float yaw = MATH_DEG_TO_RAD(deltaX * 0.5f);
        this->rotate(yaw, pitch);
        break;
    }
    };
}

void FPCameraCtrl::keyEvent(Keyboard evt)
{
    if (evt.evt == Keyboard::KEY_PRESS)
    {
        switch (evt.key)
        {
        case Keyboard::KEY_W:
            _moveFlags |= MOVE_FORWARD;
            break;
        case Keyboard::KEY_S:
            _moveFlags |= MOVE_BACKWARD;
            break;
        case Keyboard::KEY_A:
            _moveFlags |= MOVE_LEFT;
            break;
        case Keyboard::KEY_D:
            _moveFlags |= MOVE_RIGHT;
            break;

        case Keyboard::KEY_Q:
            _moveFlags |= MOVE_DOWN;
            break;
        case Keyboard::KEY_E:
            _moveFlags |= MOVE_UP;
            break;
        case Keyboard::KEY_PG_UP:
            this->rotate(0, MATH_PIOVER4);
            break;
        case Keyboard::KEY_PG_DOWN:
            this->rotate(0, -MATH_PIOVER4);
            break;

        case Keyboard::KEY_ONE:
        case Keyboard::KEY_SPACE:
            break;
        }
    }
    else if (evt.evt == Keyboard::KEY_RELEASE)
    {
        switch (evt.key)
        {
        case Keyboard::KEY_W:
            _moveFlags &= ~MOVE_FORWARD;
            break;
        case Keyboard::KEY_S:
            _moveFlags &= ~MOVE_BACKWARD;
            break;
        case Keyboard::KEY_A:
            _moveFlags &= ~MOVE_LEFT;
            break;
        case Keyboard::KEY_D:
            _moveFlags &= ~MOVE_RIGHT;
            break;
        case Keyboard::KEY_Q:
            _moveFlags &= ~MOVE_DOWN;
            break;
        case Keyboard::KEY_E:
            _moveFlags &= ~MOVE_UP;
            break;
        }
    }
}

bool FPCameraCtrl::mouseEvent(Mouse evt)
{
    switch (evt.type)
    {
    case MotionEvent::wheel:
        this->moveForward(evt.wheelDelta * MOVE_SPEED / 4.0f);
        return true;
    }
    return false;
}
