#pragma once
#include <immintrin.h>

#include "Vector3.h"

// Represents 4 floats
struct SIMD_float4 {
	__m128 data;

	inline SIMD_float4() { /* leave uninitialized */ }

	inline explicit SIMD_float4(__m128 data) : data(data) { }

	inline explicit SIMD_float4(float f) : data(_mm_set1_ps(f)) { }
	inline explicit SIMD_float4(float a, float b, float c, float d) : data(_mm_set_ps(a, b, c, d)) { }

	inline static SIMD_float4 load(const float * memory) {
		return SIMD_float4(_mm_load_ps(memory));
	}

	inline static void store(float * memory, SIMD_float4 floats) {
		_mm_store_ps(memory, floats.data);
	}

	inline static SIMD_float4 blend(SIMD_float4 case_false, SIMD_float4 case_true, SIMD_float4 mask) {
		return SIMD_float4(_mm_blendv_ps(case_false.data, case_true.data, mask.data));
	}

	inline static SIMD_float4 rcp(SIMD_float4 floats) {
		return SIMD_float4(_mm_rcp_ps(floats.data));
	}

	inline static SIMD_float4 sqrt(SIMD_float4 floats) {
		return SIMD_float4(_mm_sqrt_ps(floats.data));
	}
	
	inline static SIMD_float4 madd(SIMD_float4 a, SIMD_float4 b, SIMD_float4 c) {
		return SIMD_float4(_mm_fmadd_ps(a.data, b.data, c.data));
	}

	static SIMD_float4 sin(SIMD_float4 floats);
	static SIMD_float4 cos(SIMD_float4 floats);
	static SIMD_float4 exp(SIMD_float4 floats);

	inline static bool all_false(SIMD_float4 floats) { return _mm_movemask_ps(floats.data) == 0x0; }
	inline static bool all_true (SIMD_float4 floats) { return _mm_movemask_ps(floats.data) == 0xf; }

	inline static int mask(SIMD_float4 floats) { return _mm_movemask_ps(floats.data); }
};

inline SIMD_float4 operator-(SIMD_float4 floats) { 
	return SIMD_float4(_mm_sub_ps(_mm_set1_ps(0.0f), floats.data)); 
}

inline SIMD_float4 operator+(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_add_ps(left.data, right.data)); }
inline SIMD_float4 operator-(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_sub_ps(left.data, right.data)); }
inline SIMD_float4 operator*(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_mul_ps(left.data, right.data)); }
inline SIMD_float4 operator/(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_div_ps(left.data, right.data)); }

inline SIMD_float4 operator|(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_or_ps (left.data, right.data)); }
inline SIMD_float4 operator&(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_and_ps(left.data, right.data)); }

inline SIMD_float4 operator> (SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmpgt_ps(left.data, right.data)); }
inline SIMD_float4 operator>=(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmpge_ps(left.data, right.data)); }
inline SIMD_float4 operator< (SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmplt_ps(left.data, right.data)); }
inline SIMD_float4 operator<=(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmple_ps(left.data, right.data)); }

inline SIMD_float4 operator==(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmpeq_ps (left.data, right.data)); }
inline SIMD_float4 operator!=(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmpneq_ps(left.data, right.data)); }

// Represents 8 floats
struct SIMD_float8 {
	__m256 data;

	inline SIMD_float8() { /* leave uninitialized */ }

	inline explicit SIMD_float8(__m256 data) : data(data) { }

	inline explicit SIMD_float8(float f) : data(_mm256_set1_ps(f)) { }
	inline explicit SIMD_float8(float a, float b, float c, float d, float e, float f, float g, float h) : data(_mm256_set_ps(a, b, c, d, e, f, g, h)) { }

	inline static SIMD_float8 load(const float * memory) {
		return SIMD_float8(_mm256_load_ps(memory));
	}

	inline static void store(float * memory, SIMD_float8 floats) {
		_mm256_store_ps(memory, floats.data);
	}

	inline static SIMD_float8 blend(SIMD_float8 case_false, SIMD_float8 case_true, SIMD_float8 mask) {
		return SIMD_float8(_mm256_blendv_ps(case_false.data, case_true.data, mask.data));
	}

	inline static SIMD_float8 rcp(SIMD_float8 floats) {
		return SIMD_float8(_mm256_rcp_ps(floats.data));
	}

	inline static SIMD_float8 sqrt(SIMD_float8 floats) {
		return SIMD_float8(_mm256_sqrt_ps(floats.data));
	}

	inline static SIMD_float8 madd(SIMD_float8 a, SIMD_float8 b, SIMD_float8 c) {
		return SIMD_float8(_mm256_fmadd_ps(a.data, b.data, c.data));
	}

	static SIMD_float8 sin(SIMD_float8 floats);
	static SIMD_float8 cos(SIMD_float8 floats);
	static SIMD_float8 exp(SIMD_float8 floats);

	inline static bool all_false(SIMD_float8 floats) { return _mm256_movemask_ps(floats.data) == 0x0; }
	inline static bool all_true (SIMD_float8 floats) { return _mm256_movemask_ps(floats.data) == 0xff; }

	inline static int mask(SIMD_float8 floats) { return _mm256_movemask_ps(floats.data); }
};

inline SIMD_float8 operator-(SIMD_float8 floats) { 
	return SIMD_float8(_mm256_sub_ps(_mm256_set1_ps(0.0f), floats.data)); 
}

inline SIMD_float8 operator+(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_add_ps(left.data, right.data)); }
inline SIMD_float8 operator-(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_sub_ps(left.data, right.data)); }
inline SIMD_float8 operator*(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_mul_ps(left.data, right.data)); }
inline SIMD_float8 operator/(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_div_ps(left.data, right.data)); }

inline SIMD_float8 operator|(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_or_ps (left.data, right.data)); }
inline SIMD_float8 operator&(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_and_ps(left.data, right.data)); }

inline SIMD_float8 operator> (SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_GT_OQ)); }
inline SIMD_float8 operator>=(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_GE_OQ)); }
inline SIMD_float8 operator< (SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_LT_OQ)); }
inline SIMD_float8 operator<=(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_LE_OQ)); }

inline SIMD_float8 operator==(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_EQ_OQ)); }
inline SIMD_float8 operator!=(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_NEQ_OQ)); }

#define SIMD_LANE_SIZE 4

#if SIMD_LANE_SIZE == 4
typedef SIMD_float4 SIMD_float;
#elif SIMD_LANE_SIZE == 8
typedef SIMD_float8 SIMD_float;
#else
static_assert(false, "Unsupported Lane Size!");
#endif

// Represents SIMD_LANE_SIZE Vector3s
struct SIMD_Vector3 {
	SIMD_float x;
	SIMD_float y;
	SIMD_float z;

	inline          SIMD_Vector3() : x(0.0f), y(0.0f), z(0.0f) { }
	inline explicit SIMD_Vector3(SIMD_float f) : x(f), y(f), z(f) {}
	inline          SIMD_Vector3(SIMD_float x, SIMD_float y, SIMD_float z) : x(x), y(y), z(z) { }

	inline explicit SIMD_Vector3(const Vector3 & vector) {
		x = SIMD_float(vector.x);
		y = SIMD_float(vector.y);
		z = SIMD_float(vector.z);
	}
	
#if SIMD_LANE_SIZE == 4
	inline SIMD_Vector3(const Vector3 & a, const Vector3 & b, const Vector3 & c, const Vector3 & d) {
		x = SIMD_float(a.x, b.x, c.x, d.x);
		y = SIMD_float(a.y, b.y, c.y, d.y);
		z = SIMD_float(a.z, b.z, c.z, d.z);
	}
#elif SIMD_LANE_SIZE == 8
	inline SIMD_Vector3(const Vector3 & a, const Vector3 & b, const Vector3 & c, const Vector3 & d, const Vector3 & e, const Vector3 & f, const Vector3 & g, const Vector3 & h) {
		x = SIMD_float(a.x, b.x, c.x, d.x, e.x, f.x, g.x, h.x);
		y = SIMD_float(a.y, b.y, c.y, d.y, e.y, f.y, g.y, h.y);
		z = SIMD_float(a.z, b.z, c.z, d.z, e.z, f.z, g.z, h.z);
	}
#endif

	inline static SIMD_float length_squared(const SIMD_Vector3 & vector) {
		return dot(vector, vector);
	}

	inline static SIMD_float length(const SIMD_Vector3 & vector) {
		return SIMD_float::sqrt(length_squared(vector));
	}

	inline static SIMD_Vector3 normalize(const SIMD_Vector3 & vector) {
		SIMD_float inv_length = SIMD_float::rcp(length(vector));
		return SIMD_Vector3(
			vector.x * inv_length, 
			vector.y * inv_length, 
			vector.z * inv_length
		);
	}

	inline static SIMD_float dot(const SIMD_Vector3 & left, const SIMD_Vector3 & right) {
		return left.x * right.x + left.y * right.y + left.z * right.z;
	}

	inline static SIMD_Vector3 cross(const SIMD_Vector3 & left, const SIMD_Vector3 & right) {
		return SIMD_Vector3(
			left.y * right.z - left.z * right.y,
			left.z * right.x - left.x * right.z,
			left.x * right.y - left.y * right.x
		);
	}

	inline static SIMD_Vector3 blend(const SIMD_Vector3 & left, const SIMD_Vector3 & right, SIMD_float mask) {
		return SIMD_Vector3(
			SIMD_float::blend(left.x, right.x, mask),
			SIMD_float::blend(left.y, right.y, mask),
			SIMD_float::blend(left.z, right.z, mask)
		);
	}

	inline SIMD_Vector3 operator+=(const SIMD_Vector3 & vector) { x = x + vector.x; y = y + vector.y; z = z + vector.z; return *this; }
	inline SIMD_Vector3 operator-=(const SIMD_Vector3 & vector) { x = x - vector.x; y = y - vector.y; z = z - vector.z; return *this; }
	inline SIMD_Vector3 operator*=(const SIMD_Vector3 & vector) { x = x * vector.x; y = y * vector.y; z = z * vector.z; return *this; }
	inline SIMD_Vector3 operator/=(const SIMD_Vector3 & vector) { x = x / vector.x; y = y / vector.y; z = z / vector.z; return *this; }

	inline SIMD_Vector3 operator+=(SIMD_float f) {                                        x = x + f;      y = y + f;      z = z + f;      return *this; }
	inline SIMD_Vector3 operator-=(SIMD_float f) {                                        x = x - f;      y = y - f;      z = z - f;      return *this; }
	inline SIMD_Vector3 operator*=(SIMD_float f) {                                        x = x * f;      y = y * f;      z = z * f;      return *this; }
	inline SIMD_Vector3 operator/=(SIMD_float f) { SIMD_float inv_f = SIMD_float::rcp(f); x = x * inv_f;  y = y * inv_f;  z = z * inv_f;  return *this; }
};

inline SIMD_Vector3 operator-(const SIMD_Vector3 & vector) { 
	SIMD_float zero(0.0f);
	return SIMD_Vector3(zero - vector.x, zero - vector.y, zero - vector.z); 
}

inline SIMD_Vector3 operator+(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x + right.x, left.y + right.y, left.z + right.z); }
inline SIMD_Vector3 operator-(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x - right.x, left.y - right.y, left.z - right.z); }
inline SIMD_Vector3 operator*(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x * right.x, left.y * right.y, left.z * right.z); }
inline SIMD_Vector3 operator/(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x / right.x, left.y / right.y, left.z / right.z); }

inline SIMD_Vector3 operator+(const SIMD_Vector3 & vector, SIMD_float f) {                                        return SIMD_Vector3(vector.x + f,      vector.y + f,      vector.z + f); }
inline SIMD_Vector3 operator-(const SIMD_Vector3 & vector, SIMD_float f) {                                        return SIMD_Vector3(vector.x - f,      vector.y - f,      vector.z - f); }
inline SIMD_Vector3 operator*(const SIMD_Vector3 & vector, SIMD_float f) {                                        return SIMD_Vector3(vector.x * f,      vector.y * f,      vector.z * f); }
inline SIMD_Vector3 operator/(const SIMD_Vector3 & vector, SIMD_float f) { SIMD_float inv_f = SIMD_float::rcp(f); return SIMD_Vector3(vector.x * inv_f,  vector.y * inv_f,  vector.z * inv_f); }

inline SIMD_Vector3 operator+(SIMD_float f, const SIMD_Vector3 & vector) {                                        return SIMD_Vector3(vector.x + f,      vector.y + f,      vector.z + f); }
inline SIMD_Vector3 operator-(SIMD_float f, const SIMD_Vector3 & vector) {                                        return SIMD_Vector3(vector.x - f,      vector.y - f,      vector.z - f); }
inline SIMD_Vector3 operator*(SIMD_float f, const SIMD_Vector3 & vector) {                                        return SIMD_Vector3(vector.x * f,      vector.y * f,      vector.z * f); }
inline SIMD_Vector3 operator/(SIMD_float f, const SIMD_Vector3 & vector) { SIMD_float inv_f = SIMD_float::rcp(f); return SIMD_Vector3(vector.x * inv_f,  vector.y * inv_f,  vector.z * inv_f); }

inline SIMD_float operator==(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return (left.x == right.x) & (left.y == right.y) & (left.z == right.z); }
inline SIMD_float operator!=(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return (left.x != right.x) | (left.y != right.y) | (left.z != right.z); }
