#include "base/Base.h"
#include "PhysicsCollisionObject.h"
#include "PhysicsController.h"
#include "platform/Toolkit.h"
#include "scene/Node.h"
#include "script/ScriptController.h"
#include "PhysicsRigidBody.h"
#include "PhysicsCharacter.h"
#include "PhysicsGhostObject.h"
#include "PhysicsVehicle.h"
#include "PhysicsVehicleWheel.h"

namespace mgp
{

extern void splitURL(const std::string& url, std::string* file, std::string* id);

/**
 * Internal class used to implement the collidesWith(PhysicsCollisionObject*) function.
 * @script{ignore}
 */
struct CollidesWithCallback : public btCollisionWorld::ContactResultCallback
{
    /**
     * Called with each contact. Needed to implement collidesWith(PhysicsCollisionObject*).
     */
    btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* a, int partIdA, int indexA, const btCollisionObjectWrapper* b, int partIdB, int indexB)
    {
        result = true;
        return 0.0f;
    }

    /**
     * The result of the callback.
     */
    bool result;
};

PhysicsCollisionObject::PhysicsCollisionObject(Node* node, int group, int mask)
    : _node(node), _collisionShape(NULL), _enabled(true), _scriptListeners(NULL), _motionState(NULL), _group(group), _mask(mask)
{
}

PhysicsCollisionObject::~PhysicsCollisionObject()
{
    SAFE_DELETE(_motionState);

    if (_scriptListeners)
    {
        for (size_t i = 0, count = _scriptListeners->size(); i < count; ++i)
        {
            SAFE_DELETE((*_scriptListeners)[i]);
        }
        SAFE_DELETE(_scriptListeners);
    }

    GP_ASSERT(PhysicsController::cur());
    PhysicsController::cur()->destroyShape(_collisionShape);
}

PhysicsCollisionShape::Type PhysicsCollisionObject::getShapeType() const
{
    GP_ASSERT(getCollisionShape());
    return getCollisionShape()->getType();
}

Node* PhysicsCollisionObject::getNode() const
{
    return _node;
}

PhysicsCollisionShape* PhysicsCollisionObject::getCollisionShape() const
{
    return _collisionShape;
}

bool PhysicsCollisionObject::isKinematic() const
{
    switch (getType())
    {
    case GHOST_OBJECT:
    case CHARACTER:
        return true;
    default:
        GP_ASSERT(getCollisionObject());
        return getCollisionObject()->isKinematicObject();
    }
}

bool PhysicsCollisionObject::isStatic() const
{
    switch (getType())
    {
    case GHOST_OBJECT:
    case CHARACTER:
        return false;
    default:
        GP_ASSERT(getCollisionObject());
        return getCollisionObject()->isStaticObject();
    }
}

bool PhysicsCollisionObject::isDynamic() const
{
    GP_ASSERT(getCollisionObject());
    return !getCollisionObject()->isStaticOrKinematicObject();
}

bool PhysicsCollisionObject::isEnabled() const
{
    return _enabled;
}

void PhysicsCollisionObject::setEnabled(bool enable)
{
    if (enable)
    {  
        if (!_enabled)
        {
            PhysicsController::cur()->addCollisionObject(this);
            _motionState->updateTransformFromNode();
            _enabled = true;
        }
    }
    else
    {
        if (_enabled)
        {
            PhysicsController::cur()->removeCollisionObject(this, false);
            _enabled = false;
        }
    }
}

void PhysicsCollisionObject::addCollisionListener(CollisionListener* listener, PhysicsCollisionObject* object)
{
    GP_ASSERT(PhysicsController::cur());
    PhysicsController::cur()->addCollisionListener(listener, this, object);
}

void PhysicsCollisionObject::removeCollisionListener(CollisionListener* listener, PhysicsCollisionObject* object)
{
    GP_ASSERT(PhysicsController::cur());
    PhysicsController::cur()->removeCollisionListener(listener, this, object);
}

void PhysicsCollisionObject::addCollisionListener(const char* function, PhysicsCollisionObject* object)
{
    ScriptListener* listener = ScriptListener::create(function);
    if (!listener)
        return; // falied to load

    if (!_scriptListeners)
        _scriptListeners = new std::vector<ScriptListener*>();
    _scriptListeners->push_back(listener);

    addCollisionListener(listener, object);
}

void PhysicsCollisionObject::removeCollisionListener(const char* function, PhysicsCollisionObject* object)
{
    if (!_scriptListeners)
        return;

    std::string url = function;
    for (size_t i = 0, count = _scriptListeners->size(); i < count; ++i)
    {
        if ((*_scriptListeners)[i]->url == url)
        {
            removeCollisionListener((*_scriptListeners)[i], object);
            SAFE_DELETE((*_scriptListeners)[i]);
            _scriptListeners->erase(_scriptListeners->begin() + i);
            return;
        }
    }
}

bool PhysicsCollisionObject::collidesWith(PhysicsCollisionObject* object) const
{
    GP_ASSERT(PhysicsController::cur() && PhysicsController::cur()->_world);
    GP_ASSERT(object && object->getCollisionObject());
    GP_ASSERT(getCollisionObject());

    static CollidesWithCallback callback;

    callback.result = false;
    PhysicsController::cur()->_world->contactPairTest(getCollisionObject(), object->getCollisionObject(), callback);
    return callback.result;
}

PhysicsCollisionObject::CollisionPair::CollisionPair(PhysicsCollisionObject* objectA, PhysicsCollisionObject* objectB)
    : objectA(objectA), objectB(objectB)
{
    // unused
}

bool PhysicsCollisionObject::CollisionPair::operator < (const CollisionPair& collisionPair) const
{
    // If the pairs are equal, then return false.
    if ((objectA == collisionPair.objectA && objectB == collisionPair.objectB) || (objectA == collisionPair.objectB && objectB == collisionPair.objectA))
        return false;

    // We choose to compare based on objectA arbitrarily.
    if (objectA < collisionPair.objectA)
        return true;

    if (objectA == collisionPair.objectA)
        return objectB < collisionPair.objectB;

    return false;
}

PhysicsCollisionObject::PhysicsMotionState::PhysicsMotionState(Node* node, PhysicsCollisionObject* collisionObject, const Vector3* centerOfMassOffset) :
    _node(node), _collisionObject(collisionObject), _centerOfMassOffset(btTransform::getIdentity())
{
    if (centerOfMassOffset)
    {
        // Store the center of mass offset.
        _centerOfMassOffset.setOrigin(BV(*centerOfMassOffset));
    }

    updateTransformFromNode();
}

PhysicsCollisionObject::PhysicsMotionState::~PhysicsMotionState()
{
}

void PhysicsCollisionObject::PhysicsMotionState::getWorldTransform(btTransform &transform) const
{
    GP_ASSERT(_node);
    GP_ASSERT(_collisionObject);

    if (_collisionObject->isKinematic())
        updateTransformFromNode();

    transform = _centerOfMassOffset.inverse() * _worldTransform;
}

void PhysicsCollisionObject::PhysicsMotionState::setWorldTransform(const btTransform &transform)
{
    GP_ASSERT(_node);

    _worldTransform = transform * _centerOfMassOffset;
        
    const btQuaternion& rot = _worldTransform.getRotation();
    const btVector3& pos = _worldTransform.getOrigin();

    _node->setRotation(rot.x(), rot.y(), rot.z(), rot.w());
    _node->setTranslation(pos.x(), pos.y(), pos.z());
}

void PhysicsCollisionObject::PhysicsMotionState::updateTransformFromNode() const
{
    GP_ASSERT(_node);

    // Store the initial world transform (minus the scale) for use by Bullet later on.
    Quaternion rotation;
    const Matrix& m = _node->getWorldMatrix();
    m.getRotation(&rotation);

    if (!_centerOfMassOffset.getOrigin().isZero())
    {
        // When there is a center of mass offset, we modify the initial world transformation
        // so that when physics is initially applied, the object is in the correct location.
        btTransform offset = btTransform(BQ(rotation), btVector3(0.0f, 0.0f, 0.0f)) * _centerOfMassOffset.inverse();

        btVector3 origin(m.m[12] + _centerOfMassOffset.getOrigin().getX() + offset.getOrigin().getX(), 
                         m.m[13] + _centerOfMassOffset.getOrigin().getY() + offset.getOrigin().getY(), 
                         m.m[14] + _centerOfMassOffset.getOrigin().getZ() + offset.getOrigin().getZ());
        _worldTransform = btTransform(BQ(rotation), origin);
    }
    else
    {
        _worldTransform = btTransform(BQ(rotation), btVector3(m.m[12], m.m[13], m.m[14]));
    }
}

void PhysicsCollisionObject::PhysicsMotionState::setCenterOfMassOffset(const Vector3& centerOfMassOffset)
{
    _centerOfMassOffset.setOrigin(BV(centerOfMassOffset));
}

PhysicsCollisionObject::ScriptListener::ScriptListener()
    : script(NULL)
{
}

PhysicsCollisionObject::ScriptListener::~ScriptListener()
{
    SAFE_RELEASE(script);
}

PhysicsCollisionObject::ScriptListener* PhysicsCollisionObject::ScriptListener::create(const char* url)
{
    std::string scriptPath, func;
    splitURL(url, &scriptPath, &func);
    if (func.empty())
    {
        // Only a function was specified
        func = scriptPath;
        scriptPath = "";
    }

    Script* script = NULL;
    if (!scriptPath.empty())
    {
        script = ScriptController::cur()->loadScript(scriptPath.c_str(), Script::GLOBAL);
        if (!script)
        {
            // Failed to load script
            return NULL;
        }
    }

    ScriptListener* listener = new ScriptListener();
    listener->url = url;
    listener->script = script;
    listener->function = func;
    return listener;
}

void PhysicsCollisionObject::ScriptListener::collisionEvent(PhysicsCollisionObject::CollisionListener::EventType type,
    const PhysicsCollisionObject::CollisionPair& collisionPair, const Vector3& contactPointA, const Vector3& contactPointB)
{
    ScriptController::cur()->executeFunction<void>(function.c_str(), 
        "[PhysicsCollisionObject::CollisionListener::EventType]<PhysicsCollisionObject::CollisionPair><Vector3><Vector3>",
        NULL,
        type, &collisionPair, &contactPointA, &contactPointB);
}


PhysicsCollisionObject* PhysicsCollisionObject::load(const std::string& url, Node* node) {
    UPtr<Properties> properties = Properties::create(url.c_str());
    if (properties.get() == NULL)
    {
        //GP_ERROR("Failed to load scene file '%s'.", url);
        return NULL;
    }

    // Check if the properties is valid.
    if (!properties.get() || !(strcmp(properties->getNamespace(), "collisionObject") == 0))
    {
        GP_ERROR("Failed to load collision object from properties object: must be non-null object and have namespace equal to 'collisionObject'.");
        return NULL;
    }

    PhysicsCollisionObject *_collisionObject = NULL;

    if (const char* type = properties->getString("type"))
    {
        if (strcmp(type, "CHARACTER") == 0)
        {
            _collisionObject = PhysicsCharacter::create(node, properties.get());
        }
        else if (strcmp(type, "GHOST_OBJECT") == 0)
        {
            _collisionObject = PhysicsGhostObject::create(node, properties.get());
        }
        else if (strcmp(type, "RIGID_BODY") == 0)
        {
            _collisionObject = PhysicsRigidBody::create(node, properties.get());
        }
        else if (strcmp(type, "VEHICLE") == 0)
        {
            _collisionObject = PhysicsVehicle::create(node, properties.get());
        }
        else if (strcmp(type, "VEHICLE_WHEEL") == 0)
        {
            //
            // PhysicsVehicleWheel is special because this call will traverse up the scene graph for the
            // first ancestor node that is shared with another node of collision type VEHICLE, and then
            // proceed to add itself as a wheel onto that vehicle. This is by design, and allows the
            // visual scene hierarchy to be the sole representation of the relationship between physics
            // objects rather than forcing that upon the otherwise-flat ".physics" (properties) file.
            //
            // IMPORTANT: The VEHICLE must come before the VEHICLE_WHEEL in the ".scene" (properties) file!
            //
            _collisionObject = PhysicsVehicleWheel::create(node, properties.get());
        }
        else
        {
            GP_ERROR("Unsupported collision object type '%s'.", type);
            return NULL;
        }
    }
    else
    {
        GP_ERROR("Failed to load collision object from properties object; required attribute 'type' is missing.");
        return NULL;
    }

    node->addComponent<PhysicsCollisionObject>(UPtr<PhysicsCollisionObject>(_collisionObject));
    return _collisionObject;
}

PhysicsCollisionObject* PhysicsCollisionObject::setCollisionObject(Node* node, PhysicsCollisionObject::Type type, const PhysicsCollisionShape::Definition& shape,
    void* _rigidBodyParameters, int group, int mask)
{

    PhysicsRigidBody::Parameters* rigidBodyParameters = (PhysicsRigidBody::Parameters*)_rigidBodyParameters;
    PhysicsCollisionObject* _collisionObject = NULL;
    switch (type)
    {
    case PhysicsCollisionObject::RIGID_BODY:
    {
        _collisionObject = new PhysicsRigidBody(node, shape, rigidBodyParameters ? *rigidBodyParameters : PhysicsRigidBody::Parameters(), group, mask);
    }
    break;

    case PhysicsCollisionObject::GHOST_OBJECT:
    {
        _collisionObject = new PhysicsGhostObject(node, shape, group, mask);
    }
    break;

    case PhysicsCollisionObject::CHARACTER:
    {
        _collisionObject = new PhysicsCharacter(node, shape, rigidBodyParameters ? rigidBodyParameters->mass : 1.0f);
    }
    break;

    case PhysicsCollisionObject::VEHICLE:
    {
        _collisionObject = new PhysicsVehicle(node, shape, rigidBodyParameters ? *rigidBodyParameters : PhysicsRigidBody::Parameters());
    }
    break;

    case PhysicsCollisionObject::VEHICLE_WHEEL:
    {
        //
        // PhysicsVehicleWheel is special because this call will traverse up the scene graph for the
        // first ancestor node that is shared with another node of collision type VEHICLE, and then
        // proceed to add itself as a wheel onto that vehicle. This is by design, and allows the
        // visual scene hierarchy to be the sole representation of the relationship between physics
        // objects rather than forcing that upon the otherwise-flat ".physics" (properties) file.
        //
        // IMPORTANT: The VEHICLE must come before the VEHICLE_WHEEL in the ".scene" (properties) file!
        //
        _collisionObject = new PhysicsVehicleWheel(node, shape, rigidBodyParameters ? *rigidBodyParameters : PhysicsRigidBody::Parameters());
    }
    break;

    case PhysicsCollisionObject::NONE:
        break;  // Already deleted, Just don't add a new collision object back.
    }

    node->addComponent<PhysicsCollisionObject>(UPtr<PhysicsCollisionObject>(_collisionObject));

    //SAFE_RELEASE(_collisionObject);
    return _collisionObject;
}

}
