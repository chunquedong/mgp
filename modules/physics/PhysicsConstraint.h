#ifndef PHYSICSCONSTRAINT_H_
#define PHYSICSCONSTRAINT_H_

#include "base/Base.h"
#include "math/Vector3.h"
#include "physics.h"

namespace mgp
{
    class PhysicsRigidBody;
    class Node;

/**
 * Defines the base class for physics constraints.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Constraints
 */
class PhysicsConstraint
{
    friend class PhysicsController;
    friend class PhysicsRigidBody;

public:
    /**
     * Gets the impulse needed to break the constraint.
     * 
     * @return The impulse needed to break the constraint.
     */
    inline float getBreakingImpulse() const;

    /**
     * Sets the impulse needed to break the constraint
     * (if an impulse greater than or equal to the given
     * value is applied to the constraint, the constraint
     * will be broken).
     * 
     * @param impulse The impulse needed to break the constraint.
     */
    inline void setBreakingImpulse(float impulse);

    /**
     * Gets whether the constraint is enabled or not.
     * 
     * @return Whether the constraint is enabled or not.
     */
    inline bool isEnabled() const;

    /**
     * Sets whether the constraint is enabled or not.
     * 
     * @param enabled Whether the constraint is enabled or not.
     */
    inline void setEnabled(bool enabled);

    /**
     * Calculates the midpoint between the given nodes' centers of mass.
     * 
     * @param a The first node.
     * @param b The second node.
     */
    static Vector3 centerOfMassMidpoint(const Node* a, const Node* b);

    /**
     * Calculates the rotation offset to the given point in the given node's local space.
     * 
     * @param node The node to calculate a rotation offset for.
     * @param point The point to calculate the rotation offset to.
     */
    static Quaternion getRotationOffset(const Node* node, const Vector3& point);

    /**
     * Calculates the translation offset to the given point in the given node's local space.
     * 
     * @param node The node to calculate a translation offset for.
     * @param point The point to calculate the translation offset to.
     */
    static Vector3 getTranslationOffset(const Node* node, const Vector3& point);

protected:

    /**
     * Constructor.
     */
    PhysicsConstraint(PhysicsRigidBody* a, PhysicsRigidBody* b);

    /**
     * Destructor.
     */
    virtual ~PhysicsConstraint();

    /**
     * Calculates the transform to be used as the offset (i.e. "frame in" 
     * parameter in Bullet terms) to the given constraint origin.
     */
    static btTransform getTransformOffset(const Node* node, const Vector3& origin);
    
    /**
     * Calculates the center of mass in world space of the given node.
     */
    static Vector3 getWorldCenterOfMass(const Node* node);

    /**
     * Offsets the given vector by the given node's center of mass.
     */
    static Vector3 offsetByCenterOfMass(const Node* node, const Vector3& v);

    /**
     * Pointer to the one rigid body bound by this constraint.
     */
    PhysicsRigidBody* _a;
    
    /**
     * Pointer to the other rigid body bound by this constraint.
     */
    PhysicsRigidBody* _b;
    
    /**
     * Pointer to the Bullet constraint.
     */
    btTypedConstraint* _constraint;
};






class PhysicsRigidBody;

/**
 * Defines a completely generic constraint between two
 * rigid bodies (or one rigid body and the world) where the
 * limits for all six degrees of freedom can be set individually.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Constraints
 */
class PhysicsGenericConstraint : public PhysicsConstraint
{
    friend class PhysicsController;

public:

    /**
     * Gets the rotation offset for the first rigid body in the constraint.
     *
     * @return The rotation offset.
     */
    inline const Quaternion& getRotationOffsetA() const;

    /**
     * Gets the rotation offset for the second rigid body in the constraint.
     *
     * @return The rotation offset.
     */
    inline const Quaternion& getRotationOffsetB() const;

    /**
     * Gets the translation offset for the first rigid body in the constraint.
     *
     * @return The translation offset.
     */
    inline const Vector3& getTranslationOffsetA() const;

    /**
     * Gets the translation offset for the second rigid body in the constraint.
     *
     * @return The translation offset.
     */
    inline const Vector3& getTranslationOffsetB() const;

    /**
     * Sets the lower angular limits (as Euler angle limits) along the constraint's local
     * X, Y, and Z axes using the values in the given vector.
     *
     * @param limits The lower angular limits (as Euler angle limits) along the local X, Y, and Z axes.
     */
    inline void setAngularLowerLimit(const Vector3& limits);

    /**
     * Sets the upper angular limits (as Euler angle limits) along the constraint's local
     * X, Y, and Z axes using the values in the given vector.
     *
     * @param limits The upper angular limits (as Euler angle limits) along the local X, Y, and Z axes.
     */
    inline void setAngularUpperLimit(const Vector3& limits);

    /**
     * Sets the lower linear limits along the constraint's local
     * X, Y, and Z axes using the values in the given vector.
     *
     * @param limits The lower linear limits along the local X, Y, and Z axes.
     */
    inline void setLinearLowerLimit(const Vector3& limits);

    /**
     * Sets the upper linear limits along the constraint's local
     * X, Y, and Z axes using the values in the given vector.
     *
     * @param limits The upper linear limits along the local X, Y, and Z axes.
     */
    inline void setLinearUpperLimit(const Vector3& limits);

    /**
     * Sets the rotation offset for the first rigid body in the constraint.
     *
     * @param rotationOffset The rotation offset.
     */
    inline void setRotationOffsetA(const Quaternion& rotationOffset);

    /**
     * Sets the rotation offset for the second rigid body in the constraint.
     *
     * @param rotationOffset The rotation offset.
     */
    inline void setRotationOffsetB(const Quaternion& rotationOffset);

    /**
     * Sets the translation offset for the first rigid body in the constraint.
     *
     * @param translationOffset The translation offset.
     */
    inline void setTranslationOffsetA(const Vector3& translationOffset);

    /**
     * Sets the translation offset for the second rigid body in the constraint.
     *
     * @param translationOffset The translation offset.
     */
    inline void setTranslationOffsetB(const Vector3& translationOffset);

protected:

    /**
     * Constructor.
     *
     * Note: This should only used by subclasses that do not want
     * the _constraint member variable to be initialized.
     */
    PhysicsGenericConstraint();

    /**
     * Creates a generic constraint so that the rigid body (or bodies) is
     * (are) constrained to its (their) current world position(s).
     *
     * @param a The first (possibly only) rigid body to constrain. If this is the only rigid
     *      body specified the constraint applies between it and the global physics world object.
     * @param b The second rigid body to constrain (optional).
     */
    PhysicsGenericConstraint(PhysicsRigidBody* a, PhysicsRigidBody* b);

    /**
     * Creates a generic constraint.
     *
     * @param a The first (possibly only) rigid body to constrain. If this is the only rigid
     *      body specified the constraint applies between it and the global physics world object.
     * @param rotationOffsetA The rotation offset for the first rigid body
     *      (in its local space) with respect to the constraint joint.
     * @param translationOffsetA The translation offset for the first rigid body
     *      (in its local space) with respect to the constraint joint.
     * @param b The second rigid body to constrain (optional).
     * @param rotationOffsetB The rotation offset for the second rigid body
     *      (in its local space) with respect to the constraint joint (optional).
     * @param translationOffsetB The translation offset for the second rigid body
     *      (in its local space) with respect to the constraint joint (optional).
     */
    PhysicsGenericConstraint(PhysicsRigidBody* a, const Quaternion& rotationOffsetA, const Vector3& translationOffsetA,
        PhysicsRigidBody* b, const Quaternion& rotationOffsetB, const Vector3& translationOffsetB);

    /**
     * Destructor.
     */
    virtual ~PhysicsGenericConstraint();

private:

    mutable Quaternion* _rotationOffsetA;
    mutable Quaternion* _rotationOffsetB;
    mutable Vector3* _translationOffsetA;
    mutable Vector3* _translationOffsetB;
};






/**
 * Defines a constraint where two rigid bodies 
 * (or one rigid body and the world) are bound together.
 *
 * This is similar in concept to parenting one node to another,
 * but can be used in specific situations for a more appropriate effect
 * Ex. for implementing sticky projectiles, etc.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Constraints
 */
class PhysicsFixedConstraint : public PhysicsGenericConstraint
{
    friend class PhysicsController;

protected:

    /**
     * @see PhysicsGenericConstraint
     */
    PhysicsFixedConstraint(PhysicsRigidBody* a, PhysicsRigidBody* b);

    /**
     * Destructor.
     */
    ~PhysicsFixedConstraint();

    // Note: We make these functions protected to prevent usage
    // (these are public in the base class, PhysicsGenericConstraint).

    /**
     * Protected to prevent usage.
     */
    inline void setAngularLowerLimit(const Vector3& limit);
    
    /**
     * Protected to prevent usage.
     */
    inline void setAngularUpperLimit(const Vector3& limit);
    
    /**
     * Protected to prevent usage.
     */
    inline void setLinearLowerLimit(const Vector3& limit);
    
    /**
     * Protected to prevent usage.
     */
    inline void setLinearUpperLimit(const Vector3& limit);
};








/**
 * Defines a hinge constraint between two rigid bodies
 * (or one rigid body and the world) where movement is
 * restricted to rotation about one axis.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Constraints
 */
class PhysicsHingeConstraint : public PhysicsConstraint
{
    friend class PhysicsController;

public:

    /**
     * Sets the limits (and optionally, some properties) for the hinge.
     * 
     * @param minAngle The minimum angle for the hinge.
     * @param maxAngle The maximum angle for the hinge.
     * @param bounciness The bounciness of the hinge (this is applied as
     *      a factor to the incoming velocity when a hinge limit is met in
     *      order to calculate the outgoing velocity-for example, 0.0 corresponds
     *      to no bounce and 1.0 corresponds to an outgoing velocity that is equal
     *      in magnitude to the incoming velocity).
     */
    void setLimits(float minAngle, float maxAngle, float bounciness = 1.0f);

private:
    /**
     * Creates a hinge constraint.
     * 
     * @param a The first (possibly only) rigid body to constrain. If this is the only rigid
     *      body specified the constraint applies between it and the global physics world object.
     * @param rotationOffsetA The rotation offset for the first rigid body 
     *      (in its local space) with respect to the constraint joint.
     * @param translationOffsetA The translation offset for the first rigid body
     *      (in its local space) with respect to the constraint joint.
     * @param b The second rigid body to constrain (optional).
     * @param rotationOffsetB The rotation offset for the second rigid body
     *      (in its local space) with respect to the constraint joint (optional).
     * @param translationOffsetB The translation offset for the second rigid body
     *      (in its local space) with respect to the constraint joint (optional).
     */
    PhysicsHingeConstraint(PhysicsRigidBody* a, const Quaternion& rotationOffsetA, const Vector3& translationOffsetA,
                           PhysicsRigidBody* b, const Quaternion& rotationOffsetB, const Vector3& translationOffsetB);

    /**
     * Destructor.
     */
    ~PhysicsHingeConstraint();
};






/**
 * Defines a ball-socket or point-to-point constraint
 * between two rigid bodies (or one rigid body and the world)
 * where rotation is unrestricted about the constraint joint (pivot point).
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Constraints
 */
class PhysicsSocketConstraint : public PhysicsConstraint
{
    friend class PhysicsController;

private:

    /**
     * Creates a socket constraint so that the rigid body (or bodies) is
     * (are) constrained using its (their) current world position(s) for
     * the translation offset(s) to the constraint.
     * 
     * @param a The first (possibly only) rigid body to constrain. If this is the only rigid
     *      body specified the constraint applies between it and the global physics world object.
     * @param b The second rigid body to constrain (optional).
     */
    PhysicsSocketConstraint(PhysicsRigidBody* a, PhysicsRigidBody* b);

    /**
     * Creates a socket constraint.
     * 
     * @param a The first (possibly only) rigid body to constrain. If this is the only rigid
     *      body specified the constraint applies between it and the global physics world object.
     * @param translationOffsetA The translation offset for the first rigid body
     *      (in its local space) with respect to the constraint joint.
     * @param b The second rigid body to constrain (optional).
     * @param translationOffsetB The translation offset for the second rigid body
     *      (in its local space) with respect to the constraint joint (optional).
     */
    PhysicsSocketConstraint(PhysicsRigidBody* a, const Vector3& translationOffsetA, 
                            PhysicsRigidBody* b, const Vector3& translationOffsetB);

    /**
     * Destructor.
     */
    ~PhysicsSocketConstraint();
};





/**
 * Defines a generic spring constraint between two
 * rigid bodies (or one rigid body and the world)
 * where the spring strength and damping can be set
 * for all six degrees of freedom.
 *
 * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Constraints
 */
class PhysicsSpringConstraint : public PhysicsGenericConstraint
{
    friend class PhysicsController;

public:

    /**
     * Sets the angular damping along the constraint's local X axis.
     * 
     * @param damping The angular damping value.
     */
    inline void setAngularDampingX(float damping);

    /**
     * Sets the angular damping along the constraint's local Y axis.
     * 
     * @param damping The angular damping value.
     */
    inline void setAngularDampingY(float damping);

    /**
     * Sets the angular damping along the constraint's local Z axis.
     * 
     * @param damping The angular damping value.
     */
    inline void setAngularDampingZ(float damping);

    /**
     * Sets the angular strength along the constraint's local X axis.
     *
     * Note: setting the strength to a non-zero value enables the
     * spring for angular movement about the X axis (setting to zero disables it).
     * 
     * @param strength The angular strength value.
     */
    inline void setAngularStrengthX(float strength);

    /**
     * Sets the angular strength along the constraint's local Y axis.
     *
     * Note: setting the strength to a non-zero value enables the
     * spring for angular movement about the Y axis (setting to zero disables it).
     * 
     * @param strength The angular strength value.
     */
    inline void setAngularStrengthY(float strength);

    /**
     * Sets the angular strength along the constraint's local Z axis.
     *
     * Note: setting the strength to a non-zero value enables the
     * spring for angular movement about the Z axis (setting to zero disables it).
     * 
     * @param strength The angular strength value.
     */
    inline void setAngularStrengthZ(float strength);

    /**
     * Sets the linear damping along the constraint's local X axis.
     * 
     * @param damping The linear damping value.
     */
    inline void setLinearDampingX(float damping);

    /**
     * Sets the linear damping along the constraint's local Y axis.
     * 
     * @param damping The linear damping value.
     */
    inline void setLinearDampingY(float damping);

    /**
     * Sets the linear damping along the constraint's local Z axis.
     * 
     * @param damping The linear damping value.
     */
    inline void setLinearDampingZ(float damping);

    /**
     * Sets the linear strength along the constraint's local X axis.
     *
     * Note: setting the strength to a non-zero value enables the
     * spring for linear movement along the X axis (setting to zero disables it).
     * 
     * @param strength The linear strength value.
     */
    inline void setLinearStrengthX(float strength);

    /**
     * Sets the linear strength along the constraint's local Y axis.
     *
     * Note: setting the strength to a non-zero value enables the
     * spring for linear movement along the Y axis (setting to zero disables it).
     * 
     * @param strength The linear strength value.
     */
    inline void setLinearStrengthY(float strength);

    /**
     * Sets the linear strength along the constraint's local Z axis.
     *
     * Note: setting the strength to a non-zero value enables the
     * spring for linear movement along the Z axis (setting to zero disables it).
     * 
     * @param strength The linear strength value.
     */
    inline void setLinearStrengthZ(float strength);

private:

    // Represents the different properties that
    // can be set on the spring constraint.
    // 
    // (Note: the values map to the index parameter
    // used in the member functions of the Bullet
    // class btGeneric6DofSpringConstraint.)
    enum SpringProperty
    {
        LINEAR_X = 0,
        LINEAR_Y,
        LINEAR_Z,
        ANGULAR_X,
        ANGULAR_Y,
        ANGULAR_Z
    };

    /**
     * Creates a spring constraint so that the rigid body (or bodies) is
     * (are) constrained using its (their) current world position(s) for
     * the translation offset(s) to the constraint.
     * 
     * @param a The first (possibly only) rigid body to constrain. If this is the only rigid
     *      body specified the constraint applies between it and the global physics world object.
     * @param b The second rigid body to constrain (optional).
     */
    PhysicsSpringConstraint(PhysicsRigidBody* a, PhysicsRigidBody* b);

    /**
     * Creates a spring constraint.
     * 
     * @param a The first (possibly only) rigid body to constrain. If this is the only rigid
     *      body specified the constraint applies between it and the global physics world object.
     * @param rotationOffsetA The rotation offset for the first rigid body 
     *      (in its local space) with respect to the constraint joint.
     * @param translationOffsetA The translation offset for the first rigid body
     *      (in its local space) with respect to the constraint joint.
     * @param b The second rigid body to constrain (optional).
     * @param rotationOffsetB The rotation offset for the second rigid body
     *      (in its local space) with respect to the constraint joint (optional).
     * @param translationOffsetB The translation offset for the second rigid body
     *      (in its local space) with respect to the constraint joint (optional).
     */
    PhysicsSpringConstraint(PhysicsRigidBody* a, const Quaternion& rotationOffsetA, const Vector3& translationOffsetA,
                            PhysicsRigidBody* b, const Quaternion& rotationOffsetB, const Vector3& translationOffsetB);

    /**
     * Destructor.
     */
    ~PhysicsSpringConstraint();

    // Sets the strength for the given angular/linear 
    // X/Y/Z axis combination determined by the given index.
    // 
    // See the Bullet class btGeneric6DofSpringConstraint
    // for more information.
    void setStrength(SpringProperty property, float strength);

    // Sets the damping for the given angular/linear 
    // X/Y/Z axis combination determined by the given index.
    // 
    // See the Bullet class btGeneric6DofSpringConstraint
    // for more information.
    void setDamping(SpringProperty property, float damping);
};




}


#include "PhysicsConstraint.inl"

#endif
