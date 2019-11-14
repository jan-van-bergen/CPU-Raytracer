#include "Plane.h"

void Plane::trace(const Ray & ray, RayHit & ray_hit) const {
	float t = -(Vector3::dot(normal, ray.origin) + distance) / (Vector3::dot(normal, ray.direction));
    if (t < EPSILON || t > ray_hit.distance) return;
	
	ray_hit.hit = true;
	ray_hit.distance = t;
}

bool Plane::intersect(const Ray & ray) const {
	float t = -(Vector3::dot(normal, ray.origin) + distance) / (Vector3::dot(normal, ray.direction));

	return t > EPSILON;
}
