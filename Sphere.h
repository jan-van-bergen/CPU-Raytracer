#pragma once
#include "Ray.h"

struct Sphere {
	Vector3 position;
	float   radius_squared;

	inline Sphere(const Vector3 & position, float radius) : 
		position(position), 
		radius_squared(radius * radius) 
	{ }

	RayHit trace(const Ray & ray, float max_t) const;
	bool intersect(const Ray & ray) const;
};
