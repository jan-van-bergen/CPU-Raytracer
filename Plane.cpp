#include "Plane.h"

void Plane::trace(const Ray & ray, RayHit & ray_hit) const {
	float t = -(Vector3::dot(normal, ray.origin) + distance) / (Vector3::dot(normal, ray.direction));
    if (t < EPSILON || t > ray_hit.distance) return;
	
	ray_hit.hit      = true;
	ray_hit.distance = t;

	ray_hit.point  = ray.origin + t * ray.direction;
	ray_hit.normal = normal;

	float u = Vector3::dot(ray_hit.point, u_axis);
	float v = Vector3::dot(ray_hit.point, v_axis);
}

bool Plane::intersect(const Ray & ray) const {
	float t = -(Vector3::dot(normal, ray.origin) + distance) / (Vector3::dot(normal, ray.direction));

	return t > EPSILON;
}
