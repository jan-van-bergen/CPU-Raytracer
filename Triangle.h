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

	Vector2 tex_coord0;
	Vector2 tex_coord1;
	Vector2 tex_coord2;

	Vector3 normal0;
	Vector3 normal1;
	Vector3 normal2;

	const Material * material;

	inline void update() { }

	void       trace    (const Ray & ray, RayHit & ray_hit) const;
	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const;

	inline void expand(AABB & aabb) const {
		aabb.min = Vector3::min(Vector3::min(aabb.min, position0), Vector3::min(position1, position2));
		aabb.max = Vector3::max(Vector3::max(aabb.max, position0), Vector3::max(position1, position2));
	}

	inline Vector3 get_position() const {
		return (position0 + position1 + position2) / 3.0f;
	}
};
