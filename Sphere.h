#pragma once
#include "Primitive.h"

#include "Ray.h"
#include "RayHit.h"

struct Sphere : Primitive {
private:
	Vector3 world_position;
	float   radius_squared;

public:
	inline void init(float radius) {
		radius_squared = radius * radius;
	}

	void update();

	void trace    (const Ray & ray, RayHit & ray_hit) const;
	bool intersect(const Ray & ray, float max_distance) const;
};
