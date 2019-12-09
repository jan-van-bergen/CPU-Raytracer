#pragma once
#include "Ray.h"
#include "RayHit.h"

struct AABB {
	Vector3 min;
	Vector3 max;

	inline float surface_area() {
		Vector3 diff = max - min;

		return 2.0f * (diff.x * diff.y + diff.y * diff.z + diff.z * diff.x);
	}

	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const;
};