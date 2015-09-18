#pragma once

#include "../../nclgl/SceneNode.h"
#include "PhysicsNode.h"

class GameEntity	{
public:
	friend class GameClass;

	GameEntity(void);
	GameEntity(SceneNode* s, PhysicsNode* p);
	virtual ~GameEntity(void);

	virtual void	Update(float msec);

	SceneNode&		GetRenderNode()		{ return *renderNode; }
	PhysicsNode&	GetPhysicsNode()	{ return *physicsNode; }

	void			ConnectToSystems(SceneNode* sn, PhysicsNode* pn);
	void			ConnectToSystems();
	void			DisconnectFromSystems();

protected:
	SceneNode*		renderNode;
	PhysicsNode*	physicsNode;
};