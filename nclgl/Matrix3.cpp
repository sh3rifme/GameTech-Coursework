#include "Matrix3.h"


Matrix3::Matrix3(void)
{
}


Matrix3::~Matrix3(void)
{
}

void Matrix3::ToZero() {
	for (unsigned int i = 0; i < 9; ++i) {
		values[i] = 0.0f;
	}
}

Matrix3 Matrix3::InertiaMatrix(Vector3 &arg) {
	Matrix3 out;
	
	out.ToZero();

	out.values[0] = arg.x;
	out.values[4] = arg.y;
	out.values[8] = arg.z;

	return out;
}
