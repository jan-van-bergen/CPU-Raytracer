#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4.h"

struct Transform {
	Vector3    position;
	Quaternion rotation;

	Matrix4 world_matrix;

	// Constructs world matrix from current position and rotation
	inline void calc_world_matrix() {
		float xx = rotation.x * rotation.x;
		float yy = rotation.y * rotation.y;
		float zz = rotation.z * rotation.z;
		float xz = rotation.x * rotation.z;
		float xy = rotation.x * rotation.y;
		float yz = rotation.y * rotation.z;
		float wx = rotation.w * rotation.x;
		float wy = rotation.w * rotation.y;
		float wz = rotation.w * rotation.z;

		world_matrix(0, 0) = 1.0f - 2.0f * (yy + zz);
		world_matrix(0, 1) =        2.0f * (xy + wz);
		world_matrix(0, 2) =        2.0f * (xz - wy);
		//world_matrix(0, 3) = 0.0f;

		world_matrix(1, 0) =        2.0f * (xy - wz);
		world_matrix(1, 1) = 1.0f - 2.0f * (xx + zz);
		world_matrix(1, 2) =        2.0f * (yz + wx);
		//world_matrix(1, 3) = 0.0f;

		world_matrix(2, 0) =        2.0f * (xz + wy);
		world_matrix(2, 1) =        2.0f * (yz - wx);
		world_matrix(2, 2) = 1.0f - 2.0f * (xx + yy);
		//world_matrix(2, 3) = 0.0f;

		world_matrix(3, 0) = position.x;
		world_matrix(3, 1) = position.y;
		world_matrix(3, 2) = position.z;
		//world_matrix(3, 3) = 1.0f;
	}
};
