#pragma once
#include "BVHBuilders.h"

struct BottomLevelBVH {
	// Triangles in SoA layout
	Vector3 * position0;
	Vector3 * position_edge1;
	Vector3 * position_edge2;

	Vector2 * tex_coord0;
	Vector2 * tex_coord_edge1;
	Vector2 * tex_coord_edge2;

	Vector3 * normal0;
	Vector3 * normal_edge1;
	Vector3 * normal_edge2;

	int * material_id;
	int   material_offset;

	int primitive_count;

	int * indices;
	int   index_count;
	
	BVHNode * nodes;
	int       node_count;

	static BottomLevelBVH * load(const char * filename);

	void trace(const Ray & ray, RayHit & ray_hit, const Matrix4 & world) const;
	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const;

private:
	void init(int count);

	void build_bvh (const Triangle * triangles);
	void build_sbvh(const Triangle * triangles);

	void save_to_disk  (const char * bvh_filename) const;
	void load_from_disk(const char * bvh_filename);
	
	void       triangle_soa_trace    (int index, const Ray & ray, RayHit & ray_hit, const Matrix4 & world, int bvh_step) const;
	SIMD_float triangle_soa_intersect(int index, const Ray & ray, SIMD_float max_distance) const;
};
