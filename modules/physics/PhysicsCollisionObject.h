#ifndef PHYSICSCOLLISIONOBJECT_H_
#define PHYSICSCOLLISIONOBJECT_H_

#include "script/Script.h"
#include "math/Vector3.h"
#include "PhysicsCollisionShape.h"
#include "physics.h"
#include "scene/Component.h"

namespace mgp
{

class Node;
class PhysicsRigidBody;
class PhysicsCharacter;
class PhysicsGhostObject;
class PhysicsVehicle;
class PhysicsVehicleWheel;

#define PHYSICS_COLLISION_GROUP_DEFAULT btBroadphaseProxy::DefaultFilter
#define PHYSICS_COLLISION_MASK_DEFAULT btBroadphaseProxy::AllFilter

/**
 * Defines the base class for all physics objects that support collision events.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Collision_Objects
 */
class PhysicsCollisionObject : public Refable, public Component
{
    friend class PhysicsController;
    friend class PhysicsConstraint;
    friend class PhysicsRigidBody;
    friend class PhysicsGhostObject;

public:


    /**
     * Represents the different types of collision objects.
     */
    enum Type
    {
        /**
         * PhysicsRigidBody type.
         */
        RIGID_BODY,

        /**
         * PhysicsCharacter type.
         */
        CHARACTER,

        /** 
         * PhysicsGhostObject type.
         */
        GHOST_OBJECT,

        /** 
         * PhysicsVehicle type.
         */
        VEHICLE,

        /** 
         * PhysicsVehicleWheel type.
         */
        VEHICLE_WHEEL,

        /**
         * No collision object.
         */
        NONE
    };

    /** 
     * Defines a pair of rigid bodies that collided (or may collide).
     */
    class CollisionPair
    {
    public:

        /**
         * Constructor.
         */
        CollisionPair(PhysicsCollisionObject* objectA, PhysicsCollisionObject* objectB);

        /**
         * Less than operator (needed for use as a key in map).
         * 
         * @param collisionPair The collision pair to compare.
         * @return True if this pair is "less than" the given pair; false otherwise.
         */
        bool operator < (const CollisionPair& collisionPair) const;

        /**
         * The first object in the collision.
         */
        PhysicsCollisionObject* objectA;

        /**
         * The second object in the collision.
         */
        PhysicsCollisionObject* objectB;
    };

    /**
     * Collision listener interface.
     */
    class CollisionListener
    {
        friend class PhysicsCollisionObject;
        friend class PhysicsController;

    public:

        /**
         * The type of collision event.
         */
        enum EventType
        {
            /**
             * Event fired when the two rigid bodies start colliding.
             */
            COLLIDING,

            /**
             * Event fired when the two rigid bodies no longer collide.
             */
            NOT_COLLIDING
        };

        /**
         * Virtual destructor.
         */
        virtual ~CollisionListener() { }

        /**
         * Called when a collision occurs between two objects in the physics world.
         * 
         * NOTE: You are not permitted to disable physics objects from within this callback. Disabling physics on a collision object
         *  removes the object from the physics world. This is not permitted during the PhysicsController::update.
         *
         * @param type The type of collision event.
         * @param collisionPair The two collision objects involved in the collision.
         * @param contactPointA The contact point with the first object (in world space).
         * @param contactPointB The contact point with the second object (in world space).
         */
        virtual void collisionEvent(PhysicsCollisionObject::CollisionListener::EventType type,
                                    const PhysicsCollisionObject::CollisionPair& collisionPair,
                                    const Vector3& contactPointA = Vector3::zero(),
                                    const Vector3& contactPointB = Vector3::zero()) = 0;
    };

    /**
     * Virtual destructor.
     */
    virtual ~PhysicsCollisionObject();

    /**
     * Returns the type of the collision object.
     */
    virtual PhysicsCollisionObject::Type getType() const = 0;

    /**
     * Returns the type of the shape for this collision object.
     */
    PhysicsCollisionShape::Type getShapeType() const;

    /**
     * Returns the node associated with this collision object.
     */
    Node* getNode() const;

    /**
     * Returns the collision shape.
     *
     * @return The collision shape.
     */
    PhysicsCollisionShape* getCollisionShape() const;

    /**
     * Returns whether this collision object is kinematic.
     *
     * A kinematic collision object is an object that is not simulated by
     * the physics system and instead has its transform driven manually.
     *
     * @return true if the collision object is kinematic.
     */
    bool isKinematic() const;

    /**
     * Returns whether this collision object is static.
     *
     * A static collision object is not simulated by the physics system and cannot be
     * transformed once created.
     *
     * @return true if the collision object is static.
     */
    bool isStatic() const;

    /**
     * Returns whether this collision object is dynamic.
     *
     * A dynamic collision object is simulated entirely by the physics system,
     * such as with dynamic rigid bodies. 
     *
     * @return true if the collision object is dynamic.
     */
    bool isDynamic() const;

    /**
     * Check if the collision object is enabled.
     *
     * @return true if the collision object is enabled.
     */
    bool isEnabled() const;

    /**
     * Sets the collision object to be enabled or disabled.
     *
     * @param enable true enables the collision object, false disables it.
     */
    void setEnabled(bool enable);

    /**
     * Adds a collision listener for this collision object.
     * 
     * @param listener The listener to add.
     * @param object Optional collision object used to filter the collision event.
     */
    void addCollisionListener(CollisionListener* listener, PhysicsCollisionObject* object = NULL);

    /**
     * Removes a collision listener.
     *
     * @param listener The listener to remove.
     * @param object Optional collision object used to filter the collision event.
     */
    void removeCollisionListener(CollisionListener* listener, PhysicsCollisionObject* object = NULL);

    /**
     * Adds a collision listener for this collision object.
     * 
     * Note: the given script function must be global and it must match the function 
     * signature of PhysicsCollisionObject::CollisionListener::collisionEvent.
     * 
     * @param function A valid global script function to add as a listener callback.
     * @param object Optional collision object used to filter the collision event.
     */
    void addCollisionListener(const char* function, PhysicsCollisionObject* object = NULL);

    /**
     * Removes a collision listener.
     *
     * @param function The previously added script function to remove.
     * @param object Optional collision object used to filter the collision event.
     */
    void removeCollisionListener(const char* function, PhysicsCollisionObject* object = NULL);

    /**
     * Checks if this collision object collides with the given object.
     * 
     * @param object The collision object to test for collision with.
     * 
     * @return true if this object collides with the specified one; false otherwise.
     */
    bool collidesWith(PhysicsCollisionObject* object) const;

    void setName(const std::string& name) {
        this->name = name;
    }
    const std::string& getName() {
        return name;
    }

    /**
     * Sets the physics collision object for this node using the data from the Properties object defined at the specified URL,
     * where the URL is of the format "<file-path>.<extension>#<namespace-id>/<namespace-id>/.../<namespace-id>"
     * (and "#<namespace-id>/<namespace-id>/.../<namespace-id>" is optional).
     *
     * @param url The URL pointing to the Properties object defining the physics collision object.
     */
    static PhysicsCollisionObject* load(const std::string &name, Node* node);

    /**
     * Sets (or disables) the physics collision object for this node.
     *
     * The supported collision object types include rigid bodies, ghost objects,
     * characters, vehicles, and vehicle wheels.
     *
     * Rigid bodies are used to represent most physical objects in a game. The important
     * feature of rigid bodies is that they can be simulated by the physics system as other
     * rigid bodies or collision objects collide with them. To support this physics simulation,
     * rigid bodies require additional parameters, such as mass, friction and restitution to
     * define their physical features. These parameters can be passed into the
     * 'rigidBodyParameters' parameter.
     *
     * Vehicles consist of a rigid body with wheels. The rigid body parameters can be passed-in
     * via the 'rigidBodyParameters' parameter, and wheels can be added to the vehicle.
     *
     * Ghost objects are a simple type of collision object that are not simulated. By default
     * they pass through other objects in the scene without affecting them. Ghost objects do
     * receive collision events however, which makes them useful for representing non-simulated
     * entities in a game that still require collision events, such as volumetric triggers,
     * power-ups, etc.
     *
     * Characters are an extension of ghost objects which provide a number of additional features
     * for animating and moving characters within a game. Characters are represented as ghost
     * objects instead of rigid bodies to allow more direct control over character movement,
     * since attempting to model a physics character with a simulated rigid body usually results
     * in unresponsive and unpredictable character movement. Unlike normal ghost objects,
     * characters to react to other characters and rigid bodies in the world. Characters react
     * to gravity and collide (and respond) with rigid bodies to allow them to walk on the ground,
     * slide along walls and walk up/down slopes and stairs.
     *
     * @param type The type of the collision object to set; to disable the physics
     *        collision object, pass PhysicsCollisionObject::NONE.
     * @param shape Definition of a physics collision shape to be used for this collision object.
     *        Use the static shape methods on the PhysicsCollisionShape class to specify a shape
     *        definition, such as PhysicsCollisionShape::box().
     * @param rigidBodyParameters If type is PhysicsCollisionObject::RIGID_BODY or
     *        PhysicsCollisionObject::VEHICLE, this must point to a valid rigid body
     *        parameters object containing information about the rigid body;
     *        otherwise, this parameter may be NULL.
     * @param group Group identifier of the object for collision filtering.
     * @param mask Bitmask to filter groups of objects to collide with this one.
     */
    static PhysicsCollisionObject * setCollisionObject(Node* node, PhysicsCollisionObject::Type type,
        const PhysicsCollisionShape::Definition & shape = PhysicsCollisionShape::box(),
        void * rigidBodyParameters = NULL,
        int group = PHYSICS_COLLISION_GROUP_DEFAULT,
        int mask = PHYSICS_COLLISION_MASK_DEFAULT);

protected:

    /**
     * Handles collision event callbacks to Lua script functions.
     */
    class ScriptListener : public CollisionListener
    {
    public:

        /**
         * Destructor.
         */
        ~ScriptListener();

        /**
         * Creates a ScriptListener for the given script function url.
         *
         * @param url The global script function, or script#function.
         *
         * @return The ScriptListener, or NULL if the function could not be loaded.
         */
        static ScriptListener* create(const char* url);

        /**
         * @see PhysicsCollisionObject::CollisionListener
         */
        void collisionEvent(PhysicsCollisionObject::CollisionListener::EventType type, const PhysicsCollisionObject::CollisionPair& collisionPair,
                                    const Vector3& contactPointA, const Vector3& contactPointB);

        /** The URL to the Lua script function to use as the callback. */
        std::string url;
        /** The loaded script that contains the function. */
        Script* script;
        /** The name of the Lua script function to use as the callback. */
        std::string function;

    private:

        /**
         * Constructor.
         */
        ScriptListener();
    };

    /**
     * Constructor.
     */
    PhysicsCollisionObject(Node* node, int group = PHYSICS_COLLISION_GROUP_DEFAULT, int mask = PHYSICS_COLLISION_MASK_DEFAULT);

    /**
     * Returns the Bullet Physics collision object.
     *
     * @return The Bullet collision object.
     */
    virtual btCollisionObject* getCollisionObject() const = 0;

    /**
     * Pointer to Node contained by this collision object.
     */ 
    Node* _node;
    
    /**
     * The PhysicsCollisionObject's collision shape.
     */
    PhysicsCollisionShape* _collisionShape;

    /**
     * If the collision object is enabled or not.
     */
    bool _enabled;

    /**
     * The list of script listeners.
     */
    std::vector<ScriptListener*>* _scriptListeners;

private:

    /**
     * Interface between GamePlay and Bullet to keep object transforms synchronized properly.
     * 
     * @see btMotionState
     */
    class PhysicsMotionState : public btMotionState
    {
        friend class PhysicsConstraint;
        
    public:
        
        /**
         * Creates a physics motion state for a rigid body.
         * 
         * @param node The node that contains the transformation to be associated with the motion state.
         * @param collisionObject The collision object that owns the motion state.
         * @param centerOfMassOffset The translation offset to the center of mass of the rigid body.
         */
        PhysicsMotionState(Node* node, PhysicsCollisionObject* collisionObject, const Vector3* centerOfMassOffset = NULL);
        
        /**
         * Destructor.
         */
        virtual ~PhysicsMotionState();
        
        /**
         * @see btMotionState::getWorldTransform
         */
        virtual void getWorldTransform(btTransform &transform) const;
        
        /**
         * @see btMotionState::setWorldTransform
         */
        virtual void setWorldTransform(const btTransform &transform);
        
        /**
         * Updates the motion state's world transform from the GamePlay Node object's world transform.
         */
        void updateTransformFromNode() const;
        
        /**
         * Sets the center of mass offset for the associated collision shape.
         */
        void setCenterOfMassOffset(const Vector3& centerOfMassOffset);
        
    private:
        
        Node* _node;
        PhysicsCollisionObject* _collisionObject;
        btTransform _centerOfMassOffset;
        mutable btTransform _worldTransform;
    };

    /** 
     * The PhysicsCollisionObject's motion state.
     */
    PhysicsMotionState* _motionState;

    /**
     * Group identifier and the bitmask for collision filtering.
     */
    int _group;
    int _mask;
    std::string name;
};

}

#endif
