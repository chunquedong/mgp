#pragma once

#ifdef GP_USE_MEM_LEAK_DETECTION
#undef new
#endif

// Bullet Physics
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#define BV(v) (btVector3((v).x, (v).y, (v).z))
#define BQ(q) (btQuaternion((q).x, (q).y, (q).z, (q).w))

#ifdef GP_USE_MEM_LEAK_DETECTION
#define new DEBUG_NEW
#endif