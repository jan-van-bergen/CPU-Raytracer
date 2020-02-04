#pragma once
#include "Mesh.h"

#include "BVHBuilders.h"

struct TopLevelBVH {
	Mesh * primitives;
	int    primitive_count;

	int * indices;
	int   leaf_count;
	
	BVHNode * nodes;
	int       node_count;

	// Used for fast rebuilding
	int * indices_x;
	int * indices_y;
	int * indices_z;

	float * sah;
	int   * temp;

	void init(int count);

	void build_bvh();

	void update() const;

	void trace(const Ray & ray, RayHit & ray_hit, const Matrix4 & world) const;

	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const;
};
