#include "Sphere.h"

#include "Util.h"

void Sphere::update() {
	transform.calc_world_matrix();
}

void Sphere::trace(const Ray & ray, RayHit & ray_hit) const {
	Vector3 c = position - ray.origin;
	float t = Vector3::dot(c, ray.direction);

	Vector3 Q = c - t * ray.direction;
	float p2 = Vector3::dot(Q, Q);
	
	if (p2 > radius_squared) return;
	
	t -= sqrtf(radius_squared - p2);
	if (t < EPSILON || t > ray_hit.distance) return;

	ray_hit.hit      = true;
	ray_hit.distance = t;

	ray_hit.point  = ray.origin + t * ray.direction;
	ray_hit.normal = Vector3::normalize(ray_hit.point - position);

	ray_hit.u = 0.5f + atan2f(-ray_hit.normal.z, -ray_hit.normal.x) * ONE_OVER_TWO_PI;
	ray_hit.v = 0.5f + asinf (-ray_hit.normal.y)                    * ONE_OVER_PI;

	ray_hit.material = &material;
}

bool Sphere::intersect(const Ray & ray, float max_distance) const {
	Vector3 c = position - ray.origin;
	float t = Vector3::dot(c, ray.direction);

	Vector3 Q = c - t * ray.direction;
	float p2 = Vector3::dot(Q, Q);
	
	if (p2 > radius_squared) return false;
	
	t -= sqrtf(radius_squared - p2);
	return t > EPSILON && t < max_distance;
}
