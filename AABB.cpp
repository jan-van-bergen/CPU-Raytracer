#include "AABB.h"

// Intersection code based on: https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525

void AABB::trace(const Ray & ray, RayHit & ray_hit) const {
	SIMD_Vector3 aabb_min(min);
	SIMD_Vector3 aabb_max(max);

	SIMD_Vector3 inv_direction = SIMD_Vector3::rcp(ray.direction);
	SIMD_Vector3 t0 = (aabb_min - ray.origin) * inv_direction;
	SIMD_Vector3 t1 = (aabb_max - ray.origin) * inv_direction;
	
	SIMD_Vector3 t_s = SIMD_Vector3::min(t0, t1);
	SIMD_Vector3 t_b  = SIMD_Vector3::max(t0, t1);
	
	SIMD_float t_min = SIMD_float::max(SIMD_float::max(SIMD_float(Ray::EPSILON), t_s.x), SIMD_float::max(t_s.y, t_s.z));
	SIMD_float t_max = SIMD_float::min(SIMD_float::min(ray_hit.distance,         t_b.x), SIMD_float::min(t_b.y, t_b.z));

	SIMD_float mask = t_min < t_max;
	int int_mask = SIMD_float::mask(mask);
	if (int_mask == 0x0) return;

	ray_hit.hit      = ray_hit.hit | mask;
	ray_hit.distance = SIMD_float::blend(ray_hit.distance, t_min, mask);

	SIMD_Vector3 zero(0.0f);

	// NOTE: These values are rubbish! This method should only be used for debuggin purposes
	ray_hit.point  = SIMD_Vector3::blend(ray_hit.point,  zero, mask);
	ray_hit.normal = SIMD_Vector3::blend(ray_hit.normal, zero, mask);
	
	// NOTE: These values are rubbish! This method should only be used for debuggin purposes
	ray_hit.u = SIMD_float::blend(ray_hit.u, SIMD_float(0.0f), mask);
	ray_hit.v = SIMD_float::blend(ray_hit.v, SIMD_float(0.0f), mask);

	for (int i = 0; i < SIMD_LANE_SIZE; i++) {
		if (int_mask & (1 << i)) {
			ray_hit.material[i] = &Material::default_material;
		}
	}
}

SIMD_float AABB::intersect(const Ray & ray, SIMD_float max_distance) const {
	SIMD_Vector3 aabb_min(min);
	SIMD_Vector3 aabb_max(max);

	SIMD_Vector3 inv_direction = SIMD_Vector3::rcp(ray.direction);
	SIMD_Vector3 t0 = (aabb_min - ray.origin) * inv_direction;
	SIMD_Vector3 t1 = (aabb_max - ray.origin) * inv_direction;
	
	SIMD_Vector3 t_s = SIMD_Vector3::min(t0, t1);
	SIMD_Vector3 t_b  = SIMD_Vector3::max(t0, t1);
	
	SIMD_float t_min = SIMD_float::max(SIMD_float::max(SIMD_float(Ray::EPSILON), t_s.x), SIMD_float::max(t_s.y, t_s.z));
	SIMD_float t_max = SIMD_float::min(SIMD_float::min(max_distance,             t_b.x), SIMD_float::min(t_b.y, t_b.z));

	return t_min < t_max;
}
