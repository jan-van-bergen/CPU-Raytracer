#include "PrimitiveSet.h"

#include "Math.h"

void Triangles::trace(int index, const Ray & ray, RayHit & ray_hit) const {
	assert(index >= 0 && index < triangle_count);

	const SIMD_float zero(0.0f);
	const SIMD_float one (1.0f);
	
	const SIMD_float pos_epsilon( Ray::EPSILON);
	const SIMD_float neg_epsilon(-Ray::EPSILON);

	SIMD_Vector3 edge0(positions[index].position1 - positions[index].position0);
	SIMD_Vector3 edge1(positions[index].position2 - positions[index].position0);

	SIMD_Vector3 h = SIMD_Vector3::cross(ray.direction, edge1);
	SIMD_float   a = SIMD_Vector3::dot(edge0, h);

	// If the ray is parallel to the plane formed by 
	// the triangle no intersection is possible
	SIMD_float mask = (a < neg_epsilon) | (a > pos_epsilon);
	if (SIMD_float::all_false(mask)) return;

	SIMD_float   f = SIMD_float::rcp(a);
	SIMD_Vector3 s = ray.origin - SIMD_Vector3(positions[index].position0);
	SIMD_float   u = f * SIMD_Vector3::dot(s, h);

	// If the barycentric coordinate on the edge between vertices i and i+1 
	// is outside the interval [0, 1] we know no intersection is possible
	mask = mask & (u > zero);
	mask = mask & (u < one);
	if (SIMD_float::all_false(mask)) return;

	SIMD_Vector3 q = SIMD_Vector3::cross(s, edge0);
	SIMD_float   v = f * SIMD_Vector3::dot(ray.direction, q);

	// If the barycentric coordinate on the edge between vertices i and i+2 
	// is outside the interval [0, 1] we know no intersection is possible
	mask = mask & (v       > zero);
	mask = mask & ((u + v) < one);
	if (SIMD_float::all_false(mask)) return;

	SIMD_float t = f * SIMD_Vector3::dot(edge1, q);

	// Check if we are in the right distance range
	mask = mask & (t > pos_epsilon);
	mask = mask & (t < ray_hit.distance);

	int int_mask = SIMD_float::mask(mask);
	if (int_mask == 0x0) return;
		
	ray_hit.hit      = ray_hit.hit | mask;
	ray_hit.distance = SIMD_float::blend(ray_hit.distance, t, mask);

	ray_hit.point  = SIMD_Vector3::blend(ray_hit.point,  ray.origin + ray.direction * t, mask);
	ray_hit.normal = SIMD_Vector3::blend(ray_hit.normal, 
		SIMD_Vector3::normalize(Math::barycentric(
			SIMD_Vector3(normals[index].normal0), 
			SIMD_Vector3(normals[index].normal1), 
			SIMD_Vector3(normals[index].normal2), 
		u, v)), mask);

	// Obtain u,v by barycentric interpolation of the texture coordinates of the three current vertices
	SIMD_Vector3 tex_coord0(Vector3(mesh_data->tex_coords[index].tex_coord0.x, mesh_data->tex_coords[index].tex_coord0.y, 1.0f));
	SIMD_Vector3 tex_coord1(Vector3(mesh_data->tex_coords[index].tex_coord1.x, mesh_data->tex_coords[index].tex_coord1.y, 1.0f));
	SIMD_Vector3 tex_coord2(Vector3(mesh_data->tex_coords[index].tex_coord2.x, mesh_data->tex_coords[index].tex_coord2.y, 1.0f));

	SIMD_Vector3 tex_coords = Math::barycentric(tex_coord0, tex_coord1, tex_coord2, u, v);
	ray_hit.u = SIMD_float::blend(ray_hit.u, tex_coords.x, mask);
	ray_hit.v = SIMD_float::blend(ray_hit.v, tex_coords.y, mask);
		
	for (int j = 0; j < SIMD_LANE_SIZE; j++) {
		if (int_mask & (1 << j)) {
			ray_hit.material[j] = &mesh_data->materials[mesh_data->material_ids[index / 3]];
		}
	}
}

SIMD_float Triangles::intersect(int index, const Ray & ray, SIMD_float max_distance) const {
	assert(index >= 0 && index < triangle_count);

	const SIMD_float zero(0.0f);
	const SIMD_float one (1.0f);

	const SIMD_float pos_epsilon( Ray::EPSILON);
	const SIMD_float neg_epsilon(-Ray::EPSILON);

	SIMD_Vector3 edge0(positions[index].position1 - positions[index].position0);
	SIMD_Vector3 edge1(positions[index].position2 - positions[index].position0);

	SIMD_Vector3 h = SIMD_Vector3::cross(ray.direction, edge1);
	SIMD_float   a = SIMD_Vector3::dot(edge0, h);

	// If the ray is parallel to the plane formed by 
	// the triangle no intersection is possible
	SIMD_float mask = (a < neg_epsilon) | (a > pos_epsilon);
	if (SIMD_float::all_false(mask)) return mask;

	SIMD_float   f = SIMD_float::rcp(a);
	SIMD_Vector3 s = ray.origin - SIMD_Vector3(positions[index].position0);
	SIMD_float   u = f * SIMD_Vector3::dot(s, h);

	// If the barycentric coordinate on the edge between vertices i and i+1 
	// is outside the interval [0, 1] we know no intersection is possible
	mask = mask & (u > zero);
	mask = mask & (u < one);
	if (SIMD_float::all_false(mask)) return mask;

	SIMD_Vector3 q = SIMD_Vector3::cross(s, edge0);
	SIMD_float   v = f * SIMD_Vector3::dot(ray.direction, q);

	// If the barycentric coordinate on the edge between vertices i and i+2 
	// is outside the interval [0, 1] we know no intersection is possible
	mask = mask & (v       > zero);
	mask = mask & ((u + v) < one);
	if (SIMD_float::all_false(mask)) return mask;

	SIMD_float t = f * SIMD_Vector3::dot(edge1, q);

	// Check if we are in the right distance range
	mask = mask & (t > pos_epsilon);
	mask = mask & (t < max_distance);

	return mask;
}