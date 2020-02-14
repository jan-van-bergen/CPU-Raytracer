#pragma once
#include <immintrin.h>

#include <string>
#include <cassert>

#include "Util.h"

// Represents 1 float
struct SIMD_float1 {
	union { float data; int data_mask; };

	inline SIMD_float1() { /* leave uninitialized */ }

	inline explicit SIMD_float1(float f) : data(f) { }
	inline explicit SIMD_float1(int i) : data_mask(i) { }

	inline static FORCEINLINE SIMD_float1 load(const float * memory) {
		SIMD_float1 result;
		memcpy(&result.data, memory, sizeof(float));
		return result;
	}

	inline static FORCEINLINE void store(float * memory, SIMD_float1 floats) {
		memcpy(memory, &floats.data, sizeof(float));
	}

	inline static FORCEINLINE SIMD_float1 blend(SIMD_float1 case_false, SIMD_float1 case_true, SIMD_float1 mask) {
		return SIMD_float1(mask.data_mask ? case_true.data : case_false.data);
	}
	
	inline static FORCEINLINE SIMD_float1 min(SIMD_float1 a, SIMD_float1 b) { return SIMD_float1(a.data < b.data ? a.data : b.data); }
	inline static FORCEINLINE SIMD_float1 max(SIMD_float1 a, SIMD_float1 b) { return SIMD_float1(a.data > b.data ? a.data : b.data); }
	
	inline static FORCEINLINE SIMD_float1 floor(SIMD_float1 floats) { return SIMD_float1(floorf(floats.data)); }
	inline static FORCEINLINE SIMD_float1 ceil (SIMD_float1 floats) { return SIMD_float1(ceilf (floats.data)); }

	static FORCEINLINE SIMD_float1 mod(const SIMD_float1 & v, const SIMD_float1 & m);
	
	static FORCEINLINE SIMD_float1 clamp(const SIMD_float1 & val, const SIMD_float1 & min, const SIMD_float1 & max);

	inline static FORCEINLINE SIMD_float1 rcp(SIMD_float1 floats) { return SIMD_float1(1.0f / floats.data); }

	inline static FORCEINLINE SIMD_float1 sqrt    (SIMD_float1 floats) { return SIMD_float1(       sqrtf(floats.data)); }
	inline static FORCEINLINE SIMD_float1 inv_sqrt(SIMD_float1 floats) { return SIMD_float1(1.0f / sqrtf(floats.data)); }
	
	inline static FORCEINLINE SIMD_float1 madd(SIMD_float1 a, SIMD_float1 b, SIMD_float1 c) { return SIMD_float1(a.data * b.data + c.data); }
	inline static FORCEINLINE SIMD_float1 msub(SIMD_float1 a, SIMD_float1 b, SIMD_float1 c) { return SIMD_float1(a.data * b.data - c.data); }
	
	inline static FORCEINLINE SIMD_float1 sin(SIMD_float1 floats) { return SIMD_float1(sinf(floats.data)); }
	inline static FORCEINLINE SIMD_float1 cos(SIMD_float1 floats) { return SIMD_float1(cosf(floats.data)); }
	inline static FORCEINLINE SIMD_float1 tan(SIMD_float1 floats) { return SIMD_float1(tanf(floats.data)); }
	
	inline static FORCEINLINE SIMD_float1 asin (SIMD_float1 floats)           { return SIMD_float1(asinf(floats.data)); }
	inline static FORCEINLINE SIMD_float1 acos (SIMD_float1 floats)           { return SIMD_float1(acosf(floats.data)); }
	inline static FORCEINLINE SIMD_float1 atan (SIMD_float1 floats)           { return SIMD_float1(atanf(floats.data)); }
	inline static FORCEINLINE SIMD_float1 atan2(SIMD_float1 y, SIMD_float1 x) { return SIMD_float1(atan2f(y.data, x.data)); }

	inline static FORCEINLINE SIMD_float1 exp(SIMD_float1 floats) { return SIMD_float1(expf(floats.data)); }

	inline static FORCEINLINE bool all_false(SIMD_float1 floats) { return floats.data_mask == 0x0; }
	inline static FORCEINLINE bool all_true (SIMD_float1 floats) { return floats.data_mask == 0x1; }
	
	// Computes (not a) and b
	inline static FORCEINLINE SIMD_float1 andnot(SIMD_float1 a, SIMD_float1 b) {
		return SIMD_float1((~a.data_mask) & b.data_mask);
	}

	inline static FORCEINLINE int mask(SIMD_float1 floats) { return floats.data_mask; }

	inline FORCEINLINE       float & operator[](int index)       { assert(index == 0); return data; }
	inline FORCEINLINE const float & operator[](int index) const { assert(index == 0); return data; }
};

inline FORCEINLINE SIMD_float1 operator-(SIMD_float1 floats) { 
	return SIMD_float1(-floats.data); 
}

inline FORCEINLINE SIMD_float1 operator+(SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data + right.data); }
inline FORCEINLINE SIMD_float1 operator-(SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data - right.data); }
inline FORCEINLINE SIMD_float1 operator*(SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data * right.data); }
inline FORCEINLINE SIMD_float1 operator/(SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data / right.data); }

inline FORCEINLINE SIMD_float1 operator|(SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data_mask | right.data_mask); }
inline FORCEINLINE SIMD_float1 operator^(SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data_mask ^ right.data_mask); }
inline FORCEINLINE SIMD_float1 operator&(SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data_mask & right.data_mask); }

inline FORCEINLINE SIMD_float1 operator~(const SIMD_float1 & floats) { return SIMD_float1(~floats.data_mask); }

inline FORCEINLINE SIMD_float1 operator> (SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data >  right.data); }
inline FORCEINLINE SIMD_float1 operator>=(SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data >= right.data); }
inline FORCEINLINE SIMD_float1 operator< (SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data <  right.data); }
inline FORCEINLINE SIMD_float1 operator<=(SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data <= right.data); }

inline FORCEINLINE SIMD_float1 operator==(SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data == right.data); }
inline FORCEINLINE SIMD_float1 operator!=(SIMD_float1 left, SIMD_float1 right) { return SIMD_float1(left.data != right.data); }

// Represents 4 floats
struct SIMD_float4 {
	union { __m128 data; float floats[4]; };

	inline SIMD_float4() { /* leave uninitialized */ }

	inline explicit SIMD_float4(__m128 data) : data(data) { }

	inline explicit SIMD_float4(float f) : data(_mm_set1_ps(f)) { }
	inline explicit SIMD_float4(float a, float b, float c, float d) : data(_mm_set_ps(a, b, c, d)) { }

	inline static FORCEINLINE SIMD_float4 load(const float * memory) {
		return SIMD_float4(_mm_load_ps(memory));
	}

	inline static FORCEINLINE void store(float * memory, const SIMD_float4 & floats) {
		assert(unsigned long long(memory) % alignof(__m128) == 0);

		_mm_store_ps(memory, floats.data);
	}

	inline static FORCEINLINE SIMD_float4 blend(const SIMD_float4 & case_false, const SIMD_float4 & case_true, const SIMD_float4 & mask) {
		return SIMD_float4(_mm_blendv_ps(case_false.data, case_true.data, mask.data));
	}
	
	inline static FORCEINLINE SIMD_float4 min(const SIMD_float4 & a, const SIMD_float4 & b) { return SIMD_float4(_mm_min_ps(a.data, b.data)); }
	inline static FORCEINLINE SIMD_float4 max(const SIMD_float4 & a, const SIMD_float4 & b) { return SIMD_float4(_mm_max_ps(a.data, b.data)); }
	
	inline static FORCEINLINE SIMD_float4 floor(SIMD_float4 floats) { return SIMD_float4(_mm_floor_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float4 ceil (SIMD_float4 floats) { return SIMD_float4(_mm_ceil_ps (floats.data)); }

	static FORCEINLINE SIMD_float4 mod(const SIMD_float4 & v, const SIMD_float4 & m);
	
	static FORCEINLINE SIMD_float4 clamp(const SIMD_float4 & val, const SIMD_float4 & min, const SIMD_float4 & max);

	static FORCEINLINE SIMD_float4 rcp(const SIMD_float4 & floats);

	inline static FORCEINLINE SIMD_float4     sqrt(const SIMD_float4 & floats) { return SIMD_float4(_mm_sqrt_ps   (floats.data)); }
	inline static FORCEINLINE SIMD_float4 inv_sqrt(const SIMD_float4 & floats) { return SIMD_float4(_mm_invsqrt_ps(floats.data)); }
	
	inline static FORCEINLINE SIMD_float4 madd(const SIMD_float4 & a, const SIMD_float4 & b, const SIMD_float4 & c) { return SIMD_float4(_mm_fmadd_ps(a.data, b.data, c.data)); } // Computes a*b + c
	inline static FORCEINLINE SIMD_float4 msub(const SIMD_float4 & a, const SIMD_float4 & b, const SIMD_float4 & c) { return SIMD_float4(_mm_fmsub_ps(a.data, b.data, c.data)); } // Computes a*b - c
	
	inline static FORCEINLINE SIMD_float4 sin(const SIMD_float4 & floats) { return SIMD_float4(_mm_sin_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float4 cos(const SIMD_float4 & floats) { return SIMD_float4(_mm_cos_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float4 tan(const SIMD_float4 & floats) { return SIMD_float4(_mm_tan_ps(floats.data)); }
	
	inline static FORCEINLINE SIMD_float4 asin (const SIMD_float4 & floats)                   { return SIMD_float4(_mm_asin_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float4 acos (const SIMD_float4 & floats)                   { return SIMD_float4(_mm_acos_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float4 atan (const SIMD_float4 & floats)                   { return SIMD_float4(_mm_atan_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float4 atan2(const SIMD_float4 & y, const SIMD_float4 & x) { return SIMD_float4(_mm_atan2_ps(y.data, x.data)); }

	inline static FORCEINLINE SIMD_float4 exp(const SIMD_float4 & floats) { return SIMD_float4(_mm_exp_ps(floats.data)); }

	inline static FORCEINLINE bool all_false(const SIMD_float4 & floats) { return _mm_movemask_ps(floats.data) == 0x0; }
	inline static FORCEINLINE bool all_true (const SIMD_float4 & floats) { return _mm_movemask_ps(floats.data) == 0xf; }
	
	// Computes (not a) and b
	inline static FORCEINLINE SIMD_float4 andnot(const SIMD_float4 & a, const SIMD_float4 & b) {
		return SIMD_float4(_mm_andnot_ps(a.data, b.data));
	}

	inline static FORCEINLINE int mask(const SIMD_float4 & floats) { return _mm_movemask_ps(floats.data); }

	inline FORCEINLINE       float & operator[](int index)       { assert(index >= 0 && index < 4); return floats[index]; }
	inline FORCEINLINE const float & operator[](int index) const { assert(index >= 0 && index < 4); return floats[index]; }
};

inline FORCEINLINE SIMD_float4 operator-(const SIMD_float4 & floats) { 
	return SIMD_float4(_mm_sub_ps(_mm_set1_ps(0.0f), floats.data)); 
}

inline FORCEINLINE SIMD_float4 operator+(const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_add_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator-(const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_sub_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator*(const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_mul_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator/(const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_div_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float4 operator|(const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_or_ps (left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator^(const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_xor_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator&(const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_and_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float4 operator~(const SIMD_float4 & floats) { return SIMD_float4(_mm_xor_ps(floats.data, _mm_cmpeq_ps(floats.data, floats.data))); }

inline FORCEINLINE SIMD_float4 operator> (const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_cmpgt_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator>=(const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_cmpge_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator< (const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_cmplt_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator<=(const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_cmple_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float4 operator==(const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_cmpeq_ps (left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator!=(const SIMD_float4 & left, const SIMD_float4 & right) { return SIMD_float4(_mm_cmpneq_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float4 SIMD_float4::rcp(const SIMD_float4 & floats) { return SIMD_float4(1.0f) / floats; }

// Represents 8 floats
struct SIMD_float8 {
	union { __m256 data; float floats[8]; };

	inline SIMD_float8() { /* leave uninitialized */ }

	inline explicit SIMD_float8(__m256 data) : data(data) { }

	inline explicit SIMD_float8(float f) : data(_mm256_set1_ps(f)) { }
	inline explicit SIMD_float8(float a, float b, float c, float d, float e, float f, float g, float h) : data(_mm256_set_ps(a, b, c, d, e, f, g, h)) { }

	inline static FORCEINLINE SIMD_float8 load(const float * memory) {
		return SIMD_float8(_mm256_load_ps(memory));
	}

	inline static FORCEINLINE void store(float * memory, const SIMD_float8 & floats) {
		assert(unsigned long long(memory) % alignof(__m256) == 0);

		_mm256_store_ps(memory, floats.data);
	}

	inline static FORCEINLINE SIMD_float8 blend(const SIMD_float8 & case_false, const SIMD_float8 & case_true, const SIMD_float8 & mask) {
		return SIMD_float8(_mm256_blendv_ps(case_false.data, case_true.data, mask.data));
	}

	inline static FORCEINLINE SIMD_float8 min(const SIMD_float8 & a, const SIMD_float8 & b) { return SIMD_float8(_mm256_min_ps(a.data, b.data)); }
	inline static FORCEINLINE SIMD_float8 max(const SIMD_float8 & a, const SIMD_float8 & b) { return SIMD_float8(_mm256_max_ps(a.data, b.data)); }
	
	inline static FORCEINLINE SIMD_float8 floor(const SIMD_float8 & floats) { return SIMD_float8(_mm256_floor_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float8 ceil (const SIMD_float8 & floats) { return SIMD_float8(_mm256_ceil_ps (floats.data)); }
	
	static FORCEINLINE SIMD_float8 mod(const SIMD_float8 & v, const SIMD_float8 & m);

	static FORCEINLINE SIMD_float8 clamp(const SIMD_float8 & val, const SIMD_float8 & min, const SIMD_float8 & max);

	static FORCEINLINE SIMD_float8 rcp(const SIMD_float8 & floats);

	inline static FORCEINLINE SIMD_float8     sqrt(const SIMD_float8 & floats) { return SIMD_float8(_mm256_sqrt_ps   (floats.data)); }
	inline static FORCEINLINE SIMD_float8 inv_sqrt(const SIMD_float8 & floats) { return SIMD_float8(_mm256_invsqrt_ps(floats.data)); }

	inline static FORCEINLINE SIMD_float8 madd(const SIMD_float8 & a, const SIMD_float8 & b, const SIMD_float8 & c) { return SIMD_float8(_mm256_fmadd_ps(a.data, b.data, c.data)); } // Computes a*b + c
	inline static FORCEINLINE SIMD_float8 msub(const SIMD_float8 & a, const SIMD_float8 & b, const SIMD_float8 & c) { return SIMD_float8(_mm256_fmsub_ps(a.data, b.data, c.data)); } // Computes a*b - c
	
	inline static FORCEINLINE SIMD_float8 sin(const SIMD_float8 & floats) { return SIMD_float8(_mm256_sin_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float8 cos(const SIMD_float8 & floats) { return SIMD_float8(_mm256_cos_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float8 tan(const SIMD_float8 & floats) { return SIMD_float8(_mm256_tan_ps(floats.data)); }
	
	inline static FORCEINLINE SIMD_float8 asin (const SIMD_float8 & floats)                   { return SIMD_float8(_mm256_asin_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float8 acos (const SIMD_float8 & floats)                   { return SIMD_float8(_mm256_acos_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float8 atan (const SIMD_float8 & floats)                   { return SIMD_float8(_mm256_atan_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float8 atan2(const SIMD_float8 & y, const SIMD_float8 & x) { return SIMD_float8(_mm256_atan2_ps(y.data, x.data)); }

	inline static FORCEINLINE SIMD_float8 exp(const SIMD_float8 & floats) { return SIMD_float8(_mm256_exp_ps(floats.data)); }

	inline static FORCEINLINE bool all_false(const SIMD_float8 & floats) { return _mm256_movemask_ps(floats.data) == 0x0; }
	inline static FORCEINLINE bool all_true (const SIMD_float8 & floats) { return _mm256_movemask_ps(floats.data) == 0xff; }

	// Computes (not a) and b
	inline static FORCEINLINE SIMD_float8 andnot(const SIMD_float8 & a, const SIMD_float8 & b) {
		return SIMD_float8(_mm256_andnot_ps(a.data, b.data));
	}

	inline static FORCEINLINE int mask(const SIMD_float8 & floats) { return _mm256_movemask_ps(floats.data); }
	
	inline FORCEINLINE       float & operator[](int index)       { assert(index >= 0 && index < 8); return floats[index]; }
	inline FORCEINLINE const float & operator[](int index) const { assert(index >= 0 && index < 8); return floats[index]; }
};

inline FORCEINLINE SIMD_float8 operator-(const SIMD_float8 & floats) { 
	return SIMD_float8(_mm256_sub_ps(_mm256_set1_ps(0.0f), floats.data)); 
}

inline FORCEINLINE SIMD_float8 operator+(const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_add_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float8 operator-(const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_sub_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float8 operator*(const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_mul_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float8 operator/(const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_div_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float8 operator|(const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_or_ps (left.data, right.data)); }
inline FORCEINLINE SIMD_float8 operator^(const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_xor_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float8 operator&(const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_and_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float8 operator~(const SIMD_float8 & floats) { return SIMD_float8(_mm256_xor_ps(floats.data, _mm256_cmp_ps(floats.data, floats.data, _CMP_EQ_UQ))); }

inline FORCEINLINE SIMD_float8 operator> (const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_GT_OQ)); }
inline FORCEINLINE SIMD_float8 operator>=(const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_GE_OQ)); }
inline FORCEINLINE SIMD_float8 operator< (const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_LT_OQ)); }
inline FORCEINLINE SIMD_float8 operator<=(const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_LE_OQ)); }

inline FORCEINLINE SIMD_float8 operator==(const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_EQ_OQ)); }
inline FORCEINLINE SIMD_float8 operator!=(const SIMD_float8 & left, const SIMD_float8 & right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_NEQ_OQ)); }

inline FORCEINLINE SIMD_float8 SIMD_float8::rcp(const SIMD_float8 & floats) { return SIMD_float8(1.0f) / floats; }
