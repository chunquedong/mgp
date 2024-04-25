

## 物理引擎

通过setCollisionObject创建物理对象。
```
PhysicsRigidBody::Parameters params;
params.mass = 5;
PhysicsCollisionObject* collisionObject = PhysicsCollisionObject::setCollisionObject(modelNode, PhysicsCollisionObject::RIGID_BODY,
    PhysicsCollisionShape::box(), &params);
```

类型：
- RIGID_BODY 刚体
- CHARACTER 角色
- GHOST_OBJECT 幽灵体
- VEHICLE 载具
- VEHICLE_WHEEL 轮子

形状：
- SHAPE_BOX,
- SHAPE_SPHERE,
- SHAPE_CAPSULE,
- SHAPE_MESH,
- SHAPE_HEIGHTFIELD

