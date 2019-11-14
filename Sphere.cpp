#include "Sphere.h"

void Sphere::trace(const Ray & ray, RayHit & ray_hit) const {
	Vector3 c = position - ray.origin;
	float t = Vector3::dot(c, ray.direction);

	Vector3 Q = c - t * ray.direction;
	float p2 = Vector3::dot(Q, Q);
	
	if (p2 > radius_squared) return;
	
	t -= sqrt(radius_squared - p2);
	if (t < EPSILON || t > ray_hit.distance) return;

	ray_hit.hit = true;
	ray_hit.distance = t;
}

bool Sphere::intersect(const Ray & ray) const {
	Vector3 c = position - ray.origin;
	float t = Vector3::dot(c, ray.direction);

	Vector3 Q = c - t * ray.direction;
	float p2 = Vector3::dot(Q, Q);
	
	if (p2 > radius_squared) return false;
	
	t -= sqrt(radius_squared - p2);
	return t > EPSILON;
}
