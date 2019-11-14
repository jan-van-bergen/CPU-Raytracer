#pragma once
#include "Ray.h"

struct Plane {
	Vector3 normal;
	float   distance;
	
    Vector3 u_axis;
    Vector3 v_axis;

	inline Plane(const Vector3 & normal, float distance) :
		normal(normal),
		distance(distance)
	{
		u_axis = Vector3(normal.y, normal.z, -normal.x);
		v_axis = Vector3::cross(u_axis, normal);
	}

	RayHit trace(const Ray & ray, float max_t) const;
	bool intersect(const Ray & ray) const;
};
