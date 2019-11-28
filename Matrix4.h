#pragma once
#include <cassert>
#include <string.h>

#include "Vector3.h"

struct Matrix4 {
	float cells[16];

	inline Matrix4() {
		memset(cells, 0, sizeof(cells));
		cells[0]  = 1.0f;
		cells[5]  = 1.0f;
		cells[10] = 1.0f;
		cells[15] = 1.0f;
	}

	inline float & operator()(int i, int j) { 
		assert(i >= 0 && i < 4); 
		assert(j >= 0 && j < 4); 
		return cells[i + (j << 2)]; 
	}
	inline const float & operator()(int i, int j) const { 
		assert(i >= 0 && i < 4); 
		assert(j >= 0 && j < 4); 
		return cells[i + (j << 2)];
	}

	// Transforms a Vector3 as if the fourth coordinate is 1
	inline static Vector3 transform_position(const Matrix4 & matrix, const Vector3 & position) {
		return Vector3(
			matrix(0, 0) * position.x + matrix(1, 0) * position.y + matrix(2, 0) * position.z + matrix(3, 0),
			matrix(0, 1) * position.x + matrix(1, 1) * position.y + matrix(2, 1) * position.z + matrix(3, 1),
			matrix(0, 2) * position.x + matrix(1, 2) * position.y + matrix(2, 2) * position.z + matrix(3, 2)
		);
	}

	// Transforms a Vector3 as if the fourth coordinate is 0
	inline static Vector3 transform_direction(const Matrix4 & matrix, const Vector3 & direction) {
		return Vector3(
			matrix(0, 0) * direction.x + matrix(1, 0) * direction.y + matrix(2, 0) * direction.z,
			matrix(0, 1) * direction.x + matrix(1, 1) * direction.y + matrix(2, 1) * direction.z,
			matrix(0, 2) * direction.x + matrix(1, 2) * direction.y + matrix(2, 2) * direction.z
		);
	}
};

inline Matrix4 operator*(const Matrix4 & left, const Matrix4 & right) {
	Matrix4 result;

	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			result(i, j) = 
				left(i, 0) * right(0, j) +
				left(i, 1) * right(1, j) +
				left(i, 2) * right(2, j) +
				left(i, 3) * right(3, j);
		}
	}

	return result;
}
