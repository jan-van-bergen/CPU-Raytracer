#pragma once
#include "Triangle.h"

#include "Util.h"
#include "SIMD.h"

#include "Vector3.h"

// Various math util functions
namespace Math {
	// Clamps the value between a smallest and largest allowed value
	template<typename T>
	inline T clamp(T value, T min, T max) {
		if (value < min) return min;
		if (value > max) return max;

		return value;
	}

	// Interpolate between a,b,c given barycentric coordinates u,v
	template<typename T, typename Real>
	inline T barycentric(const T & a, const T & b, const T & c, Real u, Real v) {
        return a + (u * (b - a)) + (v * (c - a));
    }

	// Calculates the smallest enclosing AABB of a Triangle between the two planes that define a Bin
	inline AABB triangle_binned_aabb(const Triangle & triangle, int dimension, float plane_min, float plane_max) {		
        Vector3 vertices[3] = { 
			triangle.position0,
			triangle.position1, 
			triangle.position2 
		};

		// Sort the vertices along the current dimension using unrolled Bubble Sort
		if (vertices[0][dimension] > vertices[1][dimension]) Util::swap(vertices[0], vertices[1]);
		if (vertices[1][dimension] > vertices[2][dimension]) Util::swap(vertices[1], vertices[2]);
		if (vertices[0][dimension] > vertices[1][dimension]) Util::swap(vertices[0], vertices[1]);

        float vertex_min = vertices[0][dimension];
        float vertex_max = vertices[2][dimension];
        
		// If all vertices lie on one side of either plane the AABB is empty
        if (vertex_min >= plane_max || vertex_max <= plane_min) return AABB::create_empty();
		// If all verticies lie between the two planes, the AABB is just the Triangle's entire AABB
        if (vertex_min >= plane_min && vertex_max <= plane_max) return triangle.aabb;
        
        Vector3 intersections[4];
        int     intersection_count = 0;

        for (int i = 0; i < 3; i++) {
            const Vector3 & vertex_i = vertices[i];

            for (int j = i + 1; j < 3; j++) {
                const Vector3 & vertex_j = vertices[j];

                float delta_ba = vertex_j[dimension] - vertex_i[dimension];

				// Check against min plane
				float delta_min_i = plane_min - vertex_i[dimension];
                float delta_min_j = plane_min - vertex_j[dimension];

                if (delta_min_i > 0.0f && delta_min_j <= 0.0f) { 
					// Lerp to obtain exact intersection point
					float t = delta_min_i / delta_ba;
                    intersections[intersection_count++] = (1.0f - t) * vertex_i + t * vertex_j;
				}

				// Check against max plane
				float delta_max_i = plane_max - vertex_i[dimension];
                float delta_max_j = plane_max - vertex_j[dimension];

                if (delta_max_i > 0.0f && delta_max_j <= 0.0f) { 
					// Lerp to obtain exact intersection point
					float t = delta_max_i / delta_ba;
                    intersections[intersection_count++] = (1.0f - t) * vertex_i + t * vertex_j;
				}
            }
        }

		// All intersection points should be included in the AABB
        AABB result = AABB::from_points(intersections, intersection_count);

		// If the middle vertex lies between the two planes it should be included in the AABB
        if (vertices[1][dimension] >= plane_min && vertices[1][dimension] < plane_max) {
            result.expand(vertices[1]);
		}

		// In case we have only two intersections with either plane it must be the case that
		// either the leftmost or the rightmost vertex lies between the two planes
        if (intersection_count == 2) {
            result.expand(vertex_max < plane_max ? vertices[2] : vertices[0]);
		}

		result.fix_if_needed();
        return result;
	}
	
	// Reflects the vector in the normal
	// The sign of the normal is irrelevant, but it should be normalized
	inline SIMD_Vector3 reflect(const SIMD_Vector3 & vector, const SIMD_Vector3 & normal) {
		return vector - (SIMD_float(2.0f) * SIMD_Vector3::dot(vector, normal)) * normal;
	}

	// Refracts the vector in the normal, according to Snell's Law
	// The normal should be oriented such that it makes the smallest angle possible with the vector
	inline SIMD_Vector3 refract(const SIMD_Vector3 & vector, const SIMD_Vector3 & normal, SIMD_float eta, SIMD_float cos_theta, SIMD_float k) {
		return SIMD_Vector3::normalize(eta * vector + ((eta * cos_theta) - SIMD_float::sqrt(k)) * normal);
	}
	
	// Checks if n is a power of two
	inline constexpr bool is_power_of_two(int n) {
		if (n == 0) return false;

		return (n & (n - 1)) == 0;
	}
	
	// Computes positive modulo of given value
	inline unsigned mod(int value, int modulus) {
		int result = value % modulus;
		if (result < 0) {
			result += modulus;
		}

		return result;
	}

	// Calculates N-th power by repeated squaring. This only works when N is a power of 2
	template<int N> inline float pow2(float value);

	template<>      inline float pow2<0>(float value) { return 1.0f; }
	template<>      inline float pow2<1>(float value) { return value; }
	template<int N> inline float pow2   (float value) { static_assert(is_power_of_two(N)); float sqrt = pow2<N / 2>(value); return sqrt * sqrt; }

	template<int N> inline double pow2(double value);

	template<>      inline double pow2<0>(double value) { return 1.0; }
	template<>      inline double pow2<1>(double value) { return value; }
	template<int N> inline double pow2   (double value) { static_assert(is_power_of_two(N)); double sqrt = pow2<N / 2>(value); return sqrt * sqrt; }

	template<int N> inline SIMD_float pow2(SIMD_float value);

	template<>      inline SIMD_float pow2<0>(SIMD_float value) { return SIMD_float(1.0f); }
	template<>      inline SIMD_float pow2<1>(SIMD_float value) { return value; }
	template<int N> inline SIMD_float pow2   (SIMD_float value) { static_assert(is_power_of_two(N)); SIMD_float sqrt = pow2<N / 2>(value); return sqrt * sqrt; }
}
