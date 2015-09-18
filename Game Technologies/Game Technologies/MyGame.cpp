#include "MyGame.h"

MyGame::MyGame() {
	gameCamera = new Camera(0.0f,0.0f,Vector3(-1000,1000,0));

	Renderer::GetRenderer().SetCamera(gameCamera);

	cohesion = align = seperation = 1.0f;

	CubeRobot::CreateCube();

	cube	= new OBJMesh(MESHDIR"cube.obj");
	quad	= Mesh::GenerateQuad();

	allEntities.push_back(BuildQuadPlaneEntity(Vector3(2000.0f, 2000.0f, 9.0f), Quaternion::AxisAngleToQuaterion(Vector3(1,0,0), 90.0f), Vector3(), new CollisionPlane(Vector3(0,1,0), 1.0f)));
	allEntities.push_back(BuildInvisibleWall(Vector3(2000.0f, 5.0f, 1.0f), Quaternion::AxisAngleToQuaterion(Vector3(0,1,0), 90.0f), Vector3(2000, 0, 0), new CollisionPlane(Vector3(-1,0,0), -2000.0f)));
	allEntities.push_back(BuildInvisibleWall(Vector3(2000.0f, 5.0f, 1.0f), Quaternion::AxisAngleToQuaterion(Vector3(0,1,0), 180.0f), Vector3(0,0,2000), new CollisionPlane(Vector3(0,0,-1), -2000.0f)));
	allEntities.push_back(BuildInvisibleWall(Vector3(2000.0f, 5.0f, 1.0f), Quaternion::AxisAngleToQuaterion(Vector3(0,1,0), 90.0f), Vector3(-2000, 0, 0), new CollisionPlane(Vector3(1, 0, 0), -2000.0f)));
	allEntities.push_back(BuildInvisibleWall(Vector3(2000.0f, 5.0f, 1.0f), Quaternion::AxisAngleToQuaterion(Vector3(0,1,0), 180.0f), Vector3(0,0,-2000), new CollisionPlane(Vector3(0,0,1), -2000.0f)));
	allEntities.push_back(BuildInvisibleWall(Vector3(10.0f, 10.0f, 1.0f), Quaternion::AxisAngleToQuaterion(Vector3(1,0,0), 90.0f), Vector3(0,2000,0), new CollisionPlane(Vector3(0,-1,0), -2000.0f)));

	float size = 100.0f;
	Vector3 pos = Vector3(0, 1000, 0);

	GameEntity* g = BuildSphereEntity(size);

	g->GetPhysicsNode().SetPosition(pos);
	g->GetPhysicsNode().SetInverseMass(0);
	g->GetPhysicsNode().SetAnchored(true);
	g->GetRenderNode().SetTransform(Matrix4::Translation(g->GetPhysicsNode().GetPosition()));

	allEntities.push_back(g);

	

	//allEntities.push_back(BuildRobotEntity());
	//allEntities.push_back(BuildQuadAABBEntity(Vector3(1000.0f, 100.0f, 9.0f), Quaternion::AxisAngleToQuaterion(Vector3(0,1,0), 90.0f), Vector3(1000,100,0)));
	//allEntities.push_back(BuildQuadAABBEntity(Vector3(1000.0f, 100.0f, 9.0f), Quaternion::AxisAngleToQuaterion(Vector3(0,1,0), 270.0f), Vector3(-1000, 100, 0)));
	//allEntities.push_back(BuildQuadAABBEntity(Vector3(1000.0f, 100.0f, 9.0f), Quaternion::AxisAngleToQuaterion(Vector3(0,1,0), 180.0f), Vector3(0,100,-1000)));
}

MyGame::~MyGame(void)	{
	delete cube;
	delete quad;
	delete sphere;

	CubeRobot::DeleteCube();
}

void MyGame::UpdateGame(float msec) {
	if(gameCamera) {
		gameCamera->UpdateCamera(msec);
	}
	bool pop = false;
	for(vector<GameEntity*>::iterator i = allEntities.begin(); i != allEntities.end(); ++i) {
		if ((*i)){
			//If the entity is set to be removed by the physics engine, it is removed.
			if ((*i)->GetPhysicsNode().IsDead()) {
				(*i)->DisconnectFromSystems();
			}
			else if ((*i)->GetPhysicsNode().WillPop()) {
				(*i)->DisconnectFromSystems();
				float size = 100.0f;
				Vector3 pos = Vector3(0, 1000, 0);
				GameEntity* g = BuildSphereEntity(size);
				g->GetPhysicsNode().SetPosition(pos);
				g->GetPhysicsNode().SetInverseMass(0);
				g->GetPhysicsNode().SetAnchored(true);
				g->GetRenderNode().SetTransform(Matrix4::Translation(g->GetPhysicsNode().GetPosition()));
				(*i) = g;
				pop = true;
				(*i)->Update(msec);
				ToggleGravity();
				MakeBoids();
			}
			else
				(*i)->Update(msec);
		}
	}
	
	if (pop)
		Pop();
	float vars [3] = {cohesion, align, seperation};

	Renderer::GetRenderer().UpdateFlockingValues(vars);

	JustBoidThings();

	//Renderer::GetRenderer().DrawDebugBox(DEBUGDRAW_PERSPECTIVE, Vector3(0,51,0), Vector3(100,100,100), Vector3(1,0,0));
	//Renderer::GetRenderer().DrawDebugLine(DEBUGDRAW_PERSPECTIVE, Vector3(0,1,0),Vector3(200,1,200), Vector3(0,0,1), Vector3(1,0,0));
	//Renderer::GetRenderer().DrawDebugCross(DEBUGDRAW_PERSPECTIVE, Vector3(200,1,200),Vector3(50,50,50), Vector3(0,0,0));
	//Renderer::GetRenderer().DrawDebugCircle(DEBUGDRAW_PERSPECTIVE, Vector3(-200,1,-200),50.0f, Vector3(0,1,0));
}

//Fires a sphere in the direction the camera is facing.
void MyGame::ShootSphere(float size) {
	PhysicsNode* p = new PhysicsNode();
	Vector3 dir;

	sphere = Mesh::GenerateSphere(size);

	dir = gameCamera->BuildViewMatrix().GetProjectionVector();
	dir.Normalise();

	Vector3 spawnAt = Matrix4::Translation(dir * 100) * gameCamera->GetPosition();

	p->SetPosition(PositionCorrect(spawnAt));

	p->SetInverseMass(1/(size*0.005));
	p->BuildInertiaMatrixSphere(size);
	p->ApplyForce(dir * 2000.0f * (size/2), Vector3(((rand() % 4) - 2) * 0.00001f,((rand() % 4) - 2) * 0.00001f,((rand() % 4) - 2) * 0.00001f));
	p->SetCollisionVolume(new CollisionSphere(size));

	GameEntity*g = new GameEntity(new SceneNode(sphere), p);
	g->ConnectToSystems();

	SceneNode &test = g->GetRenderNode();

	Vector4 colour = Vector4((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, 1.0f);
	
	test.SetBoundingRadius(size);
	test.SetTransform(Matrix4::Translation(p->GetPosition()));
	test.SetColour(colour);
	allEntities.push_back(g);
}

/*
Makes an entity that looks like a CubeRobot!
*/
GameEntity* MyGame::BuildRobotEntity() {
	GameEntity*g = new GameEntity(new CubeRobot(), new PhysicsNode());

	g->GetPhysicsNode().SetAnchored(true);
	g->GetPhysicsNode().SetCollisionVolume(new CollisionSphere(20.0f));
	g->ConnectToSystems();
	return g;
}

/*
Makes a cube.
*/
GameEntity* MyGame::BuildCubeEntity(float size) {
	GameEntity*g = new GameEntity(new SceneNode(cube), new PhysicsNode());
	g->ConnectToSystems();

	SceneNode &test = g->GetRenderNode();

	test.SetModelScale(Vector3(size,size,size));
	test.SetBoundingRadius(size);

	return g;
}

/*
Makes a sphere, the hard way, because science.
*/
GameEntity* MyGame::BuildSphereEntity(float radius) {
	PhysicsNode* p = new PhysicsNode();
	sphere = Mesh::GenerateSphere(radius);
	p->SetInverseMass(1/(radius*0.005));
	p->SetLinearVelocity(Vector3(0,0,0));
	GameEntity*g = new GameEntity(new SceneNode(sphere), p);
	g->ConnectToSystems();
	g->GetPhysicsNode().SetSize(radius);

	Vector4 colour = Vector4((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, 1.0f);
	
	g->GetRenderNode().SetColour(colour);
	return g;
}

/*
Makes a flat quad.
*/
GameEntity* MyGame::BuildQuadPlaneEntity(Vector3 dims, Quaternion ang, Vector3 pos, CollisionPlane* plane) {
	SceneNode* s = new SceneNode(quad);

	s->SetModelScale(dims);
	s->SetColour(Vector4(0.0f, 1.0f, 1.0f, 1.0f));
	s->SetBoundingRadius(dims.x);

	PhysicsNode*p = new PhysicsNode(ang, pos);
	p->SetAnchored(true);
	p->SetCollisionVolume(plane);
	p->BuildInertiaMatrixStatic();
	p->SetInverseMass(0);
	Vector3 bb = ang.ToMatrix() * bb;
	p->SetBB(bb);
	GameEntity*g = new GameEntity(s, p);
	g->ConnectToSystems();
	return g;
}

//Makes a quad with an AABB that can be used to simulate non-infite walls etc. Collisions not being tested/checked/responded to.
//More of a hopeful sidenote than anything else.
GameEntity* MyGame::BuildQuadAABBEntity(Vector3 dims, Quaternion ang, Vector3 pos) {
	SceneNode* s = new SceneNode(quad);

	s->SetModelScale(dims);
	s->SetColour(Vector4(0.0f, 1.0f, 1.0f, 1.0f));
	s->SetBoundingRadius(dims.x);

	PhysicsNode*p = new PhysicsNode(ang, pos);
	p->SetAnchored(true);
	p->SetCollisionVolume(new CollisionBB(dims));
	p->BuildInertiaMatrixStatic();
	p->SetInverseMass(0);
	Vector3 bb = ang.ToMatrix() * bb;
	p->SetBB(dims);

	GameEntity*g = new GameEntity(s, p);
	g->ConnectToSystems();
	return g;
}

//I've heard invisble walls are the game designers best friend and immersion's invisible arch nemesis.
GameEntity* MyGame::BuildInvisibleWall(Vector3 dims, Quaternion ang, Vector3 pos, CollisionPlane* plane) {
	SceneNode* s = new SceneNode(quad);

	s->SetModelScale(dims);
	s->SetColour(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	s->SetBoundingRadius(dims.x);

	PhysicsNode*p = new PhysicsNode(ang, pos);
	p->SetAnchored(true);
	p->SetCollisionVolume(plane);
	p->BuildInertiaMatrixStatic();
	p->SetInverseMass(0);
	Vector3 bb = ang.ToMatrix() * bb;
	p->SetBB(bb);
	GameEntity*g = new GameEntity(s, p);
	g->ConnectToSystems();
	return g;
}

void MyGame::ToggleGravity() {
	Renderer::GetRenderer().ToggleEffect(2);
	PhysicsSystem::GetPhysicsSystem().ToggleGravity();
}

//TODO: Stop Spheres being made outside the box
Vector3 MyGame::PositionCorrect(Vector3 arg) {
	Vector3 max = Vector3(2000, 2000, 2000);
	Vector3 min = Vector3(-2000, 0, -2000);

	if (arg.x < min.x)
		arg.x = min.x;
	else if (arg.x > max.x)
		arg.x = max.x;

	if (arg.y < min.y)
		arg.y = min.y;
	else if (arg.y > max.y)
		arg.y = max.y;

	if (arg.z < min.z)
		arg.z = min.z;
	else if (arg.z > max.z)
		arg.z = max.z;

	return arg;
}

void MyGame::Pop() {
	float size;
	for (int i = 0; i < (rand() % 10) + 1; ++i) {
		size = ((rand() % 4) + 1) * 20.0f;
		PhysicsNode* p = new PhysicsNode();
		sphere = Mesh::GenerateSphere(size);

		Vector3 spawnAt(0, 1000, 0);
		Vector3 dir((rand() % 100) - 50, (rand() % 100) - 50, (rand() % 100) - 50);
		dir.Normalise();

		p->SetPosition(spawnAt + (dir * 100.0f));
		p->SetInverseMass(1/(size*0.005));
		p->BuildInertiaMatrixSphere(size);
		p->ApplyForce(dir * 6000.0f * (size/2), Vector3(((rand() % 4) - 2) * 0.00001f,((rand() % 4) - 2) * 0.00001f,((rand() % 4) - 2) * 0.00001f));
		p->SetCollisionVolume(new CollisionSphere(size));

		GameEntity*g = new GameEntity(new SceneNode(sphere), p);
		g->ConnectToSystems();

		SceneNode &sn = g->GetRenderNode();

		Vector4 colour = Vector4((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f, 1.0f);
		
		sn.SetBoundingRadius(size);
		sn.SetTransform(Matrix4::Translation(p->GetPosition()));
		sn.SetColour(colour);
		allEntities.push_back(g);
	}
}

void MyGame::MakeBoids() {
	float size = 10.0f;
	for (int i = 0; i < 40; ++i) {
		Boid* b = new Boid(1.0, 1.0, 1.0);
		//Set textures and positions for each boid randomly, perhaps shit them out from the exploding sphere.
		SceneNode* sn = new SceneNode(Mesh::GenerateSphere(size));
		sn->SetColour(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		sn->SetBoundingRadius(size);
		//sn->SetModelScale(size);

		PhysicsNode* pn = new PhysicsNode();
		pn->IgnoreGravity();
		pn->SetInverseMass(1);
		pn->SetPosition(Vector3(0, 1000, 0));
		Vector3 vel((rand() % 100) -50,(rand() % 100) -50,(rand() % 100) -50);
		vel.Normalise();
		pn->SetLinearVelocity(vel * 100);
		pn->SetCollisionVolume(new NoCollide());

		b->ConnectToSystems(sn, pn);

		flock.push_back(b);
	}
}

void MyGame::JustBoidThings() {
	for (vector<Boid*>::iterator i = flock.begin(); i != flock.end(); ++i) {
		(*i)->SetAlign(align);
		(*i)->SetCohes(cohesion);
		(*i)->SetSep(seperation);
		Vector3 a = ComputeAlign(*i);
		Vector3 c = ComputeCohes(*i);
		Vector3 s = ComputeSeper(*i);
		Vector3 w = ComputeWalls(*i);

		Vector3 sum = a * (*i)->GetAlignW() + c * (*i)->GetCohW() + s * (*i)->GetSepW();
		a += w;
		sum.Normalise();
		sum = sum * 300;

		(*i)->GetPhysicsNode().SetLinearVelocity(sum);
	}

}

Vector3 MyGame::ComputeAlign(Boid* b) {
	Vector3 a = Vector3(0.0f);
	int neighbors = 0;

	for (vector<Boid*>::iterator i = flock.begin(); i != flock.end(); ++i) {
		if ((*i) != b) {
			Vector3 sep = (*i)->GetSeparation(b);
			if (sep < SEARCH_RAD) {
				a += b->GetPhysicsNode().GetLinearVelocity();
				++neighbors;
			}
		}
	}
	if (neighbors == 0)
		return a;

	a = a / neighbors;
	a.Normalise();
	return a;
}

Vector3 MyGame::ComputeCohes(Boid* b) {
	Vector3 c = Vector3(0.0f);
	int neighbors = 0;

	for (vector<Boid*>::iterator i = flock.begin(); i != flock.end(); ++i) {
		if ((*i) != b) {
			Vector3 sep = (*i)->GetSeparation(b);
			if (sep < SEARCH_RAD) {
				c += b->GetPhysicsNode().GetLinearVelocity();
				++neighbors;
			}
		}
	}
	if (neighbors == 0)
		return c;
	c = c / neighbors;
	c = c - b->GetPhysicsNode().GetLinearVelocity();
	c.Normalise();
	return c;
}

Vector3 MyGame::ComputeSeper(Boid* b) {
	Vector3 s = Vector3(0.0f);
	int neighbors = 0;

	for (vector<Boid*>::iterator i = flock.begin(); i != flock.end(); ++i) {
		if ((*i) != b) {
			if ((*i)->GetSeparation(b) < SEARCH_RAD) {
				s += (*i)->GetPhysicsNode().GetLinearVelocity() - b->GetPhysicsNode().GetLinearVelocity();
				++neighbors;
			}
		}
	}
	if (neighbors == 0)
		return s;
	s = s / neighbors;
	s = s * -1;
	s.Normalise();
	return s;
}

Vector3 MyGame::ComputeWalls(Boid* b) {
	Vector3 w = Vector3(0.0f);
	Vector3 pos = b->GetPhysicsNode().GetPosition();
	float max = 1800.0f;

	if (pos.x > max)
		w.x = -10.0f;
	if (pos.y > max)
		w.y = -10.0f;
	if (pos.z > max)
		w.z = -10.0f;

	if (pos.x < -max)
		w.x = 10.0f;
	if (pos.y < 500)
		w.y = 10.0f;
	if (pos.z < -max)
		w.z = 10.0f;

	return w;
}
