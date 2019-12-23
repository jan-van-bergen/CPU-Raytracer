#include "Mesh.h"

#include <immintrin.h>

#include "Math.h"

void Mesh::init(const char * file_path) {
	mesh_data = MeshData::load(file_path);

	aabb_model = AABB::create_empty();

	for (int i = 0; i < mesh_data->triangle_bvh.primitive_count; i++) {
		aabb_model.expand(mesh_data->triangle_bvh.primitives[i].aabb);
	}

	aabb_model.fix_if_needed();
}

void Mesh::update() {
	transform.calc_world_matrix();

	aabb = AABB::transform(aabb_model, transform.world_matrix);

	transform_inv = Matrix4::invert(transform.world_matrix);
}

void Mesh::trace(const Ray & ray, RayHit & ray_hit, const Matrix4 & world, int bvh_step) const {
	// Transform the Ray into Model Space using the inverted World Space matrix of the Mesh
	Ray ray_model_space;
	ray_model_space.origin    = Matrix4::transform_position (transform_inv, ray.origin);
	ray_model_space.direction = Matrix4::transform_direction(transform_inv, ray.direction);

	mesh_data->triangle_bvh.trace(ray_model_space, ray_hit, transform.world_matrix);
}

SIMD_float Mesh::intersect(const Ray & ray, SIMD_float max_distance) const {
	// Transform the Ray into Model Space using the inverted World Space matrix of the Mesh
	Ray ray_model_space;
	ray_model_space.origin    = Matrix4::transform_position (transform_inv, ray.origin);
	ray_model_space.direction = Matrix4::transform_direction(transform_inv, ray.direction);

	return mesh_data->triangle_bvh.intersect(ray_model_space, max_distance);
}
	