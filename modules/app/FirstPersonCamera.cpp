#include "FirstPersonCamera.h"

using namespace mgp;

FirstPersonCamera::FirstPersonCamera()
    : _pitchNode(NULL), _rootNode(NULL)
{
    
}

FirstPersonCamera::~FirstPersonCamera()
{
    SAFE_RELEASE(_pitchNode);
    SAFE_RELEASE(_rootNode);
}

void FirstPersonCamera::initialize(float aspectRatio, float nearPlane, float farPlane, float fov)
{
    SAFE_RELEASE(_pitchNode);
    SAFE_RELEASE(_rootNode);
    _rootNode = Node::create("FirstPersonCamera_root").take();
    _pitchNode = Node::create("FirstPersonCamera_pitch").take();
    _rootNode->addChild(uniqueFromInstant(_pitchNode));

    assert(aspectRatio > 0.0f);
    UPtr<Camera> camera = Camera::createPerspective(fov, aspectRatio, nearPlane, farPlane);
    _pitchNode->setCamera(std::move(camera));
    //SAFE_RELEASE(camera);
}

Node* FirstPersonCamera::getRootNode()
{
    return _rootNode;
}

Camera* FirstPersonCamera::getCamera()
{
    if (_pitchNode)
        return _pitchNode->getCamera();
    return NULL;
}

void FirstPersonCamera::setPosition(const Vector3& position)
{
    _rootNode->setTranslation(position);
}

const Vector3& FirstPersonCamera::getPosition()
{
    return _rootNode->getTranslation();
}

void FirstPersonCamera::moveForward(float amount)
{
    Vector3 v = _pitchNode->getForwardVectorWorld();
    v.normalize().scale(amount);
    _rootNode->translate(v);
}

void FirstPersonCamera::moveBackward(float amount)
{
    moveForward(-amount);
}

void FirstPersonCamera::moveLeft(float amount)
{
    _rootNode->translateLeft(amount);
}

void FirstPersonCamera::moveRight(float amount)
{
    _rootNode->translateLeft(-amount);
}

void FirstPersonCamera::moveUp(float amount)
{
    _rootNode->translateUp(amount);
}

void FirstPersonCamera::moveDown(float amount)
{
    _rootNode->translateUp(-amount);
}

void FirstPersonCamera::rotate(float yaw, float pitch)
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

void FirstPersonCtrl::update(float elapsedTime)
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
            _camera->moveUp(time * UP_DOWN_SPEED);
        }
        else if (_moveFlags & MOVE_DOWN)
        {
            _camera->moveDown(time * UP_DOWN_SPEED);
        }
    }

    if (!mouseMove.isZero())
    {
        mouseMove.scale(time * MOVE_SPEED);
        _camera->moveForward(mouseMove.y);
        _camera->moveLeft(mouseMove.x);
    }
}


void FirstPersonCtrl::touchEvent(MotionEvent& evt)
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
        _camera->rotate(yaw, pitch);
        break;
    }
    };
}

void FirstPersonCtrl::keyEvent(Keyboard evt)
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
            _camera->rotate(0, MATH_PIOVER4);
            break;
        case Keyboard::KEY_PG_DOWN:
            _camera->rotate(0, -MATH_PIOVER4);
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

bool FirstPersonCtrl::mouseEvent(Mouse evt)
{
    switch (evt.type)
    {
    case MotionEvent::wheel:
        _camera->moveForward(evt.wheelDelta * MOVE_SPEED / 4.0f);
        return true;
    }
    return false;
}
