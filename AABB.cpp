#include "AABB.h"

AABB AABB::create_empty() {
	AABB aabb;
	aabb.min = Vector3(+INFINITY);
	aabb.max = Vector3(-INFINITY);

	return aabb;
}

AABB AABB::from_points(const Vector3 * points, int point_count) {
	AABB aabb = create_empty();

	for (int i = 0; i < point_count; i++) {
		aabb.expand(points[i]);
	}

	aabb.fix_if_needed();
	assert(aabb.is_valid());

	return aabb;
}

AABB AABB::overlap(const AABB & b1, const AABB & b2) {
	assert(b1.is_valid() || b1.is_empty());
	assert(b2.is_valid() || b2.is_empty());

	AABB aabb;
	aabb.min = Vector3::max(b1.min, b2.min);
	aabb.max = Vector3::min(b1.max, b2.max);

	if (!aabb.is_valid()) aabb = create_empty();

	return aabb;
}

// Ray-AABB intersection code based on: https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
SIMD_float AABB::intersect(const Ray & ray, const SIMD_Vector3 & inv_direction, SIMD_float max_distance) const {
	SIMD_Vector3 aabb_min(min);
	SIMD_Vector3 aabb_max(max);

	SIMD_Vector3 t0 = (aabb_min - ray.origin) * inv_direction;
	SIMD_Vector3 t1 = (aabb_max - ray.origin) * inv_direction;
	
	SIMD_Vector3 t_min = SIMD_Vector3::min(t0, t1);
	SIMD_Vector3 t_max = SIMD_Vector3::max(t0, t1);
	
	SIMD_float t_near = SIMD_float::max(SIMD_float::max(Ray::EPSILON, t_min.x), SIMD_float::max(t_min.y, t_min.z));
	SIMD_float t_far  = SIMD_float::min(SIMD_float::min(max_distance, t_max.x), SIMD_float::min(t_max.y, t_max.z));

	return t_near < t_far;
}

// Based on: https://zeux.io/2010/10/17/aabb-from-obb-with-component-wise-abs/
AABB AABB::transform(const AABB & aabb, const Matrix4 & transformation) {
	Vector3 center = 0.5f * (aabb.min + aabb.max);
	Vector3 extent = 0.5f * (aabb.max - aabb.min);

	// Construct matrix that contains the absolute value of all cells in the original matrix
	Matrix4 abs_transformation;
	for (int i = 0; i < 16; i++) {
		abs_transformation.cells[i] = abs(transformation.cells[i]);
	}

	Vector3 new_center = Matrix4::transform_position (    transformation, center);
    Vector3 new_extent = Matrix4::transform_direction(abs_transformation, extent);

	AABB result;
	result.min = new_center - new_extent;
	result.max = new_center + new_extent;

	return result;
}
