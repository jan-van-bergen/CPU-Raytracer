#pragma once
#include <cassert>

#include "Transform.h"

#include "BottomLevelBVH.h"

struct Mesh {
	Transform transform;
	Matrix4 transform_inv;

	AABB aabb;
	
	const BottomLevelBVH * bvh = nullptr;
	
	void init(const char * file_path);

	void update();

	void trace(const Ray & ray, RayHit & ray_hit, int bvh_step) const;

	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const;

	inline Vector3 get_position() const {
		return transform.position;
	}
};
