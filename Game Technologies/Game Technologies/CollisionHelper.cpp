#include "CollisionHelper.h"

//Was going to use this to simulate cylinder collisions, but other things got in the way.
bool CollisionHelper::SweptBBSphereCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data) {
	CollisionSphere& s0 = *(CollisionSphere*)p0.GetCollisionVolume();
	CollisionBB& aabb = *(CollisionBB*)p0.GetCollisionVolume();

	return false;
}

//Much gravity, many attraction, so physics.
int CollisionHelper::SphereWellCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data) {
	CollisionSphere& s0 = *(CollisionSphere*)p1.GetCollisionVolume();
	CollisionWell& well = *(CollisionWell*)p0.GetCollisionVolume();

	Vector3 normal = p0.GetPosition() - p1.GetPosition();
	const float distSq = LengthSq(normal);
	const float sumRadius = s0.GetRadius() + well.GetRadius();

	//First be sure that the object has not collided with the inner sphere.
	if (distSq < sumRadius * sumRadius) {
		if (data) {
			data->m_penetration = sumRadius - sqrtf(distSq);
			normal.Normalise();
			data->m_normal = normal;
			data->m_point = p0.GetPosition() - normal * (s0.GetRadius() - data->m_penetration * 0.5f);
		}
		return 1;
	}
	//If not, check if it's in the gravity well.
	const float sumGravRadius = s0.GetRadius() + well.GetGravityRadius();
	if (distSq < sumGravRadius * sumGravRadius) {
		if (data) {
			data->m_penetration = sumGravRadius - sqrtf(distSq);
			normal.Normalise();
			data->m_normal = normal;
			data->m_point = p0.GetPosition() - normal * (s0.GetRadius() - data->m_penetration * 0.5f);
		}
		return 2;
	}
	return 0;
}

bool CollisionHelper::SphereInactiveWellCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data) {
	CollisionSphere& s0 = *(CollisionSphere*)p1.GetCollisionVolume();
	CollisionWell& well = *(CollisionWell*)p0.GetCollisionVolume();

	Vector3 normal = p0.GetPosition() - p1.GetPosition();
	const float distSq = LengthSq(normal);
	const float sumRadius = s0.GetRadius() + well.GetRadius();

	if (distSq < sumRadius * sumRadius) {
		if (data) {
			data->m_penetration = sumRadius - sqrtf(distSq);
			normal.Normalise();
			data->m_normal = normal;
			data->m_point = p0.GetPosition() - normal * (s0.GetRadius() - data->m_penetration * 0.5f);
		}
		return true;
	}
	return false;
}

//Nothing fancy going on here.
bool CollisionHelper::SphereSphereCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data) {
	CollisionSphere& s0 = *(CollisionSphere*)p0.GetCollisionVolume();
	CollisionSphere& s1 = *(CollisionSphere*)p1.GetCollisionVolume();

	Vector3 normal = p0.GetPosition() - p1.GetPosition();
	const float distSq = LengthSq(normal);
	const float sumRadius = s0.GetRadius() + s1.GetRadius();

	if (distSq < sumRadius * sumRadius) {
		if (data) {
			data->m_penetration = sumRadius - sqrtf(distSq);
			normal.Normalise();
			data->m_normal = normal;
			data->m_point = p0.GetPosition() - normal * (s0.GetRadius() - data->m_penetration * 0.5f);
		}
		return true;
	}
	return false;
}

//Much of the same here.
bool CollisionHelper::SpherePlaneCollision(PhysicsNode& p0, PhysicsNode& p1, CollisionData* data) {
	CollisionSphere& s0 = *(CollisionSphere*)p0.GetCollisionVolume();
	CollisionPlane& plane = *(CollisionPlane*)p1.GetCollisionVolume();

	float separation = Vector3::Dot(p0.GetPosition(), plane.GetNormal()) - plane.GetDistance();

	if (separation > s0.GetRadius()) {
		return false;
	}
	if (data) {
		data->m_penetration = s0.GetRadius() - separation;
		data->m_normal = plane.GetNormal();
		data->m_point = p0.GetPosition() - plane.GetNormal()*separation;
	}

	return true;
}

//And again.
void CollisionHelper::AddCollisionImpulse(PhysicsNode& p0, PhysicsNode& p1, CollisionData& data) {
	if (p0.GetInverseMass() + p1.GetInverseMass() == 0.0f) return;

	Vector3 r0 = data.m_point - p0.GetPosition();
	Vector3 r1 = data.m_point - p1.GetPosition();
	
	Vector3 v0 = p0.GetLinearVelocity() + Vector3::Cross(p0.GetAngularVelocity(), r0);
	Vector3 v1 = p1.GetLinearVelocity() + Vector3::Cross(p1.GetAngularVelocity(), r1);

	Vector3 dv = v0 - v1;

	float relMov = -Vector3::Dot(dv, data.m_normal);
	if (relMov < -0.01f) return;
	
	//The only alteration i have made to this code, moving the objects out of collision before resolution.
	if (!p0.Anchored())
		p0.SetPosition(p0.GetPosition() + data.m_normal * data.m_penetration);
	if (!p1.Anchored())
		p1.SetPosition(p1.GetPosition() + data.m_normal * data.m_penetration);
	{
		//Elasticity.
		float e = 0.6f;
		float normDiv = (p0.GetInverseMass() + p1.GetInverseMass()) +
			Vector3::Dot(data.m_normal,
				Vector3::Cross(p0.GetInverseInertia()*Vector3::Cross(r0, data.m_normal), r0) +
				Vector3::Cross(p1.GetInverseInertia()*Vector3::Cross(r1, data.m_normal), r1));
		float jn = -1*(1+e)*Vector3::Dot(dv, data.m_normal)/normDiv;

		jn = jn + (data.m_penetration*0.01f);

		Vector3 l0 = p0.GetLinearVelocity() + data.m_normal*(jn*p0.GetInverseMass());
		p0.SetLinearVelocity(l0);
		Vector3 a0 = p0.GetAngularVelocity() + p0.GetInverseInertia()* Vector3::Cross(r0, data.m_normal * jn);
		p0.SetAngularVelocity(a0);

		Vector3 l1 = p1.GetLinearVelocity() - data.m_normal*(jn*p1.GetInverseMass());
		p1.SetLinearVelocity(l1);
		Vector3 a1 = p1.GetAngularVelocity() - p1.GetInverseInertia()* Vector3::Cross(r1, data.m_normal * jn);
		p1.SetAngularVelocity(a1);
	}

	{
		Vector3 tangent = dv - data.m_normal*Vector3::Dot(dv, data.m_normal);
		tangent.Normalise();
		float tangDiv = (p0.GetInverseMass() + p1.GetInverseMass()) +
			Vector3::Dot(tangent,
			Vector3::Cross( p0.GetInverseInertia()* Vector3::Cross(r0, tangent), r0) +
			Vector3::Cross( p1.GetInverseInertia()* Vector3::Cross(r1, tangent), r1));

		float jt = -1* Vector3::Dot(dv, tangent) / tangDiv;

		Vector3 l0 = p0.GetLinearVelocity() + tangent*(jt*p0.GetInverseMass());
		p0.SetLinearVelocity(l0);
		Vector3 a0 = p0.GetAngularVelocity() + p0.GetInverseInertia()* Vector3::Cross(r0, tangent * jt);
		p0.SetAngularVelocity(a0);

		Vector3 l1 = p1.GetLinearVelocity() - tangent*(jt*p1.GetInverseMass());
		p1.SetLinearVelocity(l1);
		Vector3 a1 = p1.GetAngularVelocity() - p1.GetInverseInertia()* Vector3::Cross(r1, tangent * jt);
		p1.SetAngularVelocity(a1);
	}
}

//Applies a constant force towards the center of the attractor, not real gravity, but it looks the part (ish).
void CollisionHelper::AddGravitationalImpulse(PhysicsNode& p0, PhysicsNode& p1, CollisionData& data) {
	Vector3 normalNorm = data.m_normal;
	normalNorm.Normalise();

	p1.ApplyForce(normalNorm * 700.0f , Vector3(0));
}


bool CollisionHelper::AbsorbCollidingSphere(PhysicsNode& p0, PhysicsNode& p1, CollisionData& data) {
	CollisionWell& well = *(CollisionWell*)p0.GetCollisionVolume();
	CollisionSphere& sp = *(CollisionSphere*)p1.GetCollisionVolume();
	
	//If the sphere is inside the other, return true to resolve the absorbtion.
	if (data.m_penetration * 5 <= well.GetRadius() * 0.1) {
		return true;
	}
	//If not, suck it in, check again next update.
	Vector3 normalNorm = data.m_normal;
	normalNorm.Normalise();

	p1.ApplyForce(normalNorm * 900.0f , Vector3(0));
	return false;
}