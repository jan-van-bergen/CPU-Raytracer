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

	inline void expand(const Vector3 & point) {
		min = Vector3::min(min, point);
		max = Vector3::max(max, point);
	}

	inline void expand(const AABB & aabb) {
		min = Vector3::min(min, aabb.min);
		max = Vector3::max(max, aabb.max);
	}

	inline void debug(int index) const {
		Vector3 vertices[8] = {
			Vector3(min.x, min.y, min.z),
			Vector3(min.x, min.y, max.z),
			Vector3(max.x, min.y, max.z),
			Vector3(max.x, min.y, min.z),
			Vector3(min.x, max.y, min.z),
			Vector3(min.x, max.y, max.z),
			Vector3(max.x, max.y, max.z),
			Vector3(max.x, max.y, min.z)
		};

		int faces[36] = {
			1, 2, 3, 1, 3, 4,
			1, 2, 6, 1, 6, 5,
			1, 5, 8, 1, 8, 4,
			4, 8, 7, 4, 7, 3,
			3, 7, 6, 3, 6, 2,
			5, 6, 7, 5, 7, 8
		};

		printf("o Debug_AABB%i\n", index);
		for (int v = 0; v < 8; v++) {
			printf("v %f %f %f\n", vertices[v].x, vertices[v].y, vertices[v].z);
		}

		for (int f = 0; f < 36; f += 3) {
			printf("f %i %i %i\n", 8*index + faces[f], 8*index + faces[f+1], 8*index + faces[f+2]);
		}
	}

	inline static AABB overlap(const AABB & b1, const AABB & b2) {
		AABB aabb;

		aabb.max = Vector3::min(Vector3::max(b1.max, b2.min), Vector3::max(b1.min, b2.max));
		aabb.min = Vector3::max(Vector3::min(b1.max, b2.min), Vector3::min(b1.min, b2.max));

		return aabb;
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

		return aabb;
	}

	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const;
};
