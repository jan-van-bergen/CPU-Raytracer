#include "AABB.h"

// Ray-AABB intersection code based on: https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525

SIMD_float AABB::intersect(const Ray & ray, SIMD_float max_distance) const {
	SIMD_Vector3 aabb_min(min);
	SIMD_Vector3 aabb_max(max);

	SIMD_Vector3 inv_direction = SIMD_Vector3::rcp(ray.direction);
	SIMD_Vector3 t0 = (aabb_min - ray.origin) * inv_direction;
	SIMD_Vector3 t1 = (aabb_max - ray.origin) * inv_direction;
	
	SIMD_Vector3 t_min = SIMD_Vector3::min(t0, t1);
	SIMD_Vector3 t_max = SIMD_Vector3::max(t0, t1);
	
	SIMD_float t_near = SIMD_float::max(SIMD_float::max(SIMD_float(Ray::EPSILON), t_min.x), SIMD_float::max(t_min.y, t_min.z));
	SIMD_float t_far  = SIMD_float::min(SIMD_float::min(max_distance,             t_max.x), SIMD_float::min(t_max.y, t_max.z));

	return t_near < t_far;
}
