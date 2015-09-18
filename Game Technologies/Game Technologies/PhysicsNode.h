#pragma once

#include "../../nclgl/Quaternion.h"
#include "../../nclgl/Vector3.h"
#include "../../nclgl/Matrix3.h"
#include "../../nclgl/SceneNode.h"
#include "../../nclgl/Collision.h"
#include "../../nclgl/Plane.h"

#define GRAVITY 9.81f
#define DAMP_FAC 0.995f
#define MIN_VEL 0.3f

class PhysicsNode	{
public:
	PhysicsNode(void);
	PhysicsNode(Quaternion orientation, Vector3 position);
	~PhysicsNode(void);

	void				SetPosition(Vector3 arg)				{ m_position = arg; }
	Vector3				GetPosition()							{ return m_position; }
	
	void				SetLinearVelocity(Vector3 arg)			{ m_linearVelocity = arg; }
	Vector3				GetLinearVelocity()						{ return m_linearVelocity; }
	
	void				SetInverseMass(float arg)				{ m_invMass = arg; }
	float				GetInverseMass()						{ return m_invMass; }

	Quaternion			GetOrientation()						{ return m_orientation; }
	
	void				SetAngularVelocity(Vector3 arg)			{ m_angularVelocity = arg; }
	Vector3				GetAngularVelocity()					{ return m_angularVelocity; }

	void				ApplyForce(Vector3 arg, Vector3 arg_pos){ m_force = arg; m_forcePos = arg_pos; }
	Vector3				GetForce()								{ return m_force; }

	void				ApplyTorque(Vector3 arg)				{ m_torque =  arg; }
	Vector3				GetTorque()								{ return m_torque; }

	void				SetMass(float arg)						{ m_invMass = arg; }
	
	void				RestOff()								{ rest = FALSE; }
	void				SetAnchored(bool arg);

	Matrix3				GetInverseInertia()						{ return m_invInertia; }
	
	Matrix4				BuildTransform();

	void				BuildInertiaMatrixCuboid(Vector3 dim);
	void				BuildInertiaMatrixSphere(float radius);
	void				BuildInertiaMatrixStatic();

	virtual void		Update(float msec);

	void				Stop();

	bool				StopMotion();

	void				SetTarget(SceneNode *s)					{ target = s; }

	void				SetCollisionVolume(CollisionVolume* vol){ this->vol = vol; }
	CollisionVolume*	GetCollisionVolume() const				{ return vol; }

	void				SetBB(Vector3 dims)						{ boundingBox = dims; }
	Vector3				GetBB()									{ return boundingBox; }

	bool				AtRest() const							{ return rest; }
	bool				Anchored() const						{ return anchored; }

	void				SetGravity(bool arg)					{ gravity = arg; }

	void				SetSize(float scale);
	void				IncSize(float scale);
	void				Kill()									{ terminate = true; }
	void				Explode()								{ explode = true; }

	bool				IsDead()								{ return terminate; }
	bool				WillPop()								{ return explode; }

	void				IgnoreGravity()							{ ignoreGrav = TRUE; }
	bool				IgnoresGrav()							{ return ignoreGrav; }
	float				GetSize();
protected:
	/*<------------LINEAR------------->*/
	Vector3				m_position;
	Vector3				m_linearVelocity;
	Vector3				m_force;
	float				m_invMass;

	/*<------------ANGULAR------------>*/
	Quaternion			m_orientation;
	Vector3				m_angularVelocity;
	Vector3				m_forcePos;
	Vector3				m_torque;
	Matrix3				m_invInertia;

	/*<---------Spatial Things-------->*/
	bool				anchored;
	bool				rest;
	bool				gravity;
	bool				terminate;
	bool				explode;
	bool				ignoreGrav;

	CollisionVolume*	vol;
	Vector3				boundingBox;

	SceneNode*			target;  
};