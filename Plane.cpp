#include "Plane.h"

void Plane::update() {
	transform.calc_world_matrix();

	world_normal   =  Matrix4::transform_direction(transform.world_matrix, Vector3(0.0f, 1.0f, 0.0f));
	world_distance = -Vector3::dot(world_normal, transform.position);

	u_axis = Matrix4::transform_direction(transform.world_matrix, Vector3(1.0f, 0.0f, 0.0f));
	v_axis = Vector3::cross(u_axis, world_normal);
}

void Plane::trace(const Ray & ray, RayHit & ray_hit) const {
	SIMD_Vector3 normal(world_normal);
	__m128 distance = _mm_set1_ps(world_distance);

	// Solve plane equation for t
	__m128 t = _mm_sub_ps(_mm_set1_ps(0.0f), _mm_div_ps(_mm_add_ps(SIMD_Vector3::dot(normal, ray.origin), distance), SIMD_Vector3::dot(normal, ray.direction)));

	__m128 mask = _mm_and_ps(_mm_cmpgt_ps(t, Ray::EPSILON), _mm_cmplt_ps(t, ray_hit.distance));
	int int_mask = _mm_movemask_ps(mask); 
	if (int_mask == 0x0) return;

	ray_hit.hit      = _mm_or_ps(ray_hit.hit, mask);
	ray_hit.distance = _mm_blendv_ps(ray_hit.distance, t, mask);

	ray_hit.point  = SIMD_Vector3::blend(ray_hit.point,  ray.origin + t * ray.direction, mask);
	ray_hit.normal = SIMD_Vector3::blend(ray_hit.normal, normal,                         mask);

	SIMD_Vector3 u(u_axis);
	SIMD_Vector3 v(v_axis);

	// Obtain u,v by projecting the hit point onto the u and v axes
	ray_hit.u = _mm_blendv_ps(ray_hit.u, SIMD_Vector3::dot(ray_hit.point, u), mask);
	ray_hit.v = _mm_blendv_ps(ray_hit.v, SIMD_Vector3::dot(ray_hit.point, v), mask);

	if (int_mask & 8) ray_hit.material[3] = &material;
	if (int_mask & 4) ray_hit.material[2] = &material;
	if (int_mask & 2) ray_hit.material[1] = &material;
	if (int_mask & 1) ray_hit.material[0] = &material;
}

bool Plane::intersect(const Ray & ray, __m128 max_distance) const {
	return false;

	// Solve plane equation for t
	//float t = -(Vector3::dot(world_normal, ray.origin) + world_distance) / (Vector3::dot(world_normal, ray.direction));
	//
	//// Check if we are in the right distance range
	//return t > EPSILON && t < max_distance;
}
