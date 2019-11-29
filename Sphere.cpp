#include "Sphere.h"

#include "Util.h"

void Sphere::update() {
	transform.calc_world_matrix();
}

void Sphere::trace(const Ray & ray, RayHit & ray_hit) const {
	SIMD_Vector3 center(transform.position);

	SIMD_Vector3 c = center - ray.origin;
	__m128 t = SIMD_Vector3::dot(c, ray.direction);

	SIMD_Vector3 Q = c - t * ray.direction;
	__m128 p2 = SIMD_Vector3::dot(Q, Q);
	
	__m128 rs = _mm_set1_ps(radius_squared);
	__m128 mask = _mm_cmplt_ps(p2, rs);

	if (_mm_movemask_ps(mask) == 0x0) return;
	
	t = _mm_sub_ps(t, _mm_sqrt_ps(_mm_sub_ps(rs, p2)));
	mask = _mm_and_ps(mask, _mm_cmpgt_ps(t, Ray::EPSILON));
	mask = _mm_and_ps(mask, _mm_cmplt_ps(t, ray_hit.distance));
	
	int int_mask = _mm_movemask_ps(mask);
	if (int_mask == 0x0) return;

	ray_hit.hit      = _mm_or_ps(ray_hit.hit, mask);
	ray_hit.distance = _mm_blendv_ps(ray_hit.distance, t, mask);

	ray_hit.point  = SIMD_Vector3::blend(ray_hit.point,  ray.origin + t * ray.direction,                  mask);
	ray_hit.normal = SIMD_Vector3::blend(ray_hit.normal, SIMD_Vector3::normalize(ray_hit.point - center), mask);

	// Obtain u,v by converting the normal direction to spherical coordinates
	ray_hit.u = _mm_blendv_ps(ray_hit.u, _mm_set1_ps(0.5f), mask);// + atan2f(-ray_hit.normal.z, -ray_hit.normal.x) * ONE_OVER_TWO_PI;
	ray_hit.v = _mm_blendv_ps(ray_hit.v, _mm_set1_ps(0.5f), mask);// + asinf (-ray_hit.normal.y)                    * ONE_OVER_PI;

	if (int_mask & 8) ray_hit.material[3] = &material;
	if (int_mask & 4) ray_hit.material[2] = &material;
	if (int_mask & 2) ray_hit.material[1] = &material;
	if (int_mask & 1) ray_hit.material[0] = &material;
}

bool Sphere::intersect(const Ray & ray, __m128 max_distance) const {
	return false;
	/*Vector3 c = transform.position - ray.origin;
	float t = Vector3::dot(c, ray.direction);

	Vector3 Q = c - t * ray.direction;
	float p2 = Vector3::dot(Q, Q);
	
	if (p2 > radius_squared) return false;
	
	t -= sqrtf(radius_squared - p2);
	return t > EPSILON && t < max_distance;*/
}
