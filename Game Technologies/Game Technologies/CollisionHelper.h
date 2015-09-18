#pragma once

#include "PhysicsNode.h"

//Behold, the many types of collisions that are being simulated.
class CollisionHelper
{
public:
	//Detection methods
	static bool SweptBBSphereCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data = NULL);
	static int	SphereWellCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data = NULL);
	static bool SphereInactiveWellCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data = NULL);
	static bool SphereSphereCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data = NULL);
	static bool SpherePlaneCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data = NULL);
	//Resolution methods
	static void AddCollisionImpulse(PhysicsNode& p0, PhysicsNode& p1, CollisionData& data);
	static void AddGravitationalImpulse(PhysicsNode& p0, PhysicsNode& p1, CollisionData& data);
	static bool AbsorbCollidingSphere(PhysicsNode& p0, PhysicsNode& p1, CollisionData& data);
};

inline float LengthSq(Vector3 v) {
	return Vector3::Dot(v, v);
}
