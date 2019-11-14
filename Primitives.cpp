#include "Primitives.h"

RayHit Sphere::trace(const Ray & ray, float max_t) const {
	RayHit hit;

	Vector3 c = position - ray.origin;
	float t = Vector3::dot(c, ray.direction);

	Vector3 Q = c - t * ray.direction;
	float p2 = Vector3::dot(Q, Q);
	
	if (p2 > radius_squared) return hit;
	
	t -= sqrt(radius_squared - p2);
	if (t > EPSILON && t < max_t) {
		hit.hit = true;
		hit.distance = t;
	}

	return hit;
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

RayHit Plane::trace(const Ray & ray, float max_t) const {
	RayHit hit;

	float t = -(Vector3::dot(normal, ray.origin) + distance) / (Vector3::dot(normal, ray.direction));
    if (t < EPSILON || t > max_t) return hit;

	hit.hit = true;
    return hit;
}

bool Plane::intersect(const Ray & ray) const {
	float t = -(Vector3::dot(normal, ray.origin) + distance) / (Vector3::dot(normal, ray.direction));

	return t > EPSILON;
}
