#include "Plane.h"

void Plane::update() {
	transform.calc_world_matrix();

	world_normal   =  Matrix4::transform_direction(transform.world_matrix, Vector3(0.0f, 1.0f, 0.0f));
	world_distance = -Vector3::dot(world_normal, transform.position);

	u_axis = Matrix4::transform_direction(transform.world_matrix, Vector3(1.0f, 0.0f, 0.0f));
	v_axis = Vector3::cross(u_axis, world_normal);
}

void Plane::trace(const Ray & ray, RayHit & ray_hit) const {
	float t = -(Vector3::dot(world_normal, ray.origin) + world_distance) / (Vector3::dot(world_normal, ray.direction));
    if (t < EPSILON || t > ray_hit.distance) return;
	
	ray_hit.hit      = true;
	ray_hit.distance = t;

	ray_hit.point  = ray.origin + t * ray.direction;
	ray_hit.normal = world_normal;

	ray_hit.u = Vector3::dot(ray_hit.point, u_axis);
	ray_hit.v = Vector3::dot(ray_hit.point, v_axis);

	ray_hit.material = &material;
}

bool Plane::intersect(const Ray & ray, float max_distance) const {
	float t = -(Vector3::dot(world_normal, ray.origin) + world_distance) / (Vector3::dot(world_normal, ray.direction));

	return t > EPSILON && t < max_distance;
}
