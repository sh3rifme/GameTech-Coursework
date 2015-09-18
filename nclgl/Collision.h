/*
Here we see the many types of collision volumes that can be simulated in the physics engine.
*/
#pragma once
#include "Vector3.h"

enum CollisionVolumeType {
	COLLISION_SPHERE,
	COLLISION_AABB,
	COLLISION_PLANE,
	COLLISION_WELL,
	COLLISION_NONE
};

class CollisionVolume {
public:
	virtual CollisionVolumeType GetType() const = 0;
};

class CollisionSphere : public CollisionVolume {
public:

	CollisionSphere(float radius): radius(radius) {}

	CollisionVolumeType GetType() const { return COLLISION_SPHERE; }
	float GetRadius() const				{ return radius; }

protected:
	float radius;
};

class CollisionWell : public CollisionSphere {
public:
	CollisionWell(float radius) : CollisionSphere(radius) {
		gravityRadius = 100*radius;
		absorbs = 0;
	}
	CollisionVolumeType GetType() const { return COLLISION_WELL; }
	float	GetGravityRadius() const	{ return gravityRadius; }
	int		GetBumps() const			{ return absorbs; }
	void	SetRadius(float r)			{ radius = r; gravityRadius = 3*r; }
	void	IncAbsorbs()				{ ++absorbs; }
protected:
	int absorbs;
	float gravityRadius;
};

class CollisionPlane : public CollisionVolume {
public:

	CollisionPlane(Vector3 normal, float distance): distance(distance), normal(normal) {}

	CollisionVolumeType GetType() const { return COLLISION_PLANE; }
	Vector3 GetNormal() const			{ return normal; }
	float GetDistance() const			{ return distance; }

private:
	float distance;
	Vector3 normal;
};

class CollisionBB : public CollisionVolume {
public:
	CollisionBB(Vector3 dims): dimensions(dims) {}

	CollisionVolumeType GetType() const { return COLLISION_AABB; }
	Vector3 GetDimensions() const		{ return dimensions; }

private:
	Vector3 dimensions;
};

class NoCollide : public CollisionVolume {
	CollisionVolumeType GetType() const { return COLLISION_NONE; }
};


class CollisionData {
public:
	Vector3 m_point;
	Vector3 m_normal;
	float m_penetration;
};