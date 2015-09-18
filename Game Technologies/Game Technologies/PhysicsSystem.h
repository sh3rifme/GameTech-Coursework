#pragma once

#include "CollisionHelper.h"
#include <vector>
#include <mutex>

using std::vector;

#define X_MAX 4000
#define Y_MAX 2000
#define Z_MAX 4000

class PhysicsSystem	{
public:
	friend class GameClass;

	void		Update(float msec);

	void		BroadPhaseCollisions();
	void		NarrowPhaseCollisions();
	void		BruteForceCollisions();

	//Statics
	static void Initialise() {
		instance = new PhysicsSystem();
	}

	static void Destroy() {
		delete instance;
	}

	static PhysicsSystem& GetPhysicsSystem() {
		return *instance;
	}

	void	AddNode(PhysicsNode* n);

	void	RemoveNode(PhysicsNode* n);

	int		GetNodeNum();
	int		GetCollisionCount()			{ return collisionCount; }

	void	ToggleGravity();

protected:
	PhysicsSystem(void);
	~PhysicsSystem(void);

	int						collisionCount;

	mutex					muLan;

	vector<PhysicsNode*>	sp[21*11*21];
	vector<PhysicsNode*>	collidesWithGround;
	vector<PhysicsNode*>	collidesWithWall;
	vector<PhysicsNode*>	allNodes;
	vector<PhysicsNode*>	cullNodes;

	bool					gravity;

//Statics
	static PhysicsSystem*	instance;
};

