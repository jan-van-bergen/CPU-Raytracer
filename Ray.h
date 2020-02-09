#pragma once
#include "SIMD_Vector3.h"

struct Ray {
	inline static const SIMD_float EPSILON = SIMD_float(0.005f);

	SIMD_Vector3 origin;
	SIMD_Vector3 direction;

#if RAY_DIFFERENTIALS_ENABLED
	// Ray Differentials with respect to screen space coordinates x, y
	SIMD_Vector3 dO_dx;
	SIMD_Vector3 dO_dy;
	SIMD_Vector3 dD_dx;
	SIMD_Vector3 dD_dy;
#endif
};
