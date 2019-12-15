#pragma once
#include "Ray.h"
#include "RayHit.h"

struct AABB {
	Vector3 min;
	Vector3 max;

	inline float surface_area() const {
		Vector3 diff = max - min;

		return 2.0f * (diff.x * diff.y + diff.y * diff.z + diff.z * diff.x);
	}

	inline void expand(const AABB & aabb) {
		min = Vector3::min(min, aabb.min);
		max = Vector3::max(max, aabb.max);
	}

	inline static AABB overlap(const AABB & b1, const AABB & b2) {
		AABB aabb;

		aabb.max = Vector3::min(Vector3::max(b1.max, b2.min), Vector3::max(b1.min, b2.max));
		aabb.min = Vector3::max(Vector3::min(b1.max, b2.min), Vector3::min(b1.min, b2.max));

		return aabb;
	}

	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const;
};
