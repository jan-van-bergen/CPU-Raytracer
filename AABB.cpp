#include "AABB.h"

// Ray-AABB intersection code based on: https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525

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
