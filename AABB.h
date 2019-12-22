#pragma once
#include "Ray.h"
#include "RayHit.h"

#include "Matrix4.h"

struct AABB {
	Vector3 min;
	Vector3 max;
	
	inline bool is_valid() const {
		return max.x > min.x && max.y > min.y && max.z > min.z;
	}

	inline bool is_empty() const {
		return 
			min.x ==  INFINITY && min.y ==  INFINITY && min.z ==  INFINITY &&
			max.x == -INFINITY && max.y == -INFINITY && max.z == -INFINITY;
	}

	// Make sure the AABB is non-zero along every dimension
	inline void fix_if_needed() {
		for (int dimension = 0; dimension < 3; dimension++) {
			if (max[dimension] - min[dimension] < 0.001f) {
				max[dimension] += 0.005f;
			}
		}
	}

	inline float surface_area() const {
		assert(is_valid() || is_empty());

		Vector3 diff = max - min;

		return 2.0f * (diff.x * diff.y + diff.y * diff.z + diff.z * diff.x);
	}

	inline void expand(const Vector3 & point) {
		min = Vector3::min(min, point);
		max = Vector3::max(max, point);
	}

	inline void expand(const AABB & aabb) {
		min = Vector3::min(min, aabb.min);
		max = Vector3::max(max, aabb.max);
	}

	inline static AABB overlap(const AABB & b1, const AABB & b2) {
		assert(b1.is_valid() || b1.is_empty());
		assert(b2.is_valid() || b2.is_empty());

		AABB aabb;
		aabb.min = Vector3::max(b1.min, b2.min);
		aabb.max = Vector3::min(b1.max, b2.max);

		if (!aabb.is_valid()) aabb = create_empty();

		return aabb;
	}
	
	inline static float overlap_surface_area(const AABB & b1, const AABB & b2) {
		assert(b1.is_valid());
		assert(b2.is_valid());

		AABB aabb;

		if (!intersect(b1, b2)) return 0.0f;

		aabb.min = Vector3::max(b1.min, b2.min);
		aabb.max = Vector3::min(b1.max, b2.max);

		aabb.fix_if_needed();
		assert(aabb.is_valid());

		return aabb.surface_area();
	}

	inline static AABB create_empty() {
		AABB aabb;
		aabb.min = Vector3(+INFINITY);
		aabb.max = Vector3(-INFINITY);

		return aabb;
	}

	inline static AABB from_points(const Vector3 * points, int point_count) {
		AABB aabb = create_empty();

		for (int i = 0; i < point_count; i++) {
			aabb.min = Vector3::min(aabb.min, points[i]);
			aabb.max = Vector3::max(aabb.max, points[i]);
		}

		aabb.fix_if_needed();
		assert(aabb.is_valid());

		return aabb;
	}

	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const;

	inline static bool intersect(const AABB & b1, const AABB & b2) {
		if (b2.min.x > b1.max.x || b2.min.y > b1.max.y || b2.min.z > b1.max.z) return false; 
		if (b1.min.x > b2.max.x || b1.min.y > b2.max.y || b1.min.z > b2.max.z) return false; 

		return true;		
	}

	inline static AABB transform(const AABB & aabb, const Matrix4 & transformation) {
		Vector3 corners[8] = {
			Vector3(aabb.min.x, aabb.min.y, aabb.min.z),
			Vector3(aabb.min.x, aabb.min.y, aabb.max.z),
			Vector3(aabb.max.x, aabb.min.y, aabb.max.z),
			Vector3(aabb.max.x, aabb.min.y, aabb.min.z),
			Vector3(aabb.min.x, aabb.max.y, aabb.min.z),
			Vector3(aabb.min.x, aabb.max.y, aabb.max.z),
			Vector3(aabb.max.x, aabb.max.y, aabb.max.z),
			Vector3(aabb.max.x, aabb.max.y, aabb.min.z)
		};

		for (int i = 0; i < 8; i++) {
			corners[i] = Matrix4::transform_position(transformation, corners[i]);
		}

		return AABB::from_points(corners, 8);
	}
};
