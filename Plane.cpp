#include "Plane.h"

RayHit Plane::trace(const Ray & ray, float max_t) const {
	RayHit hit;

	float t = -(Vector3::dot(normal, ray.origin) + distance) / (Vector3::dot(normal, ray.direction));
    if (t < EPSILON || t > max_t) return hit;

	hit.hit = true;
	hit.distance = t;

    return hit;
}

bool Plane::intersect(const Ray & ray) const {
	float t = -(Vector3::dot(normal, ray.origin) + distance) / (Vector3::dot(normal, ray.direction));

	return t > EPSILON;
}
