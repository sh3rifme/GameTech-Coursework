#pragma once

#include "GameEntity.h"

class Boid : public GameEntity
{
public:
	Boid(float aW, float cW, float sW): alignWeight(aW), cohWeight(cW), sepWeight(sW){}
	~Boid(void){}

	void Update();

	float GetAlignW()			{ return alignWeight; }
	float GetCohW()				{ return cohWeight; }
	float GetSepW()				{ return sepWeight; }

	void SetAlign(float arg)	{ alignWeight = arg; }
	void SetCohes(float arg)	{ cohWeight = arg; }
	void SetSep(float arg)		{ sepWeight = arg; }

	Vector3 GetSeparation(Boid* b) { return b->GetPhysicsNode().GetPosition() - physicsNode->GetPosition(); }

protected:
	float alignWeight;
	float cohWeight;
	float sepWeight;
};

