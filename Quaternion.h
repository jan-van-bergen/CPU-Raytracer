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
		left.x * right.w + left.w * right.x + left.y * q.z - left.z * right.y,
		left.y * right.w + left.w * right.y + left.z * q.x - left.x * right.z,
		left.z * right.w + left.w * right.z + left.x * q.y - left.y * right.x,
		left.w * right.w - left.x * right.x - left.y * q.y - left.z * right.z
	);
}

inline Quaternion operator*(const Quaternion & quaternion, const Vector3 & vector) {
	return Quaternion(
		 quaternion.w * vector.x + quaternion.y * vector.z - quaternion.z * vector.y,
		 quaternion.w * vector.y + quaternion.z * vector.x - quaternion.x * vector.z,
		 quaternion.w * vector.z + quaternion.x * vector.y - quaternion.y * vector.x,
		-quaternion.x * vector.x - quaternion.y * vector.y - quaternion.z * vector.z
	);
}
