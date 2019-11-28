#pragma once
#include <cassert>

#include "Vector3.h"

namespace Test {
	inline bool approx_equal(float a, float b) {
		return fabsf(a - b) < 0.01f;
	}

	// Check if Snell's Law holds for the given input and output directions
	inline bool test_refraction(float n_1, float n_2, const Vector3 & direction_in, const Vector3 & normal, const Vector3 & direction_out) {
		// Vectors are assumed to be normalized
		assert(approx_equal(Vector3::length(direction_in),  1.0f));
		assert(approx_equal(Vector3::length(direction_out), 1.0f));
		assert(approx_equal(Vector3::length(normal),        1.0f));

		assert(Vector3::dot(-direction_in,   normal) > 0.0f); // Opposite of incoming direction and normal should point in the same direction
		assert(Vector3::dot( direction_out, -normal) > 0.0f); // Outgoing direction and opposite of normal should point in the same direction

		float theta_1 = acosf(Vector3::dot(-direction_in,   normal));
		float theta_2 = acosf(Vector3::dot( direction_out, -normal));

		float lhs = n_1 * sinf(theta_1);
		float rhs = n_2 * sinf(theta_2);

		return approx_equal(lhs, rhs);
	}
}
