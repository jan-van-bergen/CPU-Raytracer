#pragma once
#include <cassert>

#include "Vector3.h"

namespace Test {
	inline bool approx_equal(__m128 a, __m128 b) {
		const __m128 epsilon = _mm_set1_ps(0.01f);

		__m128 diff = _mm_sub_ps(a, b);

		return 
			_mm_movemask_ps(_mm_cmpgt_ps(diff, _mm_sub_ps(_mm_set1_ps(0.0f), epsilon))) == 0xf &&
			_mm_movemask_ps(_mm_cmplt_ps(diff,                               epsilon))  == 0xf;
	}

	// Check if Snell's Law holds for the given input and output directions
	inline bool test_refraction(__m128 n_1, __m128 n_2, const SIMD_Vector3 & direction_in, const SIMD_Vector3 & normal, const SIMD_Vector3 & direction_out, __m128 mask) {
		const __m128 zero = _mm_set1_ps(0.0f);
		const __m128 one  = _mm_set1_ps(1.0f);

		// Vectors are assumed to be normalized
		assert(approx_equal(_mm_blendv_ps(one, SIMD_Vector3::length(direction_in),  mask), one));
		assert(approx_equal(_mm_blendv_ps(one, SIMD_Vector3::length(direction_out), mask), one));
		assert(approx_equal(_mm_blendv_ps(one, SIMD_Vector3::length(normal),        mask), one));

		assert(_mm_movemask_ps(_mm_cmpgt_ps(_mm_blendv_ps(one, SIMD_Vector3::dot(-direction_in,   normal), mask), zero)) == 0xf); // Opposite of incoming direction and normal should point in the same direction
		assert(_mm_movemask_ps(_mm_cmpgt_ps(_mm_blendv_ps(one, SIMD_Vector3::dot( direction_out, -normal), mask), zero)) == 0xf); // Outgoing direction and opposite of normal should point in the same direction

		float dot_1[4]; _mm_store_ps(dot_1, SIMD_Vector3::dot(-direction_in,   normal));
		float dot_2[4]; _mm_store_ps(dot_2, SIMD_Vector3::dot( direction_out, -normal));

		__m128 sin_theta_1 = _mm_set_ps(sinf(acosf(dot_1[3])), sinf(acosf(dot_1[2])), sinf(acosf(dot_1[1])), sinf(acosf(dot_1[0])));
		__m128 sin_theta_2 = _mm_set_ps(sinf(acosf(dot_2[3])), sinf(acosf(dot_2[2])), sinf(acosf(dot_2[1])), sinf(acosf(dot_2[0])));

		__m128 lhs = _mm_blendv_ps(zero, _mm_mul_ps(n_1, sin_theta_1), mask);
		__m128 rhs = _mm_blendv_ps(zero, _mm_mul_ps(n_2, sin_theta_2), mask);

		return approx_equal(lhs, rhs);
	}
}
