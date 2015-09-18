#pragma once
#include "Renderer.h"
#include "PhysicsSystem.h"
#include "GameEntity.h"

#include <vector>

#define RENDER_HZ	60
#define PHYSICS_HZ	120

#define PHYSICS_TIMESTEP (1000.0f / (float)PHYSICS_HZ)

class GameClass	{
public:
	GameClass();
	~GameClass(void);

	virtual void UpdateCore(float msec);
	void UpdatePhysics(volatile bool* cont);

	virtual void UpdateGame(float msec) = 0;

	static GameClass& GetGameClass() { return *instance;}

	void ToggleSlowMo() { slowMo = !slowMo; }

protected:
	bool					slowMo;

	float					renderCounter;
	float					physicsCounter;

	int						vars[4];
	int						frameCount;
	int						physFrameCount;
	float					deltaTime;

	vector<GameEntity*>		allEntities;

	mutex					muLan;

	Camera*					gameCamera;
	static GameClass*		instance;
};

