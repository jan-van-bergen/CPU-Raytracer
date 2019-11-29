#pragma once
#include "SIMD.h"

//#define EPSILON 0.005f

struct Ray {
	inline static const SIMD_float EPSILON = SIMD_float(0.005f);

	SIMD_Vector3 origin;
	SIMD_Vector3 direction;
};
