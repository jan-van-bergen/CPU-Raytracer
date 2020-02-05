#pragma once
#include "Vector2.h"
#include "Vector3.h"

#include "Ray.h"
#include "RayHit.h"

#include "AABB.h"

struct Triangle {
	Vector3 position0;
	Vector3 position1;
	Vector3 position2;

	AABB aabb;

	inline void calc_aabb() {
		Vector3 vertices[3] = { position0, position1, position2 };

		aabb = AABB::from_points(vertices, 3);
		aabb.fix_if_needed();
	}
	
	inline Vector3 get_position() const {
		return (position0 + position1 + position2) / 3.0f;
	}
};
