#pragma once
#include "Vector3.h"

class Matrix3
{
public:
	Matrix3(void);
	~Matrix3(void);

	float values[9];

	void ToZero();

	static Matrix3 InertiaMatrix(Vector3 &vec);

	inline Vector3 operator*(const Vector3 &v) const {
		Vector3 vec;

		vec.x = v.x * values[0] + v.y * values[3] + v.z * values[6];
		vec.y = v.x * values[1] + v.y * values[4] + v.z * values[7];
		vec.z = v.x * values[2] + v.y * values[5] + v.z * values[8];

		return vec;
	};
};