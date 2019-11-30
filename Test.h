#pragma once
#include <cassert>

#include "Vector3.h"

namespace Test {
	inline bool approx_equal(SIMD_float a, SIMD_float b) {
		const SIMD_float epsilon(0.01f);

		SIMD_float diff = a - b;
		return SIMD_float::all_true((diff > -epsilon) & (diff < epsilon));
	}

	// Check if Snell's Law holds for the given input and output directions
	inline bool test_refraction(SIMD_float n_1, SIMD_float n_2, const SIMD_Vector3 & direction_in, const SIMD_Vector3 & normal, const SIMD_Vector3 & direction_out, SIMD_float mask) {
		const SIMD_float zero(0.0f);
		const SIMD_float one (1.0f);
		
		// Vectors are assumed to be normalized
		assert(approx_equal(SIMD_float::blend(one, SIMD_Vector3::length(direction_in),  mask), one));
		assert(approx_equal(SIMD_float::blend(one, SIMD_Vector3::length(direction_out), mask), one));
		assert(approx_equal(SIMD_float::blend(one, SIMD_Vector3::length(normal),        mask), one));
		
		assert(SIMD_float::all_true(SIMD_float::blend(one, SIMD_Vector3::dot(-direction_in,   normal), mask) > zero)); // Opposite of incoming direction and normal should point in the same direction
		assert(SIMD_float::all_true(SIMD_float::blend(one, SIMD_Vector3::dot( direction_out, -normal), mask) > zero)); // Outgoing direction and opposite of normal should point in the same direction
		
		SIMD_float dot_1 = SIMD_Vector3::dot(-direction_in,   normal);
		SIMD_float dot_2 = SIMD_Vector3::dot( direction_out, -normal);
		
		SIMD_float theta_1(SIMD_float::acos(dot_1));
		SIMD_float theta_2(SIMD_float::acos(dot_2));

		SIMD_float lhs = SIMD_float::blend(zero, n_1 * SIMD_float::sin(theta_1), mask);
		SIMD_float rhs = SIMD_float::blend(zero, n_2 * SIMD_float::sin(theta_2), mask);
		
		bool temp = approx_equal(lhs, rhs);
		if (!temp) __debugbreak();
		return temp;
	}
}
