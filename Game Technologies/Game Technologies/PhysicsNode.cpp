#include "PhysicsNode.h"

PhysicsNode::PhysicsNode(void) {
	target		= NULL;
	anchored	= FALSE;
	rest		= FALSE;
	gravity		= TRUE;
	terminate	= FALSE;
	explode		= FALSE;
	ignoreGrav	= FALSE;
}

PhysicsNode::PhysicsNode(Quaternion orientation, Vector3 position) {
	m_orientation	= orientation;
	m_position		= position;
	terminate		= FALSE;
	explode			= FALSE;
}

PhysicsNode::~PhysicsNode(void)	{

}

void PhysicsNode::BuildInertiaMatrixCuboid(Vector3 dim) {
	float fl = 1.0f / 12.0f;

	float x2 = dim.x * dim.x;
	float y2 = dim.y * dim.y;
	float z2 = dim.z * dim.z;

	float Ixx = fl * 1/m_invMass * (y2 + x2);
	float Iyy = fl * 1/m_invMass * (z2 + x2);
	float Izz = fl * 1/m_invMass * (y2 + z2);

	m_invInertia = Matrix3::InertiaMatrix(Vector3(Ixx, Iyy, Izz));
}

void PhysicsNode::BuildInertiaMatrixSphere(float radius) {
	float I = (2 * 1/m_invMass * (radius * radius)) * 0.2;

	m_invInertia = Matrix3::InertiaMatrix(Vector3(I, I, I));
}

void PhysicsNode::BuildInertiaMatrixStatic() {
	m_invInertia.ToZero();
}

//You will perform your per-object physics integration, here!
//I've added in a bit that will set the transform of the
//graphical representation of this object, too.
void	PhysicsNode::Update(float msec) {
	float sec = msec * 0.001f;

	bool stopped;

	//Resolve all forces acting on the object,
	if (!anchored && !rest) {
		//Calc a caused by that force - a = F/m.
		Vector3 accel = m_force * m_invMass;
		//Integrate a/t to get v.
		if (!ignoreGrav)
			m_linearVelocity = (m_linearVelocity + accel * sec) * DAMP_FAC;

		stopped = StopMotion();

		if (gravity && !ignoreGrav)
			if (!stopped)
				m_linearVelocity.y -= GRAVITY;

		//Integrate v/t to get pos.
		m_position = m_position + m_linearVelocity * sec;

		//Calculate torque acting on object.
		m_torque = Vector3::Cross(m_forcePos, m_force);

		//Multiply inverse inertia matrix by torque to get ang accel.
		Vector3 angAccel = m_invInertia * m_torque;

		//Calc angular velocity and orientation from angular accel.
		if (!ignoreGrav)
			m_angularVelocity = (m_angularVelocity + angAccel * sec) * DAMP_FAC;

		m_orientation = m_orientation + m_orientation * (m_angularVelocity * sec/2);
		m_orientation.Normalise();
	}
	else {
		m_linearVelocity = Vector3();
		m_angularVelocity = Vector3();
	}

	if(target) {
		Matrix4 prev = target->GetTransform();
		target->SetTransform(BuildTransform());
		if (!anchored) {
			if (prev == target->GetTransform() && m_position.y < 50.0f) {
				rest = true;
				target->ToBlack();
			}
			else {
				target->ToColour();
			}
		}
	}

	m_force = Vector3(0.0, 0.0, 0.0);
}

Matrix4		PhysicsNode::BuildTransform() {
	Matrix4 m = m_orientation.ToMatrix();

	m.SetPositionVector(m_position);

	return m;
}

void PhysicsNode::Stop() {
	SetAnchored(true);
}

bool PhysicsNode::StopMotion() {
	if (abs(m_linearVelocity.x) < MIN_VEL)
		m_linearVelocity.x = 0;

	if (abs(m_linearVelocity.y) < MIN_VEL)
		m_linearVelocity.y = 0;

	if (abs(m_linearVelocity.z) < MIN_VEL)
		m_linearVelocity.z = 0;

	if (abs(m_angularVelocity.x) < MIN_VEL)
		m_angularVelocity.x = 0;

	if (abs(m_angularVelocity.y) < MIN_VEL)
		m_angularVelocity.y = 0;

	if (abs(m_angularVelocity.z) < MIN_VEL)
		m_angularVelocity.z = 0;

	if (m_angularVelocity == Vector3() && m_linearVelocity == Vector3())
		if (m_position.y < 50.0f)
			return true;

	return false;
}

void PhysicsNode::SetAnchored(bool arg) {
	anchored = arg;
	if (arg)
		rest = arg;
}

void PhysicsNode::SetSize(float scale) {
	target->SetBoundingRadius(scale);
	BuildInertiaMatrixSphere(scale);
	boundingBox = Vector3(scale) * 3;
	vol = new CollisionWell(scale);
}

float PhysicsNode::GetSize() {
	return target->GetBoundingRadius();
}

void PhysicsNode::IncSize(float scale) {
	target->SetModelScale(target->GetModelScale() * scale);
	target->SetBoundingRadius(target->GetBoundingRadius() * scale);
	BuildInertiaMatrixSphere(target->GetBoundingRadius() * scale);
	boundingBox = boundingBox * scale;
	CollisionWell* well = (CollisionWell*)vol;
	well->SetRadius(well->GetRadius() * scale);
	well->IncAbsorbs();
	vol = well;
}