#pragma once
#include "Vector3.h"

namespace Math3d {
	template<typename T>
	inline T clamp(T value, T min, T max) {
		if (value < min) return min;
		if (value > max) return max;

		return value;
	}
	
	// Interpolate between a,b,c given barycentric coordinates u,v
	template<typename T>
	inline T barycentric(const T & a, const T & b, const T & c, float u, float v) {
        return a + (u * (b - a)) + (v * (c - a));
    }

	// Calculates N-th power by repeated squaring. This only works when N is a power of 2
	template<int N> inline float pow2(float value);

	template<>      inline float pow2<0>(float value) { return 1.0f; }
	template<>      inline float pow2<1>(float value) { return value; }
	template<int N> inline float pow2   (float value) { float sqrt = pow2<N / 2>(value); return sqrt * sqrt; }

	// Calculates N-th power by repeated squaring. This only works when N is a power of 2
	template<int N> inline double pow2(double value);

	template<>      inline double pow2<0>(double value) { return 1.0; }
	template<>      inline double pow2<1>(double value) { return value; }
	template<int N> inline double pow2   (double value) { double sqrt = pow2<N / 2>(value); return sqrt * sqrt; }
}
