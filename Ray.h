#pragma once
#include "SIMDUtil.h"

//#define EPSILON 0.005f

struct Ray {
	inline static const __m128 EPSILON = _mm_set1_ps(0.005f);

	SIMD_Vector3 origin;
	SIMD_Vector3 direction;
};
