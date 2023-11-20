#include "base/Base.h"
#include "PhysicsConstraint.h"
#include "platform/Toolkit.h"
#include "scene/Node.h"
#include "PhysicsRigidBody.h"
#include "PhysicsController.h"

namespace mgp
{

PhysicsConstraint::PhysicsConstraint(PhysicsRigidBody* a, PhysicsRigidBody* b)
    : _a(a), _b(b), _constraint(NULL)
{
}

PhysicsConstraint::~PhysicsConstraint()
{
    // Remove the physics rigid bodies' references to this constraint.
    if (_a)
        _a->removeConstraint(this);
    if (_b)
        _b->removeConstraint(this);

    // Remove the constraint from the physics world and delete the Bullet object.
    GP_ASSERT(PhysicsController::cur());
    PhysicsController::cur()->removeConstraint(this);
    SAFE_DELETE(_constraint);
}

Vector3 PhysicsConstraint::centerOfMassMidpoint(const Node* a, const Node* b)
{
    GP_ASSERT(a);
    GP_ASSERT(b);

    Vector3 tA, tB;
    a->getWorldMatrix().getTranslation(&tA);
    b->getWorldMatrix().getTranslation(&tB);

    tA = getWorldCenterOfMass(a);
    tB = getWorldCenterOfMass(b);
    
    Vector3 d(tA, tB);
    d.scale(0.5f);
    Vector3 c(tA);
    c.add(d);

    return c;
}

Quaternion PhysicsConstraint::getRotationOffset(const Node* node, const Vector3& point)
{
    GP_ASSERT(node);

    // Create a translation matrix that translates to the given origin.
    Matrix m;
    Matrix::createTranslation(point, &m);

    // Calculate the rotation offset to the rigid body by transforming 
    // the translation matrix above into the rigid body's local space 
    // (multiply by the inverse world matrix) and extracting the rotation.
    Matrix mi;
    node->getWorldMatrix().invert(&mi);
    mi.multiply(m);
    
    Quaternion r;
    mi.getRotation(&r);

    return r;
}

Vector3 PhysicsConstraint::getTranslationOffset(const Node* node, const Vector3& point)
{
    GP_ASSERT(node);

    // Create a translation matrix that translates to the given origin.
    Matrix m;
    Matrix::createTranslation(point, &m);

    // Calculate the translation offset to the rigid body by transforming 
    // the translation matrix above into the rigid body's local space 
    // (multiply by the inverse world matrix) and extracting the translation.
    Matrix mi;
    node->getWorldMatrix().invert(&mi);
    mi.multiply(m);
    
    Vector3 t;
    mi.getTranslation(&t);

    Vector3 s;
    node->getWorldMatrix().getScale(&s);

    t.x *= s.x;
    t.y *= s.y;
    t.z *= s.z;
    
    t = offsetByCenterOfMass(node, t);

    return t;
}

btTransform PhysicsConstraint::getTransformOffset(const Node* node, const Vector3& origin)
{
    GP_ASSERT(node);

    // Create a translation matrix that translates to the given origin.
    Matrix m;
    Matrix::createTranslation(origin, &m);

    // Calculate the translation and rotation offset to the rigid body
    // by transforming the translation matrix above into the rigid body's
    // local space (multiply by the inverse world matrix and extract components).
    Matrix mi;
    node->getWorldMatrix().invert(&mi);
    mi.multiply(m);

    Quaternion r;
    mi.getRotation(&r);
    
    Vector3 t;
    mi.getTranslation(&t);

    Vector3 s;
    node->getWorldMatrix().getScale(&s);

    t.x *= s.x;
    t.y *= s.y;
    t.z *= s.z;
    
    t = offsetByCenterOfMass(node, t);

    return btTransform(BQ(r), BV(t));
}

Vector3 PhysicsConstraint::getWorldCenterOfMass(const Node* node)
{
    GP_ASSERT(node);

    const BoundingSphere& sphere = node->getBoundingSphere();
    if (!(sphere.center.isZero() && sphere.radius == 0))
    {
        // The world-space center of mass is the sphere's center.
        return sphere.center;
    }

    // Warn the user that the node has no bounding volume.
    GP_WARN("Node %s' has no bounding volume - center of mass is defaulting to local coordinate origin.", node->getName());

    Vector3 center;
    node->getWorldMatrix().transformPoint(&center);
    return center;
}

Vector3 PhysicsConstraint::offsetByCenterOfMass(const Node* node, const Vector3& v)
{
    GP_ASSERT(node);
    PhysicsCollisionObject* obj = node->getComponent<PhysicsCollisionObject>();
    GP_ASSERT(obj && obj->_motionState);
    btVector3 centerOfMassOffset = obj->_motionState->_centerOfMassOffset.getOrigin();
    return Vector3(v.x + centerOfMassOffset.x(), v.y + centerOfMassOffset.y(), v.z + centerOfMassOffset.z());
}



PhysicsFixedConstraint::PhysicsFixedConstraint(PhysicsRigidBody* a, PhysicsRigidBody* b)
    : PhysicsGenericConstraint(a, b)
{
    PhysicsGenericConstraint::setAngularLowerLimit(Vector3(0.0f, 0.0f, 0.0f));
    PhysicsGenericConstraint::setAngularUpperLimit(Vector3(0.0f, 0.0f, 0.0f));
    PhysicsGenericConstraint::setLinearLowerLimit(Vector3(0.0f, 0.0f, 0.0f));
    PhysicsGenericConstraint::setLinearUpperLimit(Vector3(0.0f, 0.0f, 0.0f));
}

PhysicsFixedConstraint::~PhysicsFixedConstraint()
{
    // Not used.
}





PhysicsGenericConstraint::PhysicsGenericConstraint()
    : PhysicsConstraint(NULL, NULL), _rotationOffsetA(NULL), _rotationOffsetB(NULL),
    _translationOffsetA(NULL), _translationOffsetB(NULL)
{
    // Not used.
}

PhysicsGenericConstraint::PhysicsGenericConstraint(PhysicsRigidBody* a, PhysicsRigidBody* b)
    : PhysicsConstraint(a, b), _rotationOffsetA(NULL), _rotationOffsetB(NULL),
    _translationOffsetA(NULL), _translationOffsetB(NULL)
{
    GP_ASSERT(a && a->_body && a->getNode());

    if (b)
    {
        GP_ASSERT(b->_body && b->getNode());
        Vector3 origin = centerOfMassMidpoint(a->getNode(), b->getNode());
        _constraint = bullet_new<btGeneric6DofConstraint>(*a->_body, *b->_body, getTransformOffset(a->getNode(), origin), getTransformOffset(b->getNode(), origin), true);
    }
    else
    {
        _constraint = bullet_new<btGeneric6DofConstraint>(*a->_body, btTransform::getIdentity(), true);
    }
}

PhysicsGenericConstraint::PhysicsGenericConstraint(PhysicsRigidBody* a, const Quaternion& rotationOffsetA, const Vector3& translationOffsetA,
    PhysicsRigidBody* b, const Quaternion& rotationOffsetB, const Vector3& translationOffsetB)
    : PhysicsConstraint(a, b), _rotationOffsetA(NULL), _rotationOffsetB(NULL), _translationOffsetA(NULL), _translationOffsetB(NULL)
{
    GP_ASSERT(a && a->_body && a->getNode());

    // Take scale into account for the first node's translation offset.
    Vector3 sA;
    a->getNode()->getWorldMatrix().getScale(&sA);
    Vector3 tA(translationOffsetA.x * sA.x, translationOffsetA.y * sA.y, translationOffsetA.z * sA.z);

    if (b)
    {
        GP_ASSERT(b->_body && b->getNode());

        // Take scale into account for the second node's translation offset.
        Vector3 sB;
        b->getNode()->getWorldMatrix().getScale(&sB);
        Vector3 tB(translationOffsetB.x * sB.x, translationOffsetB.y * sB.y, translationOffsetB.z * sB.z);

        btTransform frameInA(BQ(rotationOffsetA), BV(tA));
        btTransform frameInB(BQ(rotationOffsetB), BV(tB));
        _constraint = bullet_new<btGeneric6DofConstraint>(*a->_body, *b->_body, frameInA, frameInB, true);
    }
    else
    {
        btTransform frameInA(BQ(rotationOffsetA), BV(tA));
        _constraint = bullet_new<btGeneric6DofConstraint>(*a->_body, frameInA, true);
    }
}

PhysicsGenericConstraint::~PhysicsGenericConstraint()
{
    SAFE_DELETE(_rotationOffsetA);
    SAFE_DELETE(_rotationOffsetB);
    SAFE_DELETE(_translationOffsetA);
    SAFE_DELETE(_translationOffsetB);
}

void PhysicsHingeConstraint::setLimits(float minAngle, float maxAngle, float bounciness)
{
    // Use the defaults for softness (0.9) and biasFactor (0.3).
    GP_ASSERT(_constraint);
    ((btHingeConstraint*)_constraint)->setLimit(minAngle, maxAngle, 0.9f, 0.3f, bounciness);
}

PhysicsHingeConstraint::PhysicsHingeConstraint(PhysicsRigidBody* a, const Quaternion& rotationOffsetA, const Vector3& translationOffsetA,
    PhysicsRigidBody* b, const Quaternion& rotationOffsetB, const Vector3& translationOffsetB)
    : PhysicsConstraint(a, b)
{
    GP_ASSERT(a && a->_body && a->getNode());

    // Take scale into account for the first node's translation offset.
    Vector3 sA;
    a->getNode()->getWorldMatrix().getScale(&sA);
    Vector3 tA(translationOffsetA.x * sA.x, translationOffsetA.y * sA.y, translationOffsetA.z * sA.z);

    if (b)
    {
        GP_ASSERT(b->_body && b->getNode());

        // Take scale into account for the second node's translation offset.
        Vector3 sB;
        b->getNode()->getWorldMatrix().getScale(&sB);
        Vector3 tB(translationOffsetB.x * sB.x, translationOffsetB.y * sB.y, translationOffsetB.z * sB.z);

        btTransform frameInA(BQ(rotationOffsetA), BV(tA));
        btTransform frameInB(BQ(rotationOffsetB), BV(tB));
        _constraint = bullet_new<btHingeConstraint>(*a->_body, *b->_body, frameInA, frameInB);
    }
    else
    {
        btTransform frameInA(BQ(rotationOffsetA), BV(tA));
        _constraint = bullet_new<btHingeConstraint>(*a->_body, frameInA);
    }
}

    
PhysicsHingeConstraint::~PhysicsHingeConstraint()
{
    // Unused
}







PhysicsSocketConstraint::PhysicsSocketConstraint(PhysicsRigidBody* a, PhysicsRigidBody* b)
    : PhysicsConstraint(a, b)
{
    GP_ASSERT(a && a->_body && a->getNode());
    if (b)
    {
        GP_ASSERT(b->_body && b->getNode());
        Vector3 origin = centerOfMassMidpoint(a->getNode(), b->getNode());
        btTransform frameInA = getTransformOffset(a->getNode(), origin);
        btTransform frameInB = getTransformOffset(b->getNode(), origin);

        _constraint = bullet_new<btPoint2PointConstraint>(*a->_body, *b->_body, frameInA.getOrigin(), frameInB.getOrigin());
    }
    else
    {
        _constraint = bullet_new<btPoint2PointConstraint>(*a->_body, btVector3(0.0f, 0.0f, 0.0f));
    }
}

PhysicsSocketConstraint::PhysicsSocketConstraint(PhysicsRigidBody* a, const Vector3& translationOffsetA, 
                                                 PhysicsRigidBody* b, const Vector3& translationOffsetB)
    : PhysicsConstraint(a, b)
{
    GP_ASSERT(a && a->_body && a->getNode());

    // Take scale into account for the first node's translation offset.
    Vector3 sA;
    a->getNode()->getWorldMatrix().getScale(&sA);
    Vector3 tA(translationOffsetA.x * sA.x, translationOffsetA.y * sA.y, translationOffsetA.z * sA.z);

    if (b)
    {
        GP_ASSERT(b->_body && b->getNode());

        // Take scale into account for the second node's translation offset.
        Vector3 sB;
        b->getNode()->getWorldMatrix().getScale(&sB);
        Vector3 tB(translationOffsetB.x * sB.x, translationOffsetB.y * sB.y, translationOffsetB.z * sB.z);

        _constraint = bullet_new<btPoint2PointConstraint>(*a->_body, *b->_body, BV(tA), BV(tB));
    }
    else
    {
        _constraint = bullet_new<btPoint2PointConstraint>(*a->_body, BV(tA));
    }
}

PhysicsSocketConstraint::~PhysicsSocketConstraint()
{
    // Used
}





PhysicsSpringConstraint::PhysicsSpringConstraint(PhysicsRigidBody* a, PhysicsRigidBody* b)
{
    GP_ASSERT(a && a->_body);
    GP_ASSERT(b && b->_body);

    // Initialize the physics rigid body references since we don't call the PhysicsConstraint constructor that does it properly automatically.
    _a = a;
    _b = b;

    Vector3 origin = centerOfMassMidpoint(a->getNode(), b->getNode());
    _constraint = bullet_new<btGeneric6DofSpringConstraint>(*a->_body, *b->_body, getTransformOffset(a->getNode(), origin), getTransformOffset(b->getNode(), origin), true);
}

PhysicsSpringConstraint::PhysicsSpringConstraint(PhysicsRigidBody* a, const Quaternion& rotationOffsetA, const Vector3& translationOffsetA,
    PhysicsRigidBody* b, const Quaternion& rotationOffsetB, const Vector3& translationOffsetB)
{
    GP_ASSERT(a && a->_body && a->getNode());
    GP_ASSERT(b && b->_body && b->getNode());

    // Initialize the physics rigid body references since we don't call the PhysicsConstraint constructor that does it properly automatically.
    _a = a;
    _b = b;

    // Take scale into account for the translation offsets.
    Vector3 sA;
    a->getNode()->getWorldMatrix().getScale(&sA);
    Vector3 tA(translationOffsetA.x * sA.x, translationOffsetA.y * sA.y, translationOffsetA.z * sA.z);

    Vector3 sB;
    b->getNode()->getWorldMatrix().getScale(&sB);
    Vector3 tB(translationOffsetB.x * sB.x, translationOffsetB.y * sB.y, translationOffsetB.z * sB.z);

    btTransform frameInA(BQ(rotationOffsetA), BV(tA));
    btTransform frameInB(BQ(rotationOffsetB), BV(tB));
    _constraint = bullet_new<btGeneric6DofSpringConstraint>(*a->_body, *b->_body, frameInA, frameInB, true);
}

PhysicsSpringConstraint::~PhysicsSpringConstraint()
{
    // Used
}

void PhysicsSpringConstraint::setStrength(SpringProperty property, float strength)
{
    GP_ASSERT(_constraint);
    if (strength < MATH_EPSILON)
        ((btGeneric6DofSpringConstraint*)_constraint)->enableSpring(property, false);
    else
    {
        ((btGeneric6DofSpringConstraint*)_constraint)->enableSpring(property, true);
        ((btGeneric6DofSpringConstraint*)_constraint)->setStiffness(property, strength);
        ((btGeneric6DofSpringConstraint*)_constraint)->setEquilibriumPoint(property);
    }
}

void PhysicsSpringConstraint::setDamping(SpringProperty property, float damping)
{
    GP_ASSERT(_constraint);
    ((btGeneric6DofSpringConstraint*)_constraint)->setDamping(property, damping);
    ((btGeneric6DofSpringConstraint*)_constraint)->setEquilibriumPoint(property);
}

}
