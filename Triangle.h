#pragma once
#include "Vector2.h"
#include "Vector3.h"

#include "Ray.h"
#include "RayHit.h"

#include "AABB.h"

struct Triangle {
	Vector3 position_0;
	Vector3 position_1;
	Vector3 position_2;

	AABB aabb;

	inline void calc_aabb() {
		Vector3 vertices[3] = { position_0, position_1, position_2 };

		aabb = AABB::from_points(vertices, 3);
		aabb.fix_if_needed();
	}
	
	inline Vector3 get_position() const {
		return (position_0 + position_1 + position_2) * (1.0f / 3.0f);
	}
};
