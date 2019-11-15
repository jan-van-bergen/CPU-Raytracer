#pragma once
#include "Primitive.h"

#include "Ray.h"
#include "RayHit.h"

struct Plane : Primitive {
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

	void trace(const Ray & ray, RayHit & ray_hit) const;
	bool intersect(const Ray & ray, float max_distance) const;
};
