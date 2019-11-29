#pragma once
#include "SIMDUtil.h"
#include "Material.h"

struct RayHit {
	__m128 hit;
	__m128 distance;

	SIMD_Vector3 point;  // Coordinates of the hit in World Space
	SIMD_Vector3 normal; // Normal      of the hit in World Space

	const Material * material[4] = { nullptr };
	__m128 u, v;

	inline RayHit() {
		hit      = _mm_set1_ps(0.0f);
		distance = _mm_set1_ps(INFINITY);
	}
};
