#pragma once
#include "SIMD.h"

#include "Vector3.h"

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

	inline static FORCEINLINE SIMD_Vector3 blend(const SIMD_Vector3 & left, const SIMD_Vector3 & right, const SIMD_float & mask) {
		return SIMD_Vector3(
			SIMD_float::blend(left.x, right.x, mask),
			SIMD_float::blend(left.y, right.y, mask),
			SIMD_float::blend(left.z, right.z, mask)
		);
	}

	inline static FORCEINLINE SIMD_Vector3 madd(const SIMD_Vector3 & a, const SIMD_Vector3 & b, const SIMD_Vector3 & c) {
		return SIMD_Vector3(
			SIMD_float::madd(a.x, b.x, c.x),
			SIMD_float::madd(a.y, b.y, c.y),
			SIMD_float::madd(a.z, b.z, c.z)
		);
	}

	inline static FORCEINLINE SIMD_Vector3 madd(const SIMD_Vector3 & a, const SIMD_float & b, const SIMD_Vector3 & c) {
		return SIMD_Vector3(
			SIMD_float::madd(a.x, b, c.x),
			SIMD_float::madd(a.y, b, c.y),
			SIMD_float::madd(a.z, b, c.z)
		);
	}

	inline static FORCEINLINE SIMD_Vector3 msub(const SIMD_Vector3 & a, const SIMD_Vector3 & b, const SIMD_Vector3 & c) {
		return SIMD_Vector3(
			SIMD_float::msub(a.x, b.x, c.x),
			SIMD_float::msub(a.y, b.y, c.y),
			SIMD_float::msub(a.z, b.z, c.z)
		);
	}

	inline static FORCEINLINE SIMD_Vector3 msub(const SIMD_Vector3 & a, const SIMD_float & b, const SIMD_Vector3 & c) {
		return SIMD_Vector3(
			SIMD_float::msub(a.x, b, c.x),
			SIMD_float::msub(a.y, b, c.y),
			SIMD_float::msub(a.z, b, c.z)
		);
	}

	inline FORCEINLINE SIMD_Vector3 operator+=(const SIMD_Vector3 & vector) { x = x + vector.x; y = y + vector.y; z = z + vector.z; return *this; }
	inline FORCEINLINE SIMD_Vector3 operator-=(const SIMD_Vector3 & vector) { x = x - vector.x; y = y - vector.y; z = z - vector.z; return *this; }
	inline FORCEINLINE SIMD_Vector3 operator*=(const SIMD_Vector3 & vector) { x = x * vector.x; y = y * vector.y; z = z * vector.z; return *this; }
	inline FORCEINLINE SIMD_Vector3 operator/=(const SIMD_Vector3 & vector) { x = x / vector.x; y = y / vector.y; z = z / vector.z; return *this; }

	inline FORCEINLINE SIMD_Vector3 operator+=(const SIMD_float & f) {                                        x = x + f;      y = y + f;      z = z + f;      return *this; }
	inline FORCEINLINE SIMD_Vector3 operator-=(const SIMD_float & f) {                                        x = x - f;      y = y - f;      z = z - f;      return *this; }
	inline FORCEINLINE SIMD_Vector3 operator*=(const SIMD_float & f) {                                        x = x * f;      y = y * f;      z = z * f;      return *this; }
	inline FORCEINLINE SIMD_Vector3 operator/=(const SIMD_float & f) { SIMD_float inv_f = SIMD_float::rcp(f); x = x * inv_f;  y = y * inv_f;  z = z * inv_f;  return *this; }
};

inline FORCEINLINE SIMD_Vector3 operator-(const SIMD_Vector3 & vector) { 
	SIMD_float zero(0.0f);
	return SIMD_Vector3(zero - vector.x, zero - vector.y, zero - vector.z); 
}

inline FORCEINLINE SIMD_Vector3 operator+(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x + right.x, left.y + right.y, left.z + right.z); }
inline FORCEINLINE SIMD_Vector3 operator-(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x - right.x, left.y - right.y, left.z - right.z); }
inline FORCEINLINE SIMD_Vector3 operator*(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x * right.x, left.y * right.y, left.z * right.z); }
inline FORCEINLINE SIMD_Vector3 operator/(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return SIMD_Vector3(left.x / right.x, left.y / right.y, left.z / right.z); }

inline FORCEINLINE SIMD_Vector3 operator+(const SIMD_Vector3 & vector, const SIMD_float & f) {                                        return SIMD_Vector3(vector.x + f,      vector.y + f,      vector.z + f); }
inline FORCEINLINE SIMD_Vector3 operator-(const SIMD_Vector3 & vector, const SIMD_float & f) {                                        return SIMD_Vector3(vector.x - f,      vector.y - f,      vector.z - f); }
inline FORCEINLINE SIMD_Vector3 operator*(const SIMD_Vector3 & vector, const SIMD_float & f) {                                        return SIMD_Vector3(vector.x * f,      vector.y * f,      vector.z * f); }
inline FORCEINLINE SIMD_Vector3 operator/(const SIMD_Vector3 & vector, const SIMD_float & f) { SIMD_float inv_f = SIMD_float::rcp(f); return SIMD_Vector3(vector.x * inv_f,  vector.y * inv_f,  vector.z * inv_f); }

inline FORCEINLINE SIMD_Vector3 operator+(SIMD_float f, const SIMD_Vector3 & vector) {                                        return SIMD_Vector3(vector.x + f,      vector.y + f,      vector.z + f); }
inline FORCEINLINE SIMD_Vector3 operator-(SIMD_float f, const SIMD_Vector3 & vector) {                                        return SIMD_Vector3(vector.x - f,      vector.y - f,      vector.z - f); }
inline FORCEINLINE SIMD_Vector3 operator*(SIMD_float f, const SIMD_Vector3 & vector) {                                        return SIMD_Vector3(vector.x * f,      vector.y * f,      vector.z * f); }
inline FORCEINLINE SIMD_Vector3 operator/(SIMD_float f, const SIMD_Vector3 & vector) { SIMD_float inv_f = SIMD_float::rcp(f); return SIMD_Vector3(vector.x * inv_f,  vector.y * inv_f,  vector.z * inv_f); }

inline FORCEINLINE SIMD_float operator==(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return (left.x == right.x) & (left.y == right.y) & (left.z == right.z); }
inline FORCEINLINE SIMD_float operator!=(const SIMD_Vector3 & left, const SIMD_Vector3 & right) { return (left.x != right.x) | (left.y != right.y) | (left.z != right.z); }
