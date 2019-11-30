#pragma once
#include "Primitive.h"

#include "Ray.h"
#include "RayHit.h"

struct Plane : Primitive {
private:
	Vector3 world_normal;
	float   world_distance;
	
    Vector3 u_axis;
    Vector3 v_axis;

public:
	void update();

	void       trace    (const Ray & ray, RayHit & ray_hit) const;
	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const;
};
