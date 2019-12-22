#include "Mesh.h"

#include <immintrin.h>

#include "Math.h"

#define MESH_USE_BVH  0
#define MESH_USE_SBVH 1

#define MESH_ACCELERATOR MESH_USE_SBVH

void Mesh::init(const char * file_path) {
	mesh_data = MeshData::load(file_path);
	triangle_bvh.init(mesh_data->triangle_count);
	
	aabb_model = AABB::create_empty();

	// Copy Texture Coordinates and Material
	for (int i = 0; i < mesh_data->triangle_count; i++) {
		triangle_bvh.primitives[i] = mesh_data->triangles[i];

		Vector3 vertices[3] = { 
			triangle_bvh.primitives[i].position0, 
			triangle_bvh.primitives[i].position1, 
			triangle_bvh.primitives[i].position2 
		};
		triangle_bvh.primitives[i].aabb = AABB::from_points(vertices, 3);

		aabb_model.expand(triangle_bvh.primitives[i].aabb);
	}

	aabb_model.fix_if_needed();

#if MESH_ACCELERATOR == MESH_USE_BVH
	{
		ScopedTimer timer("Mesh BVH Construction");
		triangle_bvh.build_bvh();
	}
#elif MESH_ACCELERATOR == MESH_USE_SBVH
	{
		ScopedTimer timer("Mesh SBVH Construction");
		triangle_bvh.build_sbvh();
	}
#endif
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

	triangle_bvh.trace(ray_model_space, ray_hit, transform.world_matrix);
}

SIMD_float Mesh::intersect(const Ray & ray, SIMD_float max_distance) const {
	// Transform the Ray into Model Space using the inverted World Space matrix of the Mesh
	Ray ray_model_space;
	ray_model_space.origin    = Matrix4::transform_position (transform_inv, ray.origin);
	ray_model_space.direction = Matrix4::transform_direction(transform_inv, ray.direction);

	return triangle_bvh.intersect(ray_model_space, max_distance);
}
	