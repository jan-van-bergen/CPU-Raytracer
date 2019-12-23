#pragma once
#include <cassert>

#include "MeshData.h"
#include "Transform.h"

#include "Ray.h"
#include "RayHit.h"

struct Mesh {
	Transform transform;

	const MeshData * mesh_data = nullptr;

	void init(const char * file_path);

	void update();

	void trace(const Ray & ray, RayHit & ray_hit, const Matrix4 & world, int bvh_step) const;

	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const;
	
	// BVH Related methods
	AABB aabb_model;
	AABB aabb;

	Matrix4 transform_inv;

	inline Vector3 get_position() const {
		return transform.position;
	}
};
