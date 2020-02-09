#include "Plane.h"

void Plane::update() {
	transform.calc_world_matrix();

	world_normal   =  Matrix4::transform_direction(transform.world_matrix, Vector3(0.0f, 1.0f, 0.0f));
	world_distance = -Vector3::dot(world_normal, transform.position);

	u_axis = Matrix4::transform_direction(transform.world_matrix, Vector3(1.0f, 0.0f, 0.0f));
	v_axis = Vector3::cross(u_axis, world_normal);
}

void Plane::trace(const Ray & ray, RayHit & ray_hit) const {
	SIMD_Vector3 normal  (world_normal);
	SIMD_float   distance(world_distance);

	// Solve plane equation for t
	SIMD_float t = -(SIMD_Vector3::dot(normal, ray.origin) + distance) / SIMD_Vector3::dot(normal, ray.direction);

	SIMD_float mask = (t > Ray::EPSILON) & (t < ray_hit.distance);

	if (SIMD_float::all_false(mask)) return;

	ray_hit.hit      = ray_hit.hit | mask;
	ray_hit.distance = SIMD_float::blend(ray_hit.distance, t, mask);

	ray_hit.point  = SIMD_Vector3::blend(ray_hit.point,  ray.origin + t * ray.direction, mask);
	ray_hit.normal = SIMD_Vector3::blend(ray_hit.normal, normal,                         mask);

	ray_hit.material_id = SIMD_int::blend(ray_hit.material_id, SIMD_int(material_id), *reinterpret_cast<SIMD_int *>(&mask));

	SIMD_Vector3 u(u_axis);
	SIMD_Vector3 v(v_axis);

	// Obtain u,v by projecting the hit point onto the u and v axes
	ray_hit.u = SIMD_float::blend(ray_hit.u, SIMD_Vector3::dot(ray_hit.point, u), mask);
	ray_hit.v = SIMD_float::blend(ray_hit.v, SIMD_Vector3::dot(ray_hit.point, v), mask);
}

SIMD_float Plane::intersect(const Ray & ray, SIMD_float max_distance) const {
	SIMD_Vector3 normal  (world_normal);
	SIMD_float   distance(world_distance);

	// Solve plane equation for t
	SIMD_float t = -(SIMD_Vector3::dot(normal, ray.origin) + distance) / SIMD_Vector3::dot(normal, ray.direction);

	return (t > Ray::EPSILON) & (t < max_distance);
}
