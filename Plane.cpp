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

	const SIMD_float   one (1.0f);
	const SIMD_Vector3 zero(0.0f);

	ray_hit.hit      = ray_hit.hit | mask;
	ray_hit.distance = SIMD_float::blend(ray_hit.distance, t, mask);

	ray_hit.point  = SIMD_Vector3::blend(ray_hit.point, ray.origin + t * ray.direction, mask);
	ray_hit.normal = SIMD_Vector3::blend(ray_hit.normal, normal,                        mask);

	ray_hit.material_id = SIMD_int::blend(ray_hit.material_id, SIMD_int(material_id), *reinterpret_cast<SIMD_int *>(&mask));

	SIMD_Vector3 u(u_axis);
	SIMD_Vector3 v(v_axis);

	// Obtain u,v by projecting the hit point onto the u and v axes
	ray_hit.u = SIMD_float::blend(ray_hit.u, SIMD_Vector3::dot(ray_hit.point, u), mask);
	ray_hit.v = SIMD_float::blend(ray_hit.v, SIMD_Vector3::dot(ray_hit.point, v), mask);
	
#if RAY_DIFFERENTIALS_ENABLED
	// Formulae for Transfer Ray Differential from Igehy 99
	SIMD_Vector3 dP_dx_plus_t_dD_dx = SIMD_Vector3::madd(ray.dD_dx, t, ray.dO_dx);
	SIMD_Vector3 dP_dy_plus_t_dD_dy = SIMD_Vector3::madd(ray.dD_dy, t, ray.dO_dy);

	SIMD_float denom = -one / (SIMD_Vector3::dot(ray.direction, ray_hit.normal) + SIMD_float(1e-8f));
	SIMD_float dt_dx = SIMD_Vector3::dot(dP_dx_plus_t_dD_dx, ray_hit.normal) * denom;
	SIMD_float dt_dy = SIMD_Vector3::dot(dP_dy_plus_t_dD_dy, ray_hit.normal) * denom;
	
	SIMD_Vector3 dP_dx = SIMD_Vector3::madd(ray.direction, dt_dx, dP_dx_plus_t_dD_dx);
	SIMD_Vector3 dP_dy = SIMD_Vector3::madd(ray.direction, dt_dy, dP_dy_plus_t_dD_dy);

	ray_hit.dO_dx = SIMD_Vector3::blend(ray_hit.dO_dx, dP_dx, mask);
	ray_hit.dO_dy = SIMD_Vector3::blend(ray_hit.dO_dy, dP_dy, mask);

	// Normal does not depend on screenspace coordinates x,y
	// Thus, the derivative is zero
	ray_hit.dN_dx = SIMD_Vector3::blend(ray_hit.dN_dx, zero, mask);
	ray_hit.dN_dy = SIMD_Vector3::blend(ray_hit.dN_dy, zero, mask);

	// Formulae derived by differentiating the above formulae for u and v
	ray_hit.ds_dx = SIMD_float::blend(ray_hit.ds_dx, SIMD_Vector3::dot(dP_dx, u), mask);
	ray_hit.ds_dy = SIMD_float::blend(ray_hit.ds_dy, SIMD_Vector3::dot(dP_dy, u), mask);

	ray_hit.dt_dx = SIMD_float::blend(ray_hit.dt_dx, SIMD_Vector3::dot(dP_dx, v), mask);
	ray_hit.dt_dy = SIMD_float::blend(ray_hit.dt_dy, SIMD_Vector3::dot(dP_dy, v), mask);
#endif
}

SIMD_float Plane::intersect(const Ray & ray, SIMD_float max_distance) const {
	SIMD_Vector3 normal  (world_normal);
	SIMD_float   distance(world_distance);

	// Solve plane equation for t
	SIMD_float t = -(SIMD_Vector3::dot(normal, ray.origin) + distance) / SIMD_Vector3::dot(normal, ray.direction);

	return (t > Ray::EPSILON) & (t < max_distance);
}
