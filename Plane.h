#pragma once
#include "Primitive.h"

#include "Ray.h"
#include "RayHit.h"

struct Plane : Primitive {
private:
	Vector3 normal;
	float   distance;
	
    Vector3 u_axis;
    Vector3 v_axis;

public:
	inline void init(const Vector3 & plane_normal, float plane_distance) {
		normal   = plane_normal;
		distance = plane_distance;
		u_axis = Vector3(normal.y, normal.z, -normal.x);
		v_axis = Vector3::cross(u_axis, normal);
	}

	void update();

	void trace(const Ray & ray, RayHit & ray_hit) const;
	bool intersect(const Ray & ray, float max_distance) const;
};
