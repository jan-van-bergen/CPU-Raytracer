#pragma once
#include <immintrin.h>

#include "Vector3.h"

// Represents 4 floats
struct SIMD_float {
	__m128 data;

	inline SIMD_float() { /* leave uninitialized */ }

	inline explicit SIMD_float(__m128 data) : data(data) { }

	inline explicit SIMD_float(float f) : data(_mm_set1_ps(f)) { }
	inline explicit SIMD_float(float a, float b, float c, float d) : data(_mm_set_ps(a, b, c, d)) { }

	inline static SIMD_float load(const float * memory) {
		return SIMD_float(_mm_load_ps(memory));
	}

	inline static void store(float * memory, SIMD_float floats) {
		_mm_store_ps(memory, floats.data);
	}

	inline static SIMD_float blend(SIMD_float case_false, SIMD_float case_true, SIMD_float mask) {
		return SIMD_float(_mm_blendv_ps(case_false.data, case_true.data, mask.data));
	}

	inline static SIMD_float rcp(SIMD_float floats) {
		return SIMD_float(_mm_rcp_ps(floats.data));
	}

	inline static SIMD_float sqrt(SIMD_float floats) {
		return SIMD_float(_mm_sqrt_ps(floats.data));
	}

	// @TODO: improve
	inline static SIMD_float exp(SIMD_float floats) {
		float temp[4]; _mm_store_ps(temp, floats.data);
		temp[3] = expf(temp[3]);
		temp[2] = expf(temp[2]);
		temp[1] = expf(temp[1]);
		temp[0] = expf(temp[0]);

		return SIMD_float(_mm_load_ps(temp));
	}

	inline static bool all_false(SIMD_float floats) { return _mm_movemask_ps(floats.data) == 0x0; }
	inline static bool all_true (SIMD_float floats) { return _mm_movemask_ps(floats.data) == 0xf; }

	inline static int mask(SIMD_float floats) { return _mm_movemask_ps(floats.data); }
};

inline SIMD_float operator-(SIMD_float floats) { 
	return SIMD_float(_mm_sub_ps(_mm_set1_ps(0.0f), floats.data)); 
}

inline SIMD_float operator+(SIMD_float left, SIMD_float right) { return SIMD_float(_mm_add_ps(left.data, right.data)); }
inline SIMD_float operator-(SIMD_float left, SIMD_float right) { return SIMD_float(_mm_sub_ps(left.data, right.data)); }
inline SIMD_float operator*(SIMD_float left, SIMD_float right) { return SIMD_float(_mm_mul_ps(left.data, right.data)); }
inline SIMD_float operator/(SIMD_float left, SIMD_float right) { return SIMD_float(_mm_div_ps(left.data, right.data)); }

inline SIMD_float operator|(SIMD_float left, SIMD_float right) { return SIMD_float(_mm_or_ps (left.data, right.data)); }
inline SIMD_float operator&(SIMD_float left, SIMD_float right) { return SIMD_float(_mm_and_ps(left.data, right.data)); }

inline SIMD_float operator> (SIMD_float left, SIMD_float right) { return SIMD_float(_mm_cmpgt_ps(left.data, right.data)); }
inline SIMD_float operator>=(SIMD_float left, SIMD_float right) { return SIMD_float(_mm_cmpge_ps(left.data, right.data)); }
inline SIMD_float operator< (SIMD_float left, SIMD_float right) { return SIMD_float(_mm_cmplt_ps(left.data, right.data)); }
inline SIMD_float operator<=(SIMD_float left, SIMD_float right) { return SIMD_float(_mm_cmple_ps(left.data, right.data)); }

inline int operator==(const SIMD_float & left, const SIMD_float & right) { return _mm_movemask_ps(_mm_cmpeq_ps (left.data, right.data)); }
inline int operator!=(const SIMD_float & left, const SIMD_float & right) { return _mm_movemask_ps(_mm_cmpneq_ps(left.data, right.data)); }

// Represents 4 Vector3s
struct SIMD_Vector3 {
	__m128 x;
	__m128 y;
	__m128 z;

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

	inline static SIMD_float length_squared(const SIMD_Vector3 & vector) {
		return dot(vector, vector);
	}

	inline static SIMD_float length(const SIMD_Vector3 & vector) {
		return SIMD_float(_mm_sqrt_ps(length_squared(vector).data));
	}

	inline static SIMD_Vector3 normalize(const SIMD_Vector3 & vector) {
		__m128 inv_length = _mm_rcp_ps(length(vector).data);
		return SIMD_Vector3(_mm_mul_ps(vector.x, inv_length), _mm_mul_ps(vector.y, inv_length), _mm_mul_ps(vector.z, inv_length));
	}

	inline static SIMD_float dot(const SIMD_Vector3 & left, const SIMD_Vector3 & right) {
		return SIMD_float(_mm_add_ps(
			_mm_mul_ps(left.x, right.x), 
			_mm_add_ps(
				_mm_mul_ps(left.y, right.y), 
				_mm_mul_ps(left.z, right.z)
			)
		));
	}

	inline static SIMD_Vector3 cross(const SIMD_Vector3 & left, const SIMD_Vector3 & right) {
		return SIMD_Vector3(
			_mm_sub_ps(_mm_mul_ps(left.y, right.z), _mm_mul_ps(left.z, right.y)),
			_mm_sub_ps(_mm_mul_ps(left.z, right.x), _mm_mul_ps(left.x, right.z)),
			_mm_sub_ps(_mm_mul_ps(left.x, right.y), _mm_mul_ps(left.y, right.x))
		);
	}

	inline static SIMD_Vector3 blend(const SIMD_Vector3 & left, const SIMD_Vector3 & right, SIMD_float mask) {
		return SIMD_Vector3(
			_mm_blendv_ps(left.x, right.x, mask.data),
			_mm_blendv_ps(left.y, right.y, mask.data),
			_mm_blendv_ps(left.z, right.z, mask.data)
		);
	}

	inline SIMD_Vector3 operator+=(const SIMD_Vector3 & vector) { x = _mm_add_ps(x, vector.x); y = _mm_add_ps(y, vector.y); z = _mm_add_ps(z, vector.z); return *this; }
	inline SIMD_Vector3 operator-=(const SIMD_Vector3 & vector) { x = _mm_sub_ps(x, vector.x); y = _mm_sub_ps(y, vector.y); z = _mm_sub_ps(z, vector.z); return *this; }
	inline SIMD_Vector3 operator*=(const SIMD_Vector3 & vector) { x = _mm_mul_ps(x, vector.x); y = _mm_mul_ps(y, vector.y); z = _mm_mul_ps(z, vector.z); return *this; }
	inline SIMD_Vector3 operator/=(const SIMD_Vector3 & vector) { x = _mm_div_ps(x, vector.x); y = _mm_div_ps(y, vector.y); z = _mm_div_ps(z, vector.z); return *this; }

	inline SIMD_Vector3 operator+=(SIMD_float f) {                                    x = _mm_add_ps(x, f.data); y = _mm_add_ps(y, f.data); z = _mm_add_ps(z, f.data); return *this; }
	inline SIMD_Vector3 operator-=(SIMD_float f) {                                    x = _mm_sub_ps(x, f.data); y = _mm_sub_ps(y, f.data); z = _mm_sub_ps(z, f.data); return *this; }
	inline SIMD_Vector3 operator*=(SIMD_float f) {                                    x = _mm_mul_ps(x, f.data); y = _mm_mul_ps(y, f.data); z = _mm_mul_ps(z, f.data); return *this; }
	inline SIMD_Vector3 operator/=(SIMD_float f) { __m128 inv_f = _mm_rcp_ps(f.data); x = _mm_mul_ps(x, inv_f);  y = _mm_mul_ps(y, inv_f);  z = _mm_mul_ps(z, inv_f);  return *this; }
};

inline SIMD_Vector3 operator-(const SIMD_Vector3 & vector) { 
	__m128 zero = _mm_set1_ps(0.0f);
	return SIMD_Vector3(_mm_sub_ps(zero, vector.x), _mm_sub_ps(zero, vector.y), _mm_sub_ps(zero, vector.z)); 
}

inline SIMD_Vector3 operator+(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(_mm_add_ps(left.x, right.x), _mm_add_ps(left.y, right.y), _mm_add_ps(left.z, right.z)); }
inline SIMD_Vector3 operator-(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(_mm_sub_ps(left.x, right.x), _mm_sub_ps(left.y, right.y), _mm_sub_ps(left.z, right.z)); }
inline SIMD_Vector3 operator*(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(_mm_mul_ps(left.x, right.x), _mm_mul_ps(left.y, right.y), _mm_mul_ps(left.z, right.z)); }
inline SIMD_Vector3 operator/(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(_mm_div_ps(left.x, right.x), _mm_div_ps(left.y, right.y), _mm_div_ps(left.z, right.z)); }

inline SIMD_Vector3 operator+(const SIMD_Vector3 & vector, SIMD_float f) {                                    return SIMD_Vector3(_mm_add_ps(vector.x, f.data), _mm_add_ps(vector.y, f.data), _mm_add_ps(vector.z, f.data)); }
inline SIMD_Vector3 operator-(const SIMD_Vector3 & vector, SIMD_float f) {                                    return SIMD_Vector3(_mm_sub_ps(vector.x, f.data), _mm_sub_ps(vector.y, f.data), _mm_sub_ps(vector.z, f.data)); }
inline SIMD_Vector3 operator*(const SIMD_Vector3 & vector, SIMD_float f) {                                    return SIMD_Vector3(_mm_mul_ps(vector.x, f.data), _mm_mul_ps(vector.y, f.data), _mm_mul_ps(vector.z, f.data)); }
inline SIMD_Vector3 operator/(const SIMD_Vector3 & vector, SIMD_float f) { __m128 inv_f = _mm_rcp_ps(f.data); return SIMD_Vector3(_mm_mul_ps(vector.x, inv_f),  _mm_mul_ps(vector.y, inv_f),  _mm_mul_ps(vector.z, inv_f)); }

inline SIMD_Vector3 operator+(SIMD_float f, const SIMD_Vector3 & vector) {                                    return SIMD_Vector3(_mm_add_ps(vector.x, f.data), _mm_add_ps(vector.y, f.data), _mm_add_ps(vector.z, f.data)); }
inline SIMD_Vector3 operator-(SIMD_float f, const SIMD_Vector3 & vector) {                                    return SIMD_Vector3(_mm_sub_ps(vector.x, f.data), _mm_sub_ps(vector.y, f.data), _mm_sub_ps(vector.z, f.data)); }
inline SIMD_Vector3 operator*(SIMD_float f, const SIMD_Vector3 & vector) {                                    return SIMD_Vector3(_mm_mul_ps(vector.x, f.data), _mm_mul_ps(vector.y, f.data), _mm_mul_ps(vector.z, f.data)); }
inline SIMD_Vector3 operator/(SIMD_float f, const SIMD_Vector3 & vector) { __m128 inv_f = _mm_rcp_ps(f.data); return SIMD_Vector3(_mm_mul_ps(vector.x, inv_f),  _mm_mul_ps(vector.y, inv_f),  _mm_mul_ps(vector.z, inv_f)); }

inline int operator==(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return _mm_movemask_ps(_mm_cmpeq_ps (left.x, right.x)) & _mm_movemask_ps(_mm_cmpeq_ps (left.y, right.y)) & _mm_movemask_ps(_mm_cmpeq_ps (left.z, right.z)); }
inline int operator!=(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return _mm_movemask_ps(_mm_cmpneq_ps(left.x, right.x)) | _mm_movemask_ps(_mm_cmpneq_ps(left.y, right.y)) | _mm_movemask_ps(_mm_cmpneq_ps(left.z, right.z)); }
