#include "Sphere.h"

#include "Util.h"

void Sphere::update() {
	transform.calc_world_matrix();
}

void Sphere::trace(const Ray & ray, RayHit & ray_hit) const {
	const SIMD_float epsilon(Ray::EPSILON);
	const SIMD_float two(2.0f);

	SIMD_Vector3 center(transform.position);
	SIMD_float r2(radius_squared);

    SIMD_Vector3 oc = ray.origin - center;
	// Solve quadratic equation using abc formula
    SIMD_float a = SIMD_Vector3::length_squared(ray.direction);
    SIMD_float b = two * SIMD_Vector3::dot(oc, ray.direction);
    SIMD_float c = SIMD_Vector3::length_squared(oc) - r2;

	// Compute disciminant
    SIMD_float d = b*b - SIMD_float(4.0f) * a * c;

	// If all discriminants are invalid, early out
	SIMD_float mask = d >= SIMD_float(0.0f);
	if (SIMD_float::all_false(mask)) return;

	SIMD_float sqrt_d    =  SIMD_float::sqrt(d);
	SIMD_float inv_denom = -SIMD_float::rcp(two * a);

    SIMD_float t0 = (b + sqrt_d) * inv_denom;
    SIMD_float t1 = (b - sqrt_d) * inv_denom;

	// t should be t0, unless t0 is behind the ray
	SIMD_float t = SIMD_float::blend(t1, t0, t0 > epsilon);

	// Check if t is in the valid range
	mask = mask & (t > epsilon);
	mask = mask & (t < ray_hit.distance);

	if (SIMD_float::all_false(mask)) return;

	const SIMD_float half(0.5f);
	const SIMD_float one (1.0f);
	const SIMD_float non_zero(1e-8f);
	const SIMD_float one_over_r(radius_inv);
	const SIMD_float one_over_pi    (ONE_OVER_PI);
	const SIMD_float one_over_two_pi(ONE_OVER_TWO_PI);

	ray_hit.hit      = ray_hit.hit | mask;
	ray_hit.distance = SIMD_float::blend(ray_hit.distance, t, mask);

	ray_hit.point  = SIMD_Vector3::blend(ray_hit.point, ray.origin + t * ray.direction,         mask);
	ray_hit.normal = SIMD_Vector3::blend(ray_hit.normal, (ray_hit.point - center) * one_over_r, mask);
	
	ray_hit.material_id = SIMD_int::blend(ray_hit.material_id, SIMD_int(material_id), *reinterpret_cast<SIMD_int *>(&mask));

	// Obtain u,v by converting the normal direction to spherical coordinates
	ray_hit.u = SIMD_float::blend(ray_hit.u, SIMD_float::madd(SIMD_float::atan2(ray_hit.normal.z, ray_hit.normal.x), one_over_two_pi, half), mask);
	ray_hit.v = SIMD_float::blend(ray_hit.v, SIMD_float::madd(SIMD_float::acos (ray_hit.normal.y),                   one_over_pi,     half), mask);

	// Formulae for Transfer Ray Differential from Igehy 99
	SIMD_Vector3 dP_dx_plus_t_dD_dx = SIMD_Vector3::madd(ray.dD_dx, t, ray.dO_dx);
	SIMD_Vector3 dP_dy_plus_t_dD_dy = SIMD_Vector3::madd(ray.dD_dy, t, ray.dO_dy);

	SIMD_float denom = -one / SIMD_Vector3::dot(ray.direction, ray_hit.normal);
	SIMD_float dt_dx = SIMD_Vector3::dot(dP_dx_plus_t_dD_dx, ray_hit.normal) * denom;
	SIMD_float dt_dy = SIMD_Vector3::dot(dP_dy_plus_t_dD_dy, ray_hit.normal) * denom;

	SIMD_Vector3 dP_dx = SIMD_Vector3::madd(ray.direction, dt_dx, dP_dx_plus_t_dD_dx);
	SIMD_Vector3 dP_dy = SIMD_Vector3::madd(ray.direction, dt_dy, dP_dy_plus_t_dD_dy);

	ray_hit.dO_dx = SIMD_Vector3::blend(ray_hit.dO_dx, dP_dx, mask);
	ray_hit.dO_dy = SIMD_Vector3::blend(ray_hit.dO_dy, dP_dy, mask);

	ray_hit.dN_dx = SIMD_Vector3::blend(ray_hit.dN_dx, dP_dx * one_over_r, mask);
	ray_hit.dN_dy = SIMD_Vector3::blend(ray_hit.dN_dy, dP_dy * one_over_r, mask);

	// Formulae derived by differentiating the above formulae for u and v
	SIMD_float ds_denom = one_over_two_pi / (ray_hit.normal.x * ray_hit.normal.x + ray_hit.normal.z * ray_hit.normal.z + non_zero);
	ray_hit.ds_dx = SIMD_float::blend(ray_hit.ds_dx, (ray_hit.normal.x * ray_hit.dN_dx.z - ray_hit.normal.z * ray_hit.dN_dx.x) * ds_denom, mask); 
	ray_hit.ds_dy = SIMD_float::blend(ray_hit.ds_dy, (ray_hit.normal.x * ray_hit.dN_dy.z - ray_hit.normal.z * ray_hit.dN_dy.x) * ds_denom, mask); 

	SIMD_float dt_denom = -one_over_pi * SIMD_float::inv_sqrt(one - ray_hit.normal.y*ray_hit.normal.y + non_zero);
	ray_hit.dt_dx = SIMD_float::blend(ray_hit.dt_dx, ray_hit.dN_dx.y * dt_denom, mask);
	ray_hit.dt_dy = SIMD_float::blend(ray_hit.dt_dy, ray_hit.dN_dy.y * dt_denom, mask);
}

SIMD_float Sphere::intersect(const Ray & ray, SIMD_float max_distance) const {
	SIMD_Vector3 center(transform.position);

	SIMD_Vector3 c = center - ray.origin;
	SIMD_float   t = SIMD_Vector3::dot(c, ray.direction);

	SIMD_Vector3 Q  = c - t * ray.direction;
	SIMD_float   p2 = SIMD_Vector3::dot(Q, Q);
	
	SIMD_float rs(radius_squared);
	SIMD_float mask = p2 < rs;

	if (SIMD_float::all_false(mask)) return mask;
	
	t = t - SIMD_float::sqrt(rs - p2);

	mask = mask & (t > Ray::EPSILON);
	mask = mask & (t < max_distance);
	
	return mask;
}
