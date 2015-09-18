#pragma once
#include "GameClass.h"
#include "Boid.h"
#include "../../nclgl/Camera.h"
#include "../../nclgl/CubeRobot.h"

#define SEARCH_RAD 1000.0f

class MyGame : public GameClass	{
public:
	MyGame();
	~MyGame(void);

	virtual void UpdateGame(float msec);

	void ShootSphere(float size);

	void ToggleGravity();

	void IncCohesion()			{ if (cohesion < 1.0f) cohesion += 0.1; }
	void IncSep()				{ if (seperation < 1.0f) seperation += 0.1; }
	void IncAlign()				{ if (align < 1.0f) align += 0.1; }

	void DecCohesion()			{ if (cohesion > 0.0f) cohesion -= 0.1; }
	void DecSep()				{ if (seperation > 0.0f) seperation -= 0.1; }
	void DecAlign()				{ if (align > 0.0f) align -= 0.1; }

protected:
	Vector3		PositionCorrect(Vector3 arg);

	void		Pop();

	void		JustBoidThings();
	void		MakeBoids();

	float		cohesion;
	float		align;
	float		seperation;

	Vector3		ComputeAlign(Boid* b);
	Vector3		ComputeCohes(Boid* b);
	Vector3		ComputeSeper(Boid* b);
	Vector3		ComputeWalls(Boid* b);

	GameEntity* BuildRobotEntity();
	GameEntity* BuildCubeEntity(float size);
	GameEntity* BuildSphereEntity(float radius);
	GameEntity* BuildQuadPlaneEntity(Vector3 dims, Quaternion ang, Vector3 pos, CollisionPlane* plane);
	GameEntity* BuildQuadAABBEntity(Vector3 dims, Quaternion ang, Vector3 pos);
	GameEntity* BuildInvisibleWall(Vector3 dims, Quaternion ang, Vector3 pos, CollisionPlane* plane);

	Mesh* cube;
	Mesh* quad;
	Mesh* sphere;

	vector<Boid*> flock;
};

