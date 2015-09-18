#include "GameClass.h"

GameClass* GameClass::instance = NULL;

GameClass::GameClass()	{
	renderCounter	= 0.0f;
	physicsCounter	= 0.0f;
	frameCount		= 0;
	physFrameCount	= 0;
	deltaTime		= 0.0f;
	slowMo			= false;
	instance		= this;

	for each (int var in vars) {
		var = 0;
	}
}

GameClass::~GameClass(void)	{
	for(vector<GameEntity*>::iterator i = allEntities.begin(); i != allEntities.end(); ++i) {
		delete (*i);
	}
	delete gameCamera;
}

void GameClass::UpdateCore(float msec) {
	renderCounter	-= msec;
	deltaTime += msec;
	
	Renderer::GetRenderer().UpdateDisplayValues(vars);

	if(renderCounter <= 0.0f) {	//Update our rendering logic
		frameCount++;
		Renderer::GetRenderer().UpdateScene(1000.0f / (float)RENDER_HZ);
		Renderer::GetRenderer().RenderScene();
		renderCounter += (1000.0f / (float)RENDER_HZ);
	}

	if (deltaTime >= 1000.0f) {
		if (vars[0] > 0 && vars[1] > 0) {
			vars[0]		= (frameCount + vars[0]) / 2;
			vars[1] = (physFrameCount + vars[1]) / 2;
		}
		else {
			vars[0]		= frameCount;
			vars[1] = physFrameCount;
		}
		frameCount = 0;
		physFrameCount = 0;
		deltaTime = 0.0f;
	}
	
	vars[2] = PhysicsSystem::GetPhysicsSystem().GetNodeNum();
	vars[3] = PhysicsSystem::GetPhysicsSystem().GetCollisionCount();
}

void GameClass::UpdatePhysics(volatile bool* cont) {
	GameTimer msec;
	bool a = true;
	while (*cont) {
		float mil = msec.GetTimedMS();
		if (slowMo)
			physicsCounter += mil * 0.2;
		else
			physicsCounter += mil;

		while(physicsCounter >= 0.0f) {
			physFrameCount++;
			physicsCounter -= PHYSICS_TIMESTEP;
			PhysicsSystem::GetPhysicsSystem().Update(PHYSICS_TIMESTEP);
		}
	}
}
