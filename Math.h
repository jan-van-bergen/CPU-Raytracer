#pragma once
#include "SIMDUtil.h"

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
	
	// Reflects the vector in the normal
	// The sign of the normal is irrelevant, but it should be normalized
	inline SIMD_Vector3 reflect(const SIMD_Vector3 & vector, const SIMD_Vector3 & normal) {
		return vector - _mm_mul_ps(_mm_set1_ps(2.0f), SIMD_Vector3::dot(vector, normal)) * normal;
	}

	// Refracts the vector in the normal, according to Snell's Law
	// The normal should be oriented such that it makes the smallest angle possible with the vector
	inline SIMD_Vector3 refract(const SIMD_Vector3 & vector, const SIMD_Vector3 & normal, __m128 eta, __m128 cos_theta, __m128 k) {
		return SIMD_Vector3::normalize(eta * vector + (_mm_sub_ps(_mm_mul_ps(eta, cos_theta), _mm_sqrt_ps(k))) * normal);
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

	template<int N> inline __m128 pow2(__m128 value);

	template<>      inline __m128 pow2<0>(__m128 value) { return _mm_set1_ps(1.0); }
	template<>      inline __m128 pow2<1>(__m128 value) { return value; }
	template<int N> inline __m128 pow2   (__m128 value) { static_assert(is_power_of_two(N)); __m128 sqrt = pow2<N / 2>(value); return _mm_mul_ps(sqrt, sqrt); }

	// Source: https://stackoverflow.com/a/47025627
	inline __m128 exp(__m128 x) {
		float temp[4]; _mm_store_ps(temp, x);
		temp[3] = expf(temp[3]);
		temp[2] = expf(temp[2]);
		temp[1] = expf(temp[1]);
		temp[0] = expf(temp[0]);
		return _mm_load_ps(temp);
		//__m128 f, p, r;
		//__m128i t, j;
		//const __m128 a = _mm_set1_ps (12102203.0f); /* (1 << 23) / log(2) */
		//const __m128i m = _mm_set1_epi32 (0xff800000); /* mask for integer bits */
		//const __m128 ttm23 = _mm_set1_ps (1.1920929e-7f); /* exp2(-23) */
		//const __m128 c0 = _mm_set1_ps (0.3371894346f);
		//const __m128 c1 = _mm_set1_ps (0.657636276f);
		//const __m128 c2 = _mm_set1_ps (1.00172476f);

		//t = _mm_cvtps_epi32 (_mm_mul_ps (a, x));
		//j = _mm_and_si128 (t, m);            /* j = (int)(floor (x/log(2))) << 23 */
		//t = _mm_sub_epi32 (t, j);
		//f = _mm_mul_ps (ttm23, _mm_cvtepi32_ps (t)); /* f = (x/log(2)) - floor (x/log(2)) */
		//p = c0;                              /* c0 */
		//p = _mm_mul_ps (p, f);               /* c0 * f */
		//p = _mm_add_ps (p, c1);              /* c0 * f + c1 */
		//p = _mm_mul_ps (p, f);               /* (c0 * f + c1) * f */
		//p = _mm_add_ps (p, c2);              /* p = (c0 * f + c1) * f + c2 ~= 2^f */
		//r = _mm_castsi128_ps (_mm_add_epi32 (j, _mm_castps_si128 (p))); /* r = p * 2^i*/
		//return r;
	}
}
