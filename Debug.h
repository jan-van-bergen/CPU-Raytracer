#pragma once
#include <cassert>

#include "Vector3.h"
#include "Texture.h"

namespace Debug {
	inline const Texture * heat_palette; // Used to visualize the number of steps through a BVH

	template<typename Float>
	inline bool approx_equal(Float a, Float b, Float epsilon = static_cast<Float>(0.01)) {
		return abs(a - b) > epsilon;
	}

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
		
		assert(SIMD_float::all_true(SIMD_float::blend(one, SIMD_Vector3::dot(-direction_in,   normal), mask) >= SIMD_float(-1e8))); // Opposite of incoming direction and normal should point in the same direction
		assert(SIMD_float::all_true(SIMD_float::blend(one, SIMD_Vector3::dot( direction_out, -normal), mask) >= SIMD_float(-1e8))); // Outgoing direction and opposite of normal should point in the same direction
		
		SIMD_float dot_1 = SIMD_Vector3::dot(-direction_in,   normal);
		SIMD_float dot_2 = SIMD_Vector3::dot( direction_out, -normal);
		
		SIMD_float theta_1(SIMD_float::acos(SIMD_float::clamp(dot_1, -one, one)));
		SIMD_float theta_2(SIMD_float::acos(SIMD_float::clamp(dot_2, -one, one)));

		SIMD_float lhs = SIMD_float::blend(zero, n_1 * SIMD_float::sin(theta_1), mask);
		SIMD_float rhs = SIMD_float::blend(zero, n_2 * SIMD_float::sin(theta_2), mask);
		
		return approx_equal(lhs, rhs);
	}
	
	// Writes an Triangle to the given FILE in obj format
	inline void obj_write_triangle(const Triangle & triangle, FILE * file, int & index) {
		fprintf(file, "v %f %f %f\n", triangle.position0.x, triangle.position0.y, triangle.position0.z);
		fprintf(file, "v %f %f %f\n", triangle.position1.x, triangle.position1.y, triangle.position1.z);
		fprintf(file, "v %f %f %f\n", triangle.position2.x, triangle.position2.y, triangle.position2.z);

		fprintf(file, "f %i %i %i\n", index, index + 1, index + 2);

		index += 3;
	}
	
	// Writes an AABB to the given FILE in obj format
	inline void obj_write_aabb(const AABB & aabb, FILE * file, int & index) {
		Vector3 vertices[8] = {
			Vector3(aabb.min.x, aabb.min.y, aabb.min.z),
			Vector3(aabb.min.x, aabb.min.y, aabb.max.z),
			Vector3(aabb.max.x, aabb.min.y, aabb.max.z),
			Vector3(aabb.max.x, aabb.min.y, aabb.min.z),
			Vector3(aabb.min.x, aabb.max.y, aabb.min.z),
			Vector3(aabb.min.x, aabb.max.y, aabb.max.z),
			Vector3(aabb.max.x, aabb.max.y, aabb.max.z),
			Vector3(aabb.max.x, aabb.max.y, aabb.min.z)
		};

		int faces[36] = {
			0, 1, 2, 0, 2, 3,
			0, 1, 5, 0, 5, 4,
			0, 4, 7, 0, 7, 3,
			3, 7, 6, 3, 6, 2,
			2, 6, 5, 2, 5, 1,
			4, 5, 6, 4, 6, 7
		};

		fprintf(file, "o Debug_AABB_%i\n", index);
		for (int v = 0; v < 8; v++) {
			fprintf(file, "v %f %f %f\n", vertices[v].x, vertices[v].y, vertices[v].z);
		}

		for (int f = 0; f < 36; f += 3) {
			fprintf(file, "f %i %i %i\n", 8*index + faces[f], 8*index + faces[f+1], 8*index + faces[f+2]);
		}

		index += 8;
	}
}
