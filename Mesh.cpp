#include "Mesh.h"

#include "Math.h"

void Mesh::init(const char * file_path) {
	bvh = BottomLevelBVH::load(file_path);
}

void Mesh::update() {
	transform.calc_world_matrix();

	aabb = AABB::transform(bvh->nodes[0].aabb, transform.world_matrix);

	transform_inv = Matrix4::invert(transform.world_matrix);
}

void Mesh::trace(const Ray & ray, RayHit & ray_hit, int bvh_step) const {
	// Transform the Ray into Model Space using the inverted World Space matrix of the Mesh
	Ray ray_model_space;
	ray_model_space.origin    = Matrix4::transform_position (transform_inv, ray.origin);
	ray_model_space.direction = Matrix4::transform_direction(transform_inv, ray.direction);

	ray_model_space.dO_dx = Matrix4::transform_position (transform_inv, ray.dO_dx);
	ray_model_space.dO_dy = Matrix4::transform_position (transform_inv, ray.dO_dy);
	ray_model_space.dD_dx = Matrix4::transform_direction(transform_inv, ray.dD_dx);
	ray_model_space.dD_dy = Matrix4::transform_direction(transform_inv, ray.dD_dy);

	bvh->trace(ray_model_space, ray_hit, transform.world_matrix);
}

SIMD_float Mesh::intersect(const Ray & ray, SIMD_float max_distance) const {
	// Transform the Ray into Model Space using the inverted World Space matrix of the Mesh
	Ray ray_model_space;
	ray_model_space.origin    = Matrix4::transform_position (transform_inv, ray.origin);
	ray_model_space.direction = Matrix4::transform_direction(transform_inv, ray.direction);

	return bvh->intersect(ray_model_space, max_distance);
}
	