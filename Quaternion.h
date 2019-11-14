#pragma once
#include <math.h>

#include "Vector3.h"

struct Quaternion {
	float x, y, z, w;

	inline Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) { }
	inline Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }
	
	inline float length() const {
		return sqrtf(x*x + y*y + z*z + w*w);
	}

	inline static Quaternion conjugate(const Quaternion & quaternion) {
		return Quaternion(-quaternion.x, -quaternion.y, -quaternion.z, quaternion.w);
	}

	inline static Quaternion axis_angle(const Vector3 & axis, float angle) {
		float half_angle = 0.5f * angle;
		float sine = sinf(half_angle);

		return Quaternion(
			axis.x * sine,
			axis.y * sine,
			axis.z * sine,
			cosf(half_angle)
		);
	}
};

inline Quaternion operator*(const Quaternion & left, Quaternion & right) {
	return Quaternion(
		left.x * right.w + left.w * right.x + left.y * right.z - left.z * right.y,
		left.y * right.w + left.w * right.y + left.z * right.x - left.x * right.z,
		left.z * right.w + left.w * right.z + left.x * right.y - left.y * right.x,
		left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z
	);
}

inline Vector3 operator*(const Quaternion & quaternion, const Vector3 & vector) {
	Vector3 q(quaternion.x, quaternion.y, quaternion.z);

	return 2.0f * Vector3::dot(q, vector) * q +
		(quaternion.w * quaternion.w - Vector3::dot(q, q)) * vector +
		2.0f * quaternion.w * Vector3::cross(q, vector);
}
