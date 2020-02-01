#pragma once
#include "SIMD.h"
#include "Material.h"

struct RayHit {
	SIMD_float hit;
	SIMD_float distance;

	SIMD_Vector3 point;  // Coordinates of the hit in World Space
	SIMD_Vector3 normal; // Normal      of the hit in World Space

	int material_id[SIMD_LANE_SIZE] = { 0 };
	SIMD_float u, v;

	SIMD_float bvh_steps;

	inline RayHit() {
		hit      = SIMD_float(0.0f);
		distance = SIMD_float(INFINITY);

		bvh_steps = SIMD_float(0);
	}
};
