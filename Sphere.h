#pragma once
#include "Ray.h"
#include "Primitive.h"

struct Sphere : Primitive {
	Vector3 position;
	float   radius_squared;

	inline Sphere(const Vector3 & position, float radius) : position(position), radius_squared(radius * radius) { }

	void trace(const Ray & ray, RayHit & ray_hit) const;
	bool intersect(const Ray & ray, float max_distance) const;
};
