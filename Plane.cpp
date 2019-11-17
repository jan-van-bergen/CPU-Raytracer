#include "Plane.h"

void Plane::update() {
	transform.calc_world_matrix();
}

void Plane::trace(const Ray & ray, RayHit & ray_hit) const {
	float t = -(Vector3::dot(normal, ray.origin) + distance) / (Vector3::dot(normal, ray.direction));
    if (t < EPSILON || t > ray_hit.distance) return;
	
	ray_hit.hit      = true;
	ray_hit.distance = t;

	ray_hit.point  = ray.origin + t * ray.direction;
	ray_hit.normal = normal;

	ray_hit.u = Vector3::dot(ray_hit.point, u_axis);
	ray_hit.v = Vector3::dot(ray_hit.point, v_axis);

	ray_hit.material = &material;
}

bool Plane::intersect(const Ray & ray, float max_distance) const {
	float t = -(Vector3::dot(normal, ray.origin) + distance) / (Vector3::dot(normal, ray.direction));

	return t > EPSILON && t < max_distance;
}
