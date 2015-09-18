#include "PhysicsSystem.h"

PhysicsSystem* PhysicsSystem::instance = 0;

PhysicsSystem::PhysicsSystem(void)	{
	collisionCount = 0;
	gravity = true;
}

PhysicsSystem::~PhysicsSystem(void)	{

}

void PhysicsSystem::Update(float msec) {	
	muLan.lock();
	BroadPhaseCollisions();
	NarrowPhaseCollisions();
	
	//BruteForceCollisions();

	for(vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i) {
		//Checks to see if the current element is a well and is supposed to explode.
		if ((*i)->GetCollisionVolume()->GetType() == COLLISION_WELL) {
			CollisionWell* well = (CollisionWell*)(*i)->GetCollisionVolume();
			if (well->GetBumps() > 4) {
				(*i)->Explode();
			}
		}
		(*i)->SetGravity(gravity);
		(*i)->Update(msec);
	}

	//This removes any nodes by setting the "kill" boolean to true, which is picked up in the MyGame update method.
	if (cullNodes.size() > 0)
		for (vector<PhysicsNode*>::iterator i = cullNodes.begin(); i != cullNodes.end(); ++i) {
			(*i)->Kill();
			RemoveNode(*i);
		}

	muLan.unlock();
}
/*Uses "World Space Partitioning" to reduce the number of collisions to be tested in the narrowphase section.
Divides the world into a 21(x) by 11(y) by 21(z) 3d grid, and places a pointer to any entity whose bounding box
occupies a grid section. One entity can occupy many sections and many entities can occupy many sections.
*/
void PhysicsSystem::BroadPhaseCollisions() {
	//int hx = rx & 15;
	for (int x = 0; x < 21 * 11 * 21; ++x)
		sp[x].clear();
	
	Vector3 pos, bb;

	for (vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i) {
		if ((*i)->GetCollisionVolume()->GetType() != COLLISION_PLANE && !(*i)->IgnoresGrav()) {
			Vector3 pos	= (*i)->GetPosition();
			Vector3 bb	= (*i)->GetBB();
			//Make the bounding boxes bigger to increase accuracy.
			bb = bb * 2;
			//Get the vectors for two adjacent corners, adjusted into postive only values.
			Vector3 low		= Vector3(pos.x - bb.x + 2000, pos.y - bb.y, pos.z - bb.z + 2000); 
			Vector3 high	= Vector3(pos.x + bb.x + 2000, pos.y + bb.y, pos.z + bb.z + 2000);

			//Checks to see if entities are likely to collide with the ground/walls, and add them to their own vectors.
			if ((low.y < 100.0f && (*i)->GetCollisionVolume()->GetType() == COLLISION_SPHERE) && (*i)->AtRest() == false)
				collidesWithGround.push_back(*(i));

			if ((low.x < 100.0f || high.x > (X_MAX - 100.0f) || low.z < 100.0f || high.z > (Z_MAX - 100.0f) || high.y > (Y_MAX - 100.0f))
				&& (*i)->GetCollisionVolume()->GetType() == COLLISION_SPHERE)
				collidesWithWall.push_back(*(i));

			//Divide by 200 so the x, y, and z values can be used as the array index value.
			low		= low * 0.005f;
			high	= high * 0.005f;

			//In here the x, y, and z values are used to place pointers to the PhysicsNode object into the vectors in the array.
			for (int x = (int)low.x; x <= (int)high.x; ++x) 
			{
				for (int y = (int)low.y; y <= (int)high.y; ++y) 
				{
					for (int z = (int)low.z; z <= (int)high.z; ++z) 
					{
						sp[x + 21*y + 21*11*z].push_back(*(i));
					}
				}
			}
		}
	}
}

void PhysicsSystem::NarrowPhaseCollisions() {
	CollisionData* cData;
	//The world space array is iterated through to check for probable collision pairs.
	for (int x = 0; x < 21 * 11 * 21; ++x) {
		if (sp[x].size() > 1) 
		{
			for (vector<PhysicsNode*>::iterator i = sp[x].begin(); i != sp[x].end(); ++i) 
			{
				for (vector<PhysicsNode*>::iterator o = sp[x].begin() + 1; o != sp[x].end(); ++o) 
				{
					//Standard Sphere-Sphere collision dection + resolution.
					cData = new CollisionData();
					if ((*i)->GetCollisionVolume()->GetType() == COLLISION_SPHERE && (*o)->GetCollisionVolume()->GetType() == COLLISION_SPHERE) 
					{
						if (CollisionHelper::SphereSphereCollision(*(*i), *(*o), cData)) 
						{
							if ((*i)->Anchored() == false && (*i)->AtRest() == true)
								(*i)->RestOff();

							if ((*o)->Anchored() == false && (*o)->AtRest() == true)
								(*o)->RestOff();

							++collisionCount;
							CollisionHelper::AddCollisionImpulse(*(*i), *(*o), *cData);
						}
					}
					else if ((*i)->GetCollisionVolume()->GetType() == COLLISION_WELL && (*o)->GetCollisionVolume()->GetType() == COLLISION_SPHERE)
					{
						if (gravity) {
							cData = new CollisionData();
							if (CollisionHelper::SphereInactiveWellCollision(*(*i), *(*o), cData)) {
								if ((*i)->Anchored() == false && (*i)->AtRest() == true)
									(*i)->RestOff();

								if ((*o)->Anchored() == false && (*o)->AtRest() == true)
									(*o)->RestOff();

								++collisionCount;
								CollisionHelper::AddCollisionImpulse(*(*i), *(*o), *cData);
							}

						}
						else {
							//THATS NO MOON, ITS A SPACE STATION. Well its actually a well, of gravity (sort of).
							cData = new CollisionData();
							int colType = CollisionHelper::SphereWellCollision(*(*i), *(*o), cData);
							if (colType == 1) 
							{
								if ((*i)->Anchored() == false && (*i)->AtRest() == true)
									(*i)->RestOff();

								if ((*o)->Anchored() == false && (*o)->AtRest() == true)
									(*o)->RestOff();
								++collisionCount;
								//Checks to see if the colliding sphere has entered the attractor.
								if (CollisionHelper::AbsorbCollidingSphere(*(*i), *(*o), *cData)) {
									//Increase radius, increment absorbs, delete absorbed sphere.
									(*i)->IncSize(1.1f);
									cullNodes.push_back(*o);
									CollisionWell* well = (CollisionWell*)(*i)->GetCollisionVolume();
									cout << well->GetBumps() << endl;
								}
							}
 							else if (colType == 2)
							{
								//NONE SHALL ESCAPE THE PULL OF GRAVITY, unless of course, the velocity is high enough.
								CollisionHelper::AddGravitationalImpulse(*(*i), *(*o), *cData);
							}
						}
					}
				}
			}
		}
	}

	//Sphere + ground collisions are checked + resolved.
	for (vector<PhysicsNode*>::iterator i = collidesWithGround.begin(); i != collidesWithGround.end(); ++i) {
		cData = new CollisionData();
		if (CollisionHelper::SpherePlaneCollision(*(*i), *allNodes.at(0), cData)) {
			++collisionCount;
			CollisionHelper::AddCollisionImpulse(*(*i), *allNodes.at(0), *cData);
		}
	}
	//Next the bulk of collisions are checked.
	
	//Lastly the walls are tested for collsions.
	for (vector<PhysicsNode*>::iterator i = collidesWithWall.begin(); i != collidesWithWall.end(); ++i) {
		for (vector<PhysicsNode*>::iterator o = allNodes.begin() + 1; o != allNodes.begin() + 6; ++o) {
			cData = new CollisionData();
			if (CollisionHelper::SpherePlaneCollision(*(*i), *(*o), cData)) {
				++collisionCount;
				CollisionHelper::AddCollisionImpulse(*(*i), *(*o), *cData);
			}
		}
	}

	collidesWithGround.clear();
	collidesWithWall.clear();
}


void PhysicsSystem::BruteForceCollisions() {
	for(vector<PhysicsNode*>::iterator o = allNodes.begin(); o != allNodes.end(); ++o) 
	{
		for(vector<PhysicsNode*>::iterator i = o + 1; i != allNodes.end(); ++i) 
		{
			if (i != o)
			{
				CollisionVolumeType iType = (*i)->GetCollisionVolume()->GetType();
				CollisionVolumeType oType = (*o)->GetCollisionVolume()->GetType();

				if (iType == COLLISION_SPHERE && oType == COLLISION_SPHERE) 
				{
					CollisionData* cData = new CollisionData();
					if (CollisionHelper::SphereSphereCollision(*(*i), *(*o), cData)) 
					{
						++collisionCount;
						if ((*i)->Anchored() == false && (*i)->AtRest() == true)
							(*i)->RestOff();

						if ((*o)->Anchored() == false && (*o)->AtRest() == true)
							(*o)->RestOff();

						CollisionHelper::AddCollisionImpulse(*(*i), *(*o), *cData);
					}
				}
				if ((iType == COLLISION_SPHERE && !(*i)->AtRest()) && oType == COLLISION_PLANE) 
				{
					CollisionData* cData = new CollisionData();
					if (CollisionHelper::SpherePlaneCollision(*(*i), *(*o), cData))
					{
						++collisionCount;
						CollisionHelper::AddCollisionImpulse(*(*i), *(*o), *cData);
					}
				}
			}
		}
	}
	cout << "CC: " << collisionCount << endl;
}

int PhysicsSystem::GetNodeNum() {
	return allNodes.size();
}

void PhysicsSystem::AddNode(PhysicsNode* n) {
	muLan.lock();
	allNodes.push_back(n);
	muLan.unlock();
}

void PhysicsSystem::RemoveNode(PhysicsNode* n) {
	for(vector<PhysicsNode*>::iterator i = allNodes.begin(); i != allNodes.end(); ++i) {
		if((*i) == n) {
			allNodes.erase(i);
			return;
		}
	}
}

void PhysicsSystem::ToggleGravity() {
	gravity = !gravity;
}
