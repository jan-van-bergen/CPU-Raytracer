#pragma once
#include <string>
#include <cassert>
#include <immintrin.h>

#include "Util.h"
#include "Vector3.h"

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

	static FORCEINLINE SIMD_float1 mod(SIMD_float1 v, SIMD_float1 m);
	
	static FORCEINLINE SIMD_float1 clamp(SIMD_float1 val, SIMD_float1 min, SIMD_float1 max);

	inline static FORCEINLINE SIMD_float1 rcp(SIMD_float1 floats) { return SIMD_float1(1.0f / floats.data); }

	inline static FORCEINLINE SIMD_float1 sqrt    (SIMD_float1 floats) { return SIMD_float1(       sqrtf(floats.data)); }
	inline static FORCEINLINE SIMD_float1 inv_sqrt(SIMD_float1 floats) { return SIMD_float1(1.0f / sqrtf(floats.data)); }
	
	inline static FORCEINLINE SIMD_float1 madd(SIMD_float1 a, SIMD_float1 b, SIMD_float1 c) { return SIMD_float1(a.data * b.data + c.data); }
	inline static FORCEINLINE SIMD_float1 msub(SIMD_float1 a, SIMD_float1 b, SIMD_float1 c) { return SIMD_float1(a.data * b.data - c.data); }
	
	inline static FORCEINLINE SIMD_float1 sin(SIMD_float1 floats) { return SIMD_float1(sinf(floats.data)); }
	inline static FORCEINLINE SIMD_float1 cos(SIMD_float1 floats) { return SIMD_float1(cosf(floats.data)); }
	
	inline static FORCEINLINE SIMD_float1 atan2(SIMD_float1 y, SIMD_float1 x) { return SIMD_float1(atan2f(y.data, x.data)); }
	inline static FORCEINLINE SIMD_float1 acos (SIMD_float1 floats)           { return SIMD_float1(acosf(floats.data)); }

	inline static FORCEINLINE SIMD_float1 exp(SIMD_float1 floats) { return SIMD_float1(expf(floats.data)); }

	inline static FORCEINLINE bool all_false(SIMD_float1 floats) { return floats.data_mask == 0x0; }
	inline static FORCEINLINE bool all_true (SIMD_float1 floats) { return floats.data_mask == 0x1; }
	
	// Computes (not a) and b
	inline static FORCEINLINE SIMD_float1 andnot(SIMD_float1 a, SIMD_float1 b) {
		return SIMD_float1((~a.data_mask) & b.data_mask);
	}

	inline static FORCEINLINE int mask(SIMD_float1 floats) { return floats.data_mask; }

	inline FORCEINLINE       float & operator[](int index)       { return data; }
	inline FORCEINLINE const float & operator[](int index) const { return data; }
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

	inline static FORCEINLINE void store(float * memory, SIMD_float4 floats) {
		assert(unsigned long long(memory) % alignof(__m128) == 0);

		_mm_store_ps(memory, floats.data);
	}

	inline static FORCEINLINE SIMD_float4 blend(SIMD_float4 case_false, SIMD_float4 case_true, SIMD_float4 mask) {
		return SIMD_float4(_mm_blendv_ps(case_false.data, case_true.data, mask.data));
	}
	
	inline static FORCEINLINE SIMD_float4 min(SIMD_float4 a, SIMD_float4 b) { return SIMD_float4(_mm_min_ps(a.data, b.data)); }
	inline static FORCEINLINE SIMD_float4 max(SIMD_float4 a, SIMD_float4 b) { return SIMD_float4(_mm_max_ps(a.data, b.data)); }
	
	inline static FORCEINLINE SIMD_float4 floor(SIMD_float4 floats) { return SIMD_float4(_mm_floor_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float4 ceil (SIMD_float4 floats) { return SIMD_float4(_mm_ceil_ps (floats.data)); }

	static FORCEINLINE SIMD_float4 mod(SIMD_float4 v, SIMD_float4 m);
	
	static FORCEINLINE SIMD_float4 clamp(SIMD_float4 val, SIMD_float4 min, SIMD_float4 max);

	static FORCEINLINE SIMD_float4 rcp(SIMD_float4 floats);

	inline static FORCEINLINE SIMD_float4     sqrt(SIMD_float4 floats) { return SIMD_float4(_mm_sqrt_ps   (floats.data)); }
	inline static FORCEINLINE SIMD_float4 inv_sqrt(SIMD_float4 floats) { return SIMD_float4(_mm_invsqrt_ps(floats.data)); }
	
	inline static FORCEINLINE SIMD_float4 madd(SIMD_float4 a, SIMD_float4 b, SIMD_float4 c) { return SIMD_float4(_mm_fmadd_ps(a.data, b.data, c.data)); } // Computes a*b + c
	inline static FORCEINLINE SIMD_float4 msub(SIMD_float4 a, SIMD_float4 b, SIMD_float4 c) { return SIMD_float4(_mm_fmsub_ps(a.data, b.data, c.data)); } // Computes a*b - c
	
	inline static FORCEINLINE SIMD_float4 sin(SIMD_float4 floats) { return SIMD_float4(_mm_sin_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float4 cos(SIMD_float4 floats) { return SIMD_float4(_mm_cos_ps(floats.data)); }
	
	inline static FORCEINLINE SIMD_float4 atan2(SIMD_float4 y, SIMD_float4 x) { return SIMD_float4(_mm_atan2_ps(y.data, x.data)); }
	inline static FORCEINLINE SIMD_float4 acos (SIMD_float4 floats)           { return SIMD_float4(_mm_acos_ps(floats.data)); }

	inline static FORCEINLINE SIMD_float4 exp(SIMD_float4 floats) { return SIMD_float4(_mm_exp_ps(floats.data)); }

	inline static FORCEINLINE bool all_false(SIMD_float4 floats) { return _mm_movemask_ps(floats.data) == 0x0; }
	inline static FORCEINLINE bool all_true (SIMD_float4 floats) { return _mm_movemask_ps(floats.data) == 0xf; }
	
	// Computes (not a) and b
	inline static FORCEINLINE SIMD_float4 andnot(SIMD_float4 a, SIMD_float4 b) {
		return SIMD_float4(_mm_andnot_ps(a.data, b.data));
	}

	inline static FORCEINLINE int mask(SIMD_float4 floats) { return _mm_movemask_ps(floats.data); }

	inline FORCEINLINE       float & operator[](int index)       { return floats[index]; }
	inline FORCEINLINE const float & operator[](int index) const { return floats[index]; }
};

inline FORCEINLINE SIMD_float4 operator-(SIMD_float4 floats) { 
	return SIMD_float4(_mm_sub_ps(_mm_set1_ps(0.0f), floats.data)); 
}

inline FORCEINLINE SIMD_float4 operator+(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_add_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator-(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_sub_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator*(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_mul_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator/(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_div_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float4 operator|(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_or_ps (left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator^(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_xor_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator&(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_and_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float4 operator> (SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmpgt_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator>=(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmpge_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator< (SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmplt_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator<=(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmple_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float4 operator==(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmpeq_ps (left.data, right.data)); }
inline FORCEINLINE SIMD_float4 operator!=(SIMD_float4 left, SIMD_float4 right) { return SIMD_float4(_mm_cmpneq_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float4 SIMD_float4::rcp(SIMD_float4 floats) { return SIMD_float4(1.0f) / floats; }

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

	inline static FORCEINLINE void store(float * memory, SIMD_float8 floats) {
		assert(unsigned long long(memory) % alignof(__m256) == 0);

		_mm256_store_ps(memory, floats.data);
	}

	inline static FORCEINLINE SIMD_float8 blend(SIMD_float8 case_false, SIMD_float8 case_true, SIMD_float8 mask) {
		return SIMD_float8(_mm256_blendv_ps(case_false.data, case_true.data, mask.data));
	}

	inline static FORCEINLINE SIMD_float8 min(SIMD_float8 a, SIMD_float8 b) { return SIMD_float8(_mm256_min_ps(a.data, b.data)); }
	inline static FORCEINLINE SIMD_float8 max(SIMD_float8 a, SIMD_float8 b) { return SIMD_float8(_mm256_max_ps(a.data, b.data)); }
	
	inline static FORCEINLINE SIMD_float8 floor(SIMD_float8 floats) { return SIMD_float8(_mm256_floor_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float8 ceil (SIMD_float8 floats) { return SIMD_float8(_mm256_ceil_ps (floats.data)); }
	
	static FORCEINLINE SIMD_float8 mod(SIMD_float8 v, SIMD_float8 m);

	static FORCEINLINE SIMD_float8 clamp(SIMD_float8 val, SIMD_float8 min, SIMD_float8 max);

	static FORCEINLINE SIMD_float8 rcp(SIMD_float8 floats);

	inline static FORCEINLINE SIMD_float8     sqrt(SIMD_float8 floats) { return SIMD_float8(_mm256_sqrt_ps   (floats.data)); }
	inline static FORCEINLINE SIMD_float8 inv_sqrt(SIMD_float8 floats) { return SIMD_float8(_mm256_invsqrt_ps(floats.data)); }

	inline static FORCEINLINE SIMD_float8 madd(SIMD_float8 a, SIMD_float8 b, SIMD_float8 c) { return SIMD_float8(_mm256_fmadd_ps(a.data, b.data, c.data)); } // Computes a*b + c
	inline static FORCEINLINE SIMD_float8 msub(SIMD_float8 a, SIMD_float8 b, SIMD_float8 c) { return SIMD_float8(_mm256_fmsub_ps(a.data, b.data, c.data)); } // Computes a*b - c
	
	inline static FORCEINLINE SIMD_float8 sin(SIMD_float8 floats) { return SIMD_float8(_mm256_sin_ps(floats.data)); }
	inline static FORCEINLINE SIMD_float8 cos(SIMD_float8 floats) { return SIMD_float8(_mm256_cos_ps(floats.data)); }
	
	inline static FORCEINLINE SIMD_float8 atan2(SIMD_float8 y, SIMD_float8 x) { return SIMD_float8(_mm256_atan2_ps(y.data, x.data)); }
	inline static FORCEINLINE SIMD_float8 acos (SIMD_float8 floats)           { return SIMD_float8(_mm256_acos_ps(floats.data)); }

	inline static FORCEINLINE SIMD_float8 exp(SIMD_float8 floats) { return SIMD_float8(_mm256_exp_ps(floats.data)); }

	inline static FORCEINLINE bool all_false(SIMD_float8 floats) { return _mm256_movemask_ps(floats.data) == 0x0; }
	inline static FORCEINLINE bool all_true (SIMD_float8 floats) { return _mm256_movemask_ps(floats.data) == 0xff; }

	// Computes (not a) and b
	inline static FORCEINLINE SIMD_float8 andnot(SIMD_float8 a, SIMD_float8 b) {
		return SIMD_float8(_mm256_andnot_ps(a.data, b.data));
	}

	inline static FORCEINLINE int mask(SIMD_float8 floats) { return _mm256_movemask_ps(floats.data); }
	
	inline FORCEINLINE       float & operator[](int index)       { return floats[index]; }
	inline FORCEINLINE const float & operator[](int index) const { return floats[index]; }
};

inline FORCEINLINE SIMD_float8 operator-(SIMD_float8 floats) { 
	return SIMD_float8(_mm256_sub_ps(_mm256_set1_ps(0.0f), floats.data)); 
}

inline FORCEINLINE SIMD_float8 operator+(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_add_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float8 operator-(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_sub_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float8 operator*(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_mul_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float8 operator/(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_div_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float8 operator|(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_or_ps (left.data, right.data)); }
inline FORCEINLINE SIMD_float8 operator^(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_xor_ps(left.data, right.data)); }
inline FORCEINLINE SIMD_float8 operator&(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_and_ps(left.data, right.data)); }

inline FORCEINLINE SIMD_float8 operator> (SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_GT_OQ)); }
inline FORCEINLINE SIMD_float8 operator>=(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_GE_OQ)); }
inline FORCEINLINE SIMD_float8 operator< (SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_LT_OQ)); }
inline FORCEINLINE SIMD_float8 operator<=(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_LE_OQ)); }

inline FORCEINLINE SIMD_float8 operator==(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_EQ_OQ)); }
inline FORCEINLINE SIMD_float8 operator!=(SIMD_float8 left, SIMD_float8 right) { return SIMD_float8(_mm256_cmp_ps(left.data, right.data, _CMP_NEQ_OQ)); }

inline FORCEINLINE SIMD_float8 SIMD_float8::rcp(SIMD_float8 floats) { return SIMD_float8(1.0f) / floats; }

// Represents 1 int
struct SIMD_int1 {
	int data;

	inline SIMD_int1() { /* leave uninitialized */ }

	inline explicit SIMD_int1(int i) : data(i) { }

	inline static FORCEINLINE SIMD_int1 min(SIMD_int1 a, SIMD_int1 b) { return SIMD_int1(a.data < b.data ? a.data : b.data); }
	inline static FORCEINLINE SIMD_int1 max(SIMD_int1 a, SIMD_int1 b) { return SIMD_int1(a.data > b.data ? a.data : b.data); }
	
	inline FORCEINLINE int & operator[](int index) { return data; }
};

inline FORCEINLINE SIMD_int1 operator-(SIMD_int1 ints) { 
	return SIMD_int1(-ints.data); 
}

inline FORCEINLINE SIMD_int1 operator+(SIMD_int1 left, SIMD_int1 right) { return SIMD_int1(left.data + right.data); }
inline FORCEINLINE SIMD_int1 operator-(SIMD_int1 left, SIMD_int1 right) { return SIMD_int1(left.data - right.data); }
inline FORCEINLINE SIMD_int1 operator*(SIMD_int1 left, SIMD_int1 right) { return SIMD_int1(left.data * right.data); }
inline FORCEINLINE SIMD_int1 operator/(SIMD_int1 left, SIMD_int1 right) { return SIMD_int1(left.data / right.data); }

inline FORCEINLINE SIMD_int1 operator> (SIMD_int1 left, SIMD_int1 right) { return SIMD_int1(left.data < right.data); }
inline FORCEINLINE SIMD_int1 operator< (SIMD_int1 left, SIMD_int1 right) { return SIMD_int1(left.data > right.data); }

inline FORCEINLINE SIMD_int1 operator==(SIMD_int1 left, SIMD_int1 right) { return SIMD_int1(left.data == right.data); }

// Represents 4 ints
struct SIMD_int4 {
	union { __m128i data; int ints[4]; };

	inline SIMD_int4() { /* leave uninitialized */ }

	inline explicit SIMD_int4(__m128i data) : data(data) { }

	inline explicit SIMD_int4(int i) : data(_mm_set1_epi32(i)) { }
	inline explicit SIMD_int4(int a, int b, int c, int d) : data(_mm_set_epi32(a, b, c, d)) { }

	inline static FORCEINLINE SIMD_int4 min(SIMD_int4 a, SIMD_int4 b) { return SIMD_int4(_mm_min_epi32(a.data, b.data)); }
	inline static FORCEINLINE SIMD_int4 max(SIMD_int4 a, SIMD_int4 b) { return SIMD_int4(_mm_max_epi32(a.data, b.data)); }
	
	inline FORCEINLINE       int & operator[](int index)       { return ints[index]; }
	inline FORCEINLINE const int & operator[](int index) const { return ints[index]; }
};

inline FORCEINLINE SIMD_int4 operator-(SIMD_int4 ints) { 
	return SIMD_int4(_mm_sub_epi32(_mm_set1_epi32(0), ints.data)); 
}

inline FORCEINLINE SIMD_int4 operator+(SIMD_int4 left, SIMD_int4 right) { return SIMD_int4(_mm_add_epi32  (left.data, right.data)); }
inline FORCEINLINE SIMD_int4 operator-(SIMD_int4 left, SIMD_int4 right) { return SIMD_int4(_mm_sub_epi32  (left.data, right.data)); }
inline FORCEINLINE SIMD_int4 operator*(SIMD_int4 left, SIMD_int4 right) { return SIMD_int4(_mm_mullo_epi32(left.data, right.data)); }
inline FORCEINLINE SIMD_int4 operator/(SIMD_int4 left, SIMD_int4 right) { return SIMD_int4(_mm_div_epi32  (left.data, right.data)); }

inline FORCEINLINE SIMD_int4 operator> (SIMD_int4 left, SIMD_int4 right) { return SIMD_int4(_mm_cmpgt_epi32(left.data, right.data)); }
inline FORCEINLINE SIMD_int4 operator< (SIMD_int4 left, SIMD_int4 right) { return SIMD_int4(_mm_cmplt_epi32(left.data, right.data)); }

inline FORCEINLINE SIMD_int4 operator==(SIMD_int4 left, SIMD_int4 right) { return SIMD_int4(_mm_cmpeq_epi32 (left.data, right.data)); }

// Represents 8 ints
struct SIMD_int8 {
	union { __m256i data; int ints[8]; };

	inline SIMD_int8() { /* leave uninitialized */ }

	inline explicit SIMD_int8(__m256i data) : data(data) { }

	inline explicit SIMD_int8(int i) : data(_mm256_set1_epi32(i)) { }
	inline explicit SIMD_int8(int a, int b, int c, int d, int e, int f, int g, int h) : data(_mm256_set_epi32(a, b, c, d, e, f, g, h)) { }

	inline static FORCEINLINE SIMD_int8 min(SIMD_int8 a, SIMD_int8 b) { return SIMD_int8(_mm256_min_epi32(a.data, b.data)); }
	inline static FORCEINLINE SIMD_int8 max(SIMD_int8 a, SIMD_int8 b) { return SIMD_int8(_mm256_max_epi32(a.data, b.data)); }
	
	inline FORCEINLINE       int & operator[](int index)       { return ints[index]; }
	inline FORCEINLINE const int & operator[](int index) const { return ints[index]; }
};

inline FORCEINLINE SIMD_int8 operator-(SIMD_int8 ints) { 
	return SIMD_int8(_mm256_sub_epi32(_mm256_set1_epi32(0), ints.data)); 
}

inline FORCEINLINE SIMD_int8 operator+(SIMD_int8 left, SIMD_int8 right) { return SIMD_int8(_mm256_add_epi32  (left.data, right.data)); }
inline FORCEINLINE SIMD_int8 operator-(SIMD_int8 left, SIMD_int8 right) { return SIMD_int8(_mm256_sub_epi32  (left.data, right.data)); }
inline FORCEINLINE SIMD_int8 operator*(SIMD_int8 left, SIMD_int8 right) { return SIMD_int8(_mm256_mullo_epi32(left.data, right.data)); }
inline FORCEINLINE SIMD_int8 operator/(SIMD_int8 left, SIMD_int8 right) { return SIMD_int8(_mm256_div_epi32  (left.data, right.data)); }

inline FORCEINLINE SIMD_int8 operator> (SIMD_int8 left, SIMD_int8 right) { return SIMD_int8(_mm256_cmpgt_epi32(left.data, right.data)); }
//inline FORCEINLINE SIMD_int8 operator< (SIMD_int8 left, SIMD_int8 right) { return SIMD_int8(_mm256_cmplt_epi32(left.data, right.data)); }

inline FORCEINLINE SIMD_int8 operator==(SIMD_int8 left, SIMD_int8 right) { return SIMD_int8(_mm256_cmpeq_epi32 (left.data, right.data)); }

#define SIMD_LANE_SIZE 4

#if SIMD_LANE_SIZE == 1
typedef SIMD_float1 SIMD_float;
typedef SIMD_int1   SIMD_int;

inline FORCEINLINE SIMD_int   SIMD_float_to_int(SIMD_float floats) { return SIMD_int  (floats.data); }
inline FORCEINLINE SIMD_float SIMD_int_to_float(SIMD_int   ints)   { return SIMD_float(ints.data); }
#elif SIMD_LANE_SIZE == 4
typedef SIMD_float4 SIMD_float;
typedef SIMD_int4   SIMD_int;

inline FORCEINLINE SIMD_int   SIMD_float_to_int(SIMD_float floats) { return SIMD_int  (_mm_cvtps_epi32(floats.data)); }
inline FORCEINLINE SIMD_float SIMD_int_to_float(SIMD_int   ints)   { return SIMD_float(_mm_cvtepi32_ps(ints.data)); }
#elif SIMD_LANE_SIZE == 8
typedef SIMD_float8 SIMD_float;
typedef SIMD_int8   SIMD_int;

inline FORCEINLINE SIMD_int   SIMD_float_to_int(SIMD_float floats) { return SIMD_int  (_mm256_cvtps_epi32(floats.data)); }
inline FORCEINLINE SIMD_float SIMD_int_to_float(SIMD_int   ints)   { return SIMD_float(_mm256_cvtepi32_ps(ints.data)); }
#else
static_assert(false, "Unsupported Lane Size!");
#endif

inline FORCEINLINE SIMD_float SIMD_float::mod(SIMD_float v, SIMD_float m) { return SIMD_float(v - m * SIMD_float::floor(v / m)); }

inline FORCEINLINE SIMD_float SIMD_float::clamp(SIMD_float val, SIMD_float min, SIMD_float max) { return SIMD_float::min(SIMD_float::max(val, min), max); }

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

	inline static FORCEINLINE SIMD_float length_squared(const SIMD_Vector3 & vector) {
		return dot(vector, vector);
	}

	inline static FORCEINLINE SIMD_float length(const SIMD_Vector3 & vector) {
		return SIMD_float::sqrt(length_squared(vector));
	}

	inline static FORCEINLINE SIMD_Vector3 normalize(const SIMD_Vector3 & vector) {
		SIMD_float inv_length = SIMD_float::inv_sqrt(length_squared(vector));
		return SIMD_Vector3(
			vector.x * inv_length, 
			vector.y * inv_length, 
			vector.z * inv_length
		);
	}

	inline static FORCEINLINE SIMD_float dot(const SIMD_Vector3 & left, const SIMD_Vector3 & right) {
		return SIMD_float::madd(left.x, right.x, SIMD_float::madd(left.y, right.y, left.z * right.z));
	}

	inline static FORCEINLINE SIMD_Vector3 cross(const SIMD_Vector3 & left, const SIMD_Vector3 & right) {
		return SIMD_Vector3(
			SIMD_float::msub(left.y, right.z, left.z * right.y),
			SIMD_float::msub(left.z, right.x, left.x * right.z),
			SIMD_float::msub(left.x, right.y, left.y * right.x)
		);
	}

	inline static FORCEINLINE SIMD_Vector3 rcp(const SIMD_Vector3 & vector) {
		return SIMD_Vector3(
			SIMD_float::rcp(vector.x),
			SIMD_float::rcp(vector.y),
			SIMD_float::rcp(vector.z)
		);
	}

	inline static FORCEINLINE SIMD_Vector3 min(const SIMD_Vector3 & left, const SIMD_Vector3 & right) {
		return SIMD_Vector3(
			SIMD_float::min(left.x, right.x),
			SIMD_float::min(left.y, right.y),
			SIMD_float::min(left.z, right.z)
		);
	}

	inline static FORCEINLINE SIMD_Vector3 max(const SIMD_Vector3 & left, const SIMD_Vector3 & right) {
		return SIMD_Vector3(
			SIMD_float::max(left.x, right.x),
			SIMD_float::max(left.y, right.y),
			SIMD_float::max(left.z, right.z)
		);
	}

	inline static FORCEINLINE SIMD_Vector3 blend(const SIMD_Vector3 & left, const SIMD_Vector3 & right, SIMD_float mask) {
		return SIMD_Vector3(
			SIMD_float::blend(left.x, right.x, mask),
			SIMD_float::blend(left.y, right.y, mask),
			SIMD_float::blend(left.z, right.z, mask)
		);
	}

	inline FORCEINLINE SIMD_Vector3 operator+=(const SIMD_Vector3 & vector) { x = x + vector.x; y = y + vector.y; z = z + vector.z; return *this; }
	inline FORCEINLINE SIMD_Vector3 operator-=(const SIMD_Vector3 & vector) { x = x - vector.x; y = y - vector.y; z = z - vector.z; return *this; }
	inline FORCEINLINE SIMD_Vector3 operator*=(const SIMD_Vector3 & vector) { x = x * vector.x; y = y * vector.y; z = z * vector.z; return *this; }
	inline FORCEINLINE SIMD_Vector3 operator/=(const SIMD_Vector3 & vector) { x = x / vector.x; y = y / vector.y; z = z / vector.z; return *this; }

	inline FORCEINLINE SIMD_Vector3 operator+=(SIMD_float f) {                                        x = x + f;      y = y + f;      z = z + f;      return *this; }
	inline FORCEINLINE SIMD_Vector3 operator-=(SIMD_float f) {                                        x = x - f;      y = y - f;      z = z - f;      return *this; }
	inline FORCEINLINE SIMD_Vector3 operator*=(SIMD_float f) {                                        x = x * f;      y = y * f;      z = z * f;      return *this; }
	inline FORCEINLINE SIMD_Vector3 operator/=(SIMD_float f) { SIMD_float inv_f = SIMD_float::rcp(f); x = x * inv_f;  y = y * inv_f;  z = z * inv_f;  return *this; }
};

inline FORCEINLINE SIMD_Vector3 operator-(const SIMD_Vector3 & vector) { 
	SIMD_float zero(0.0f);
	return SIMD_Vector3(zero - vector.x, zero - vector.y, zero - vector.z); 
}

inline FORCEINLINE SIMD_Vector3 operator+(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x + right.x, left.y + right.y, left.z + right.z); }
inline FORCEINLINE SIMD_Vector3 operator-(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x - right.x, left.y - right.y, left.z - right.z); }
inline FORCEINLINE SIMD_Vector3 operator*(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x * right.x, left.y * right.y, left.z * right.z); }
inline FORCEINLINE SIMD_Vector3 operator/(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x / right.x, left.y / right.y, left.z / right.z); }

inline FORCEINLINE SIMD_Vector3 operator+(const SIMD_Vector3 & vector, SIMD_float f) {                                        return SIMD_Vector3(vector.x + f,      vector.y + f,      vector.z + f); }
inline FORCEINLINE SIMD_Vector3 operator-(const SIMD_Vector3 & vector, SIMD_float f) {                                        return SIMD_Vector3(vector.x - f,      vector.y - f,      vector.z - f); }
inline FORCEINLINE SIMD_Vector3 operator*(const SIMD_Vector3 & vector, SIMD_float f) {                                        return SIMD_Vector3(vector.x * f,      vector.y * f,      vector.z * f); }
inline FORCEINLINE SIMD_Vector3 operator/(const SIMD_Vector3 & vector, SIMD_float f) { SIMD_float inv_f = SIMD_float::rcp(f); return SIMD_Vector3(vector.x * inv_f,  vector.y * inv_f,  vector.z * inv_f); }

inline FORCEINLINE SIMD_Vector3 operator+(SIMD_float f, const SIMD_Vector3 & vector) {                                        return SIMD_Vector3(vector.x + f,      vector.y + f,      vector.z + f); }
inline FORCEINLINE SIMD_Vector3 operator-(SIMD_float f, const SIMD_Vector3 & vector) {                                        return SIMD_Vector3(vector.x - f,      vector.y - f,      vector.z - f); }
inline FORCEINLINE SIMD_Vector3 operator*(SIMD_float f, const SIMD_Vector3 & vector) {                                        return SIMD_Vector3(vector.x * f,      vector.y * f,      vector.z * f); }
inline FORCEINLINE SIMD_Vector3 operator/(SIMD_float f, const SIMD_Vector3 & vector) { SIMD_float inv_f = SIMD_float::rcp(f); return SIMD_Vector3(vector.x * inv_f,  vector.y * inv_f,  vector.z * inv_f); }

inline FORCEINLINE SIMD_float operator==(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return (left.x == right.x) & (left.y == right.y) & (left.z == right.z); }
inline FORCEINLINE SIMD_float operator!=(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return (left.x != right.x) | (left.y != right.y) | (left.z != right.z); }
