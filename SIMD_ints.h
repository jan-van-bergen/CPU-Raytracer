#pragma once
#include <immintrin.h>

#include "Util.h"

// Represents 1 int
struct SIMD_int1 {
	int data;

	inline SIMD_int1() { /* leave uninitialized */ }

	inline explicit SIMD_int1(int i) : data(i) { }
	
	inline static FORCEINLINE SIMD_int1 blend(SIMD_int1 case_false, SIMD_int1 case_true, SIMD_int1 mask) {
		return SIMD_int1(mask.data ? case_true.data : case_false.data);
	}

	inline static FORCEINLINE SIMD_int1 min(SIMD_int1 a, SIMD_int1 b) { return SIMD_int1(a.data < b.data ? a.data : b.data); }
	inline static FORCEINLINE SIMD_int1 max(SIMD_int1 a, SIMD_int1 b) { return SIMD_int1(a.data > b.data ? a.data : b.data); }
	
	inline FORCEINLINE int & operator[](int index) { assert(index == 0); return data; }
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
	
	inline static FORCEINLINE SIMD_int4 blend(const SIMD_int4 & case_false, const SIMD_int4 & case_true, const SIMD_int4 & mask) {
		return SIMD_int4(_mm_blendv_epi8(case_false.data, case_true.data, mask.data));
	}

	inline static FORCEINLINE SIMD_int4 min(const SIMD_int4 & a, const SIMD_int4 & b) { return SIMD_int4(_mm_min_epi32(a.data, b.data)); }
	inline static FORCEINLINE SIMD_int4 max(const SIMD_int4 & a, const SIMD_int4 & b) { return SIMD_int4(_mm_max_epi32(a.data, b.data)); }
	
	inline FORCEINLINE       int & operator[](int index)       { assert(index >= 0 && index < 4); return ints[index]; }
	inline FORCEINLINE const int & operator[](int index) const { assert(index >= 0 && index < 4); return ints[index]; }
};

inline FORCEINLINE SIMD_int4 operator-(const SIMD_int4 & ints) { 
	return SIMD_int4(_mm_sub_epi32(_mm_set1_epi32(0), ints.data)); 
}

inline FORCEINLINE SIMD_int4 operator+(const SIMD_int4 & left, const SIMD_int4 & right) { return SIMD_int4(_mm_add_epi32  (left.data, right.data)); }
inline FORCEINLINE SIMD_int4 operator-(const SIMD_int4 & left, const SIMD_int4 & right) { return SIMD_int4(_mm_sub_epi32  (left.data, right.data)); }
inline FORCEINLINE SIMD_int4 operator*(const SIMD_int4 & left, const SIMD_int4 & right) { return SIMD_int4(_mm_mullo_epi32(left.data, right.data)); }
inline FORCEINLINE SIMD_int4 operator/(const SIMD_int4 & left, const SIMD_int4 & right) { return SIMD_int4(_mm_div_epi32  (left.data, right.data)); }

inline FORCEINLINE SIMD_int4 operator> (const SIMD_int4 & left, const SIMD_int4 & right) { return SIMD_int4(_mm_cmpgt_epi32(left.data, right.data)); }
inline FORCEINLINE SIMD_int4 operator< (const SIMD_int4 & left, const SIMD_int4 & right) { return SIMD_int4(_mm_cmplt_epi32(left.data, right.data)); }

inline FORCEINLINE SIMD_int4 operator==(const SIMD_int4 & left, const SIMD_int4 & right) { return SIMD_int4(_mm_cmpeq_epi32 (left.data, right.data)); }

// Represents 8 ints
struct SIMD_int8 {
	union { __m256i data; int ints[8]; };

	inline SIMD_int8() { /* leave uninitialized */ }

	inline explicit SIMD_int8(__m256i data) : data(data) { }
	
	inline static FORCEINLINE SIMD_int8 blend(const SIMD_int8 & case_false, const SIMD_int8 & case_true, const SIMD_int8 & mask) {
		return SIMD_int8(_mm256_blendv_epi8(case_false.data, case_true.data, mask.data));
	}

	inline explicit SIMD_int8(int i) : data(_mm256_set1_epi32(i)) { }
	inline explicit SIMD_int8(int a, int b, int c, int d, int e, int f, int g, int h) : data(_mm256_set_epi32(a, b, c, d, e, f, g, h)) { }

	inline static FORCEINLINE SIMD_int8 min(const SIMD_int8 & a, const SIMD_int8 & b) { return SIMD_int8(_mm256_min_epi32(a.data, b.data)); }
	inline static FORCEINLINE SIMD_int8 max(const SIMD_int8 & a, const SIMD_int8 & b) { return SIMD_int8(_mm256_max_epi32(a.data, b.data)); }
	
	inline FORCEINLINE       int & operator[](int index)       { assert(index >= 0 && index < 8); return ints[index]; }
	inline FORCEINLINE const int & operator[](int index) const { assert(index >= 0 && index < 8); return ints[index]; }
};

inline FORCEINLINE SIMD_int8 operator-(SIMD_int8 ints) { 
	return SIMD_int8(_mm256_sub_epi32(_mm256_set1_epi32(0), ints.data)); 
}

inline FORCEINLINE SIMD_int8 operator+(const SIMD_int8 & left, const SIMD_int8 & right) { return SIMD_int8(_mm256_add_epi32  (left.data, right.data)); }
inline FORCEINLINE SIMD_int8 operator-(const SIMD_int8 & left, const SIMD_int8 & right) { return SIMD_int8(_mm256_sub_epi32  (left.data, right.data)); }
inline FORCEINLINE SIMD_int8 operator*(const SIMD_int8 & left, const SIMD_int8 & right) { return SIMD_int8(_mm256_mullo_epi32(left.data, right.data)); }
inline FORCEINLINE SIMD_int8 operator/(const SIMD_int8 & left, const SIMD_int8 & right) { return SIMD_int8(_mm256_div_epi32  (left.data, right.data)); }

inline FORCEINLINE SIMD_int8 operator> (const SIMD_int8 & left, const SIMD_int8 & right) { return SIMD_int8(_mm256_cmpgt_epi32(left.data, right.data)); }
//inline FORCEINLINE SIMD_int8 operator< (SIMD_int8 left, SIMD_int8 right) { return SIMD_int8(_mm256_cmplt_epi32(left.data, right.data)); }

inline FORCEINLINE SIMD_int8 operator==(const SIMD_int8 & left, const SIMD_int8 & right) { return SIMD_int8(_mm256_cmpeq_epi32 (left.data, right.data)); }
