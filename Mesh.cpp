#include "Mesh.h"

#include <immintrin.h>

#include "Math.h"

void Mesh::update(const Matrix4 & world_matrix) {
	for (int i = 0; i < mesh_data->vertex_count; i++) {
		world_positions[i] = Matrix4::transform_position (world_matrix, mesh_data->positions[i]);
		world_normals  [i] = Matrix4::transform_direction(world_matrix, mesh_data->normals  [i]);
	}
}

void Mesh::trace(const Ray & ray, RayHit & ray_hit) const {
	// Iterate over all Triangles in the Mesh
	for (int i = 0; i < mesh_data->vertex_count; i += 3) {
		SIMD_Vector3 edge0(world_positions[i+1] - world_positions[i]);
		SIMD_Vector3 edge1(world_positions[i+2] - world_positions[i]);

		SIMD_Vector3 h = SIMD_Vector3::cross(ray.direction, edge1);
		SIMD_float   a = SIMD_Vector3::dot(edge0, h);

		// If the ray is parallel to the plane formed by 
		// the triangle no intersection is possible
		SIMD_float mask = (a < -Ray::EPSILON) | (a > Ray::EPSILON);
		if (SIMD_float::all_false(mask)) continue;

		SIMD_float f   = SIMD_float::rcp(a);
		SIMD_Vector3 s = ray.origin - SIMD_Vector3(world_positions[i]);
		SIMD_float u   = f * SIMD_Vector3::dot(s, h);

		// If the barycentric coordinate on the edge between vertices i and i+1 
		// is outside the interval [0, 1] we know no intersection is possible
		mask = mask & (u > SIMD_float(0.0f));
		mask = mask & (u < SIMD_float(1.0f));
		if (SIMD_float::all_false(mask)) continue;

		SIMD_Vector3 q = SIMD_Vector3::cross(s, edge0);
		SIMD_float   v = f * SIMD_Vector3::dot(ray.direction, q);

		// If the barycentric coordinate on the edge between vertices i and i+2 
		// is outside the interval [0, 1] we know no intersection is possible
		mask = mask & (v       > SIMD_float(0.0f));
		mask = mask & ((u + v) < SIMD_float(1.0f));
		if (SIMD_float::all_false(mask)) continue;

		SIMD_float t = f * SIMD_Vector3::dot(edge1, q);

		// Check if we are in the right distance range
		mask = mask & (t > Ray::EPSILON);
		mask = mask & (t < ray_hit.distance);

		int int_mask = SIMD_float::mask(mask);
		if (int_mask == 0x0) continue;
		
		ray_hit.hit      = ray_hit.hit | mask;
		ray_hit.distance = SIMD_float::blend(ray_hit.distance, t, mask);

		ray_hit.point  = SIMD_Vector3::blend(ray_hit.point,  ray.origin + ray.direction * t, mask);
		ray_hit.normal = SIMD_Vector3::blend(ray_hit.normal, SIMD_Vector3::normalize(Math::barycentric(SIMD_Vector3(world_normals[i]), SIMD_Vector3(world_normals[i+1]), SIMD_Vector3(world_normals[i+2]), u, v)), mask);

		// Obtain u,v by barycentric interpolation of the texture coordinates of the three current vertices
		//SIMD_Vector3 tex_coords = Math::barycentric(mesh_data->tex_coords[i], mesh_data->tex_coords[i+1], mesh_data->tex_coords[i+2], u, v);
		ray_hit.u = SIMD_float::blend(ray_hit.u, SIMD_float(0.5f), mask);//tex_coords.x;
		ray_hit.v = SIMD_float::blend(ray_hit.v, SIMD_float(0.5f), mask);//tex_coords.y;
			
		for (int i = 0; i < SIMD_LANE_SIZE; i++) {
			if (int_mask & (1 << i)) {
				ray_hit.material[i] = &mesh_data->material;
			}
		}
	}
}

bool Mesh::intersect(const Ray & ray, SIMD_float max_distance) const {
	// Iterate over all Triangles in the Mesh
	//for (int i = 0; i < mesh_data->vertex_count; i += 3) {
	//	Vector3 edge0 = world_positions[i+1] - world_positions[i];
	//	Vector3 edge1 = world_positions[i+2] - world_positions[i];

	//	Vector3 h = Vector3::cross(ray.direction, edge1);
	//	float a = Vector3::dot(edge0, h);

	//	// If the ray is parallel to the plane formed by 
	//	// the triangle no intersection is possible
	//	if (a > -EPSILON && a < EPSILON) continue;

	//	float f = 1.0f / a;
	//	Vector3 s = ray.origin - world_positions[i];
	//	float u = f * Vector3::dot(s, h);

	//	// If the barycentric coordinate on the edge between vertices i and i+1 
	//	// is outside the interval [0, 1] we know no intersection is possible
	//	if (u < 0.0f || u > 1.0f) continue;

	//	Vector3 q = Vector3::cross(s, edge0);
	//	float v = f * Vector3::dot(ray.direction, q);

	//	// If the barycentric coordinate on the edge between vertices i and i+2 
	//	// is outside the interval [0, 1] we know no intersection is possible
	//	if (v < 0.0f || u + v > 1.0f) continue;

	//	float t = f * Vector3::dot(edge1, q);

	//	// Check if we are in the right distance range
	//	if (t < EPSILON || t > max_distance) continue;
	//	
	//	return true;
	//}

	return false;
}
