#pragma once
#include <immintrin.h>

#include "Vector3.h"

// Represents multiple Vector3s
struct SIMD_Vector3 {
	union { __m128 x; float xs[4]; };
	union { __m128 y; float ys[4]; };
	union { __m128 z; float zs[4]; };

	inline          SIMD_Vector3() : x(_mm_set1_ps(0.0f)), y(_mm_set1_ps(0.0f)), z(_mm_set1_ps(0.0f)) { }
	inline explicit SIMD_Vector3(__m128 f) : x(f), y(f), z(f) {}
	inline          SIMD_Vector3(__m128 x, __m128 y, __m128 z) : x(x), y(y), z(z) { }

	inline explicit SIMD_Vector3(const Vector3 & vector) {
		x = _mm_set1_ps(vector.x);
		y = _mm_set1_ps(vector.y);
		z = _mm_set1_ps(vector.z);
	}
	
	inline SIMD_Vector3(const Vector3 & a, const Vector3 & b, const Vector3 & c, const Vector3 & d) {
		x = _mm_set_ps(a.x, b.x, c.x, d.x);
		y = _mm_set_ps(a.y, b.y, c.y, d.y);
		z = _mm_set_ps(a.z, b.z, c.z, d.z);
	}

	inline void set(int index, const Vector3 & vector) {
		xs[index] = vector.x;
		ys[index] = vector.y;
		zs[index] = vector.z;
	}

	inline static __m128 length_squared(const SIMD_Vector3 & vector) {
		return dot(vector, vector);
	}

	inline static __m128 length(const SIMD_Vector3 & vector) {
		return _mm_sqrt_ps(length_squared(vector));
	}

	inline static SIMD_Vector3 normalize(const SIMD_Vector3 & vector) {
		__m128 inv_length = _mm_rcp_ps(length(vector));
		return SIMD_Vector3(_mm_mul_ps(vector.x, inv_length), _mm_mul_ps(vector.y, inv_length), _mm_mul_ps(vector.z, inv_length));
	}

	inline static __m128 dot(const SIMD_Vector3 & left, const SIMD_Vector3 & right) {
		return _mm_add_ps(
			_mm_mul_ps(left.x, right.x), 
			_mm_add_ps(
				_mm_mul_ps(left.y, right.y), 
				_mm_mul_ps(left.z, right.z)
			)
		);
	}

	inline static SIMD_Vector3 cross(const SIMD_Vector3 & left, const SIMD_Vector3 & right) {
		return SIMD_Vector3(
			_mm_sub_ps(_mm_mul_ps(left.y, right.z), _mm_mul_ps(left.z, right.y)),
			_mm_sub_ps(_mm_mul_ps(left.z, right.x), _mm_mul_ps(left.x, right.z)),
			_mm_sub_ps(_mm_mul_ps(left.x, right.y), _mm_mul_ps(left.y, right.x))
		);
	}

	inline static SIMD_Vector3 blend(const SIMD_Vector3 & left, const SIMD_Vector3 & right, __m128 mask) {
		return SIMD_Vector3(
			_mm_blendv_ps(left.x, right.x, mask),
			_mm_blendv_ps(left.y, right.y, mask),
			_mm_blendv_ps(left.z, right.z, mask)
		);
	}

	inline SIMD_Vector3 operator+=(const SIMD_Vector3 & vector) { x = _mm_add_ps(x, vector.x); y = _mm_add_ps(y, vector.y); z = _mm_add_ps(z, vector.z); return *this; }
	inline SIMD_Vector3 operator-=(const SIMD_Vector3 & vector) { x = _mm_sub_ps(x, vector.x); y = _mm_sub_ps(y, vector.y); z = _mm_sub_ps(z, vector.z); return *this; }
	inline SIMD_Vector3 operator*=(const SIMD_Vector3 & vector) { x = _mm_mul_ps(x, vector.x); y = _mm_mul_ps(y, vector.y); z = _mm_mul_ps(z, vector.z); return *this; }
	inline SIMD_Vector3 operator/=(const SIMD_Vector3 & vector) { x = _mm_div_ps(x, vector.x); y = _mm_div_ps(y, vector.y); z = _mm_div_ps(z, vector.z); return *this; }

	inline SIMD_Vector3 operator+=(__m128 scalar) {                                         x = _mm_add_ps(x, scalar);     y = _mm_add_ps(y, scalar);     z = _mm_add_ps(z, scalar);     return *this; }
	inline SIMD_Vector3 operator-=(__m128 scalar) {                                         x = _mm_sub_ps(x, scalar);     y = _mm_sub_ps(y, scalar);     z = _mm_sub_ps(z, scalar);     return *this; }
	inline SIMD_Vector3 operator*=(__m128 scalar) {                                         x = _mm_mul_ps(x, scalar);     y = _mm_mul_ps(y, scalar);     z = _mm_mul_ps(z, scalar);     return *this; }
	inline SIMD_Vector3 operator/=(__m128 scalar) { __m128 inv_scalar = _mm_rcp_ps(scalar); x = _mm_mul_ps(x, inv_scalar); y = _mm_mul_ps(y, inv_scalar); z = _mm_mul_ps(z, inv_scalar); return *this; }
};

inline SIMD_Vector3 operator-(const SIMD_Vector3 & vector) { 
	__m128 zero = _mm_set1_ps(0.0f);
	return SIMD_Vector3(_mm_sub_ps(zero, vector.x), _mm_sub_ps(zero, vector.y), _mm_sub_ps(zero, vector.z)); 
}

inline SIMD_Vector3 operator+(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(_mm_add_ps(left.x, right.x), _mm_add_ps(left.y, right.y), _mm_add_ps(left.z, right.z)); }
inline SIMD_Vector3 operator-(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(_mm_sub_ps(left.x, right.x), _mm_sub_ps(left.y, right.y), _mm_sub_ps(left.z, right.z)); }
inline SIMD_Vector3 operator*(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(_mm_mul_ps(left.x, right.x), _mm_mul_ps(left.y, right.y), _mm_mul_ps(left.z, right.z)); }
inline SIMD_Vector3 operator/(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(_mm_div_ps(left.x, right.x), _mm_div_ps(left.y, right.y), _mm_div_ps(left.z, right.z)); }

inline SIMD_Vector3 operator+(const SIMD_Vector3 & vector, __m128 scalar) {                                         return SIMD_Vector3(_mm_add_ps(vector.x, scalar),     _mm_add_ps(vector.y, scalar),     _mm_add_ps(vector.z, scalar)); }
inline SIMD_Vector3 operator-(const SIMD_Vector3 & vector, __m128 scalar) {                                         return SIMD_Vector3(_mm_sub_ps(vector.x, scalar),     _mm_sub_ps(vector.y, scalar),     _mm_sub_ps(vector.z, scalar)); }
inline SIMD_Vector3 operator*(const SIMD_Vector3 & vector, __m128 scalar) {                                         return SIMD_Vector3(_mm_mul_ps(vector.x, scalar),     _mm_mul_ps(vector.y, scalar),     _mm_mul_ps(vector.z, scalar)); }
inline SIMD_Vector3 operator/(const SIMD_Vector3 & vector, __m128 scalar) { __m128 inv_scalar = _mm_rcp_ps(scalar); return SIMD_Vector3(_mm_mul_ps(vector.x, inv_scalar), _mm_mul_ps(vector.y, inv_scalar), _mm_mul_ps(vector.z, inv_scalar)); }

inline SIMD_Vector3 operator+(__m128 scalar, const SIMD_Vector3 & vector) {                                         return SIMD_Vector3(_mm_add_ps(vector.x, scalar),     _mm_add_ps(vector.y, scalar),     _mm_add_ps(vector.z, scalar)); }
inline SIMD_Vector3 operator-(__m128 scalar, const SIMD_Vector3 & vector) {                                         return SIMD_Vector3(_mm_sub_ps(vector.x, scalar),     _mm_sub_ps(vector.y, scalar),     _mm_sub_ps(vector.z, scalar)); }
inline SIMD_Vector3 operator*(__m128 scalar, const SIMD_Vector3 & vector) {                                         return SIMD_Vector3(_mm_mul_ps(vector.x, scalar),     _mm_mul_ps(vector.y, scalar),     _mm_mul_ps(vector.z, scalar)); }
inline SIMD_Vector3 operator/(__m128 scalar, const SIMD_Vector3 & vector) { __m128 inv_scalar = _mm_rcp_ps(scalar); return SIMD_Vector3(_mm_mul_ps(vector.x, inv_scalar), _mm_mul_ps(vector.y, inv_scalar), _mm_mul_ps(vector.z, inv_scalar)); }

inline int operator==(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return _mm_movemask_ps(_mm_cmpeq_ps (left.x, right.x)) & _mm_movemask_ps(_mm_cmpeq_ps (left.y, right.y)) & _mm_movemask_ps(_mm_cmpeq_ps (left.z, right.z)); }
inline int operator!=(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return _mm_movemask_ps(_mm_cmpneq_ps(left.x, right.x)) | _mm_movemask_ps(_mm_cmpneq_ps(left.y, right.y)) | _mm_movemask_ps(_mm_cmpneq_ps(left.z, right.z)); }
