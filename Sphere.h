#pragma once
#include "Primitive.h"

#include "Ray.h"
#include "RayHit.h"

struct Sphere : Primitive {
private:
	float radius_squared;

public:
	inline void init(float radius) {
		radius_squared = radius * radius;
	}

	void update();

	void trace    (const Ray & ray, RayHit & ray_hit) const;
	bool intersect(const Ray & ray, __m128 max_distance) const;
};
