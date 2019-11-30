#include "Sphere.h"

#include "Util.h"

void Sphere::update() {
	transform.calc_world_matrix();
}

void Sphere::trace(const Ray & ray, RayHit & ray_hit) const {
	SIMD_Vector3 center(transform.position);

	SIMD_Vector3 c = center - ray.origin;
	SIMD_float   t = SIMD_Vector3::dot(c, ray.direction);

	SIMD_Vector3 Q  = c - t * ray.direction;
	SIMD_float   p2 = SIMD_Vector3::dot(Q, Q);
	
	SIMD_float rs(radius_squared);
	SIMD_float mask = p2 < rs;

	if (SIMD_float::all_false(mask)) return;
	
	t = t - SIMD_float::sqrt(rs - p2);
	mask = mask & (t > Ray::EPSILON);
	mask = mask & (t < ray_hit.distance);
	
	int int_mask = SIMD_float::mask(mask);
	if (int_mask == 0x0) return;

	ray_hit.hit      = ray_hit.hit | mask;
	ray_hit.distance = SIMD_float::blend(ray_hit.distance, t, mask);

	ray_hit.point  = SIMD_Vector3::blend(ray_hit.point,  ray.origin + t * ray.direction,                  mask);
	ray_hit.normal = SIMD_Vector3::blend(ray_hit.normal, SIMD_Vector3::normalize(ray_hit.point - center), mask);

	// Obtain u,v by converting the normal direction to spherical coordinates
	ray_hit.u = SIMD_float::blend(ray_hit.u, SIMD_float(0.5f), mask);// + atan2f(-ray_hit.normal.z, -ray_hit.normal.x) * ONE_OVER_TWO_PI;
	ray_hit.v = SIMD_float::blend(ray_hit.v, SIMD_float(0.5f), mask);// + asinf (-ray_hit.normal.y)                    * ONE_OVER_PI;

	for (int i = 0; i < SIMD_LANE_SIZE; i++) {
		if (int_mask & (1 << i)) {
			ray_hit.material[i] = &material;
		}
	}
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
