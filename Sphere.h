#pragma once
#include "Primitive.h"

#include "Ray.h"
#include "RayHit.h"

struct Sphere : Primitive {
private:
	Vector3 position;
	float   radius_squared;

public:
	inline void init (const Vector3 & sphere_position, float sphere_radius) {
		position       = sphere_position;
		radius_squared = sphere_radius * sphere_radius;
	}

	void update();

	void trace(const Ray & ray, RayHit & ray_hit) const;
	bool intersect(const Ray & ray, float max_distance) const;
};
