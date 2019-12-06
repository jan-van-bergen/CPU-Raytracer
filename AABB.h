#pragma once
#include "Ray.h"
#include "RayHit.h"

struct AABB {
	Vector3 min;
	Vector3 max;

	inline void init(const Vector3 & min_bound, const Vector3 & max_bound) {
		min = min_bound;
		max = max_bound;
	}

	void trace(const Ray & ray, RayHit & ray_hit) const;

	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const;
};
