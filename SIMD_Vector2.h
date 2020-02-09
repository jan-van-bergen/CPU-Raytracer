#pragma once
#include "SIMD.h"

#include "Vector2.h"

// Represents SIMD_LANE_SIZE Vector2s
struct SIMD_Vector2 {
	SIMD_float x;
	SIMD_float y;

	inline          SIMD_Vector2() : x(0.0f), y(0.0f) { }
	inline explicit SIMD_Vector2(SIMD_float f) : x(f), y(f) {}
	inline          SIMD_Vector2(SIMD_float x, SIMD_float y) : x(x), y(y) { }

	inline explicit SIMD_Vector2(const Vector2 & vector) {
		x = SIMD_float(vector.x);
		y = SIMD_float(vector.y);
	}
	
#if SIMD_LANE_SIZE == 4
	inline SIMD_Vector2(const Vector2 & a, const Vector2 & b, const Vector2 & c, const Vector2 & d) {
		x = SIMD_float(a.x, b.x, c.x, d.x);
		y = SIMD_float(a.y, b.y, c.y, d.y);
	}
#elif SIMD_LANE_SIZE == 8
	inline SIMD_Vector2(const Vector2 & a, const Vector2 & b, const Vector2 & c, const Vector2 & d, const Vector2 & e, const Vector2 & f, const Vector2 & g, const Vector2 & h) {
		x = SIMD_float(a.x, b.x, c.x, d.x, e.x, f.x, g.x, h.x);
		y = SIMD_float(a.y, b.y, c.y, d.y, e.y, f.y, g.y, h.y);
	}
#endif

	inline static FORCEINLINE SIMD_float length_squared(const SIMD_Vector2 & vector) {
		return dot(vector, vector);
	}

	inline static FORCEINLINE SIMD_float length(const SIMD_Vector2 & vector) {
		return SIMD_float::sqrt(length_squared(vector));
	}

	inline static FORCEINLINE SIMD_Vector2 normalize(const SIMD_Vector2 & vector) {
		SIMD_float inv_length = SIMD_float::inv_sqrt(length_squared(vector));
		return SIMD_Vector2(
			vector.x * inv_length,
			vector.y * inv_length
		);
	}

	inline static FORCEINLINE SIMD_float dot(const SIMD_Vector2 & left, const SIMD_Vector2 & right) {
		return SIMD_float::madd(left.x, right.x, left.y* right.y);
	}

	inline static FORCEINLINE SIMD_Vector2 rcp(const SIMD_Vector2 & vector) {
		return SIMD_Vector2(
			SIMD_float::rcp(vector.x),
			SIMD_float::rcp(vector.y)
		);
	}

	inline static FORCEINLINE SIMD_Vector2 min(const SIMD_Vector2 & left, const SIMD_Vector2 & right) {
		return SIMD_Vector2(
			SIMD_float::min(left.x, right.x),
			SIMD_float::min(left.y, right.y)
		);
	}

	inline static FORCEINLINE SIMD_Vector2 max(const SIMD_Vector2 & left, const SIMD_Vector2 & right) {
		return SIMD_Vector2(
			SIMD_float::max(left.x, right.x),
			SIMD_float::max(left.y, right.y)
		);
	}

	inline static FORCEINLINE SIMD_Vector2 blend(const SIMD_Vector2 & left, const SIMD_Vector2 & right, const SIMD_float & mask) {
		return SIMD_Vector2(
			SIMD_float::blend(left.x, right.x, mask),
			SIMD_float::blend(left.y, right.y, mask)
		);
	}

	inline static FORCEINLINE SIMD_Vector2 madd(const SIMD_Vector2 & a, const SIMD_Vector2 & b, const SIMD_Vector2 & c) {
		return SIMD_Vector2(
			SIMD_float::madd(a.x, b.x, c.x),
			SIMD_float::madd(a.y, b.y, c.y)
		);
	}

	inline static FORCEINLINE SIMD_Vector2 madd(const SIMD_Vector2 & a, const SIMD_float & b, const SIMD_Vector2 & c) {
		return SIMD_Vector2(
			SIMD_float::madd(a.x, b, c.x),
			SIMD_float::madd(a.y, b, c.y)
		);
	}

	inline static FORCEINLINE SIMD_Vector2 msub(const SIMD_Vector2 & a, const SIMD_Vector2 & b, const SIMD_Vector2 & c) {
		return SIMD_Vector2(
			SIMD_float::msub(a.x, b.x, c.x),
			SIMD_float::msub(a.y, b.y, c.y)
		);
	}

	inline static FORCEINLINE SIMD_Vector2 msub(const SIMD_Vector2 & a, const SIMD_float & b, const SIMD_Vector2 & c) {
		return SIMD_Vector2(
			SIMD_float::msub(a.x, b, c.x),
			SIMD_float::msub(a.y, b, c.y)
		);
	}

	inline FORCEINLINE SIMD_Vector2 operator+=(const SIMD_Vector2 & vector) { x = x + vector.x; y = y + vector.y; return *this; }
	inline FORCEINLINE SIMD_Vector2 operator-=(const SIMD_Vector2 & vector) { x = x - vector.x; y = y - vector.y; return *this; }
	inline FORCEINLINE SIMD_Vector2 operator*=(const SIMD_Vector2 & vector) { x = x * vector.x; y = y * vector.y; return *this; }
	inline FORCEINLINE SIMD_Vector2 operator/=(const SIMD_Vector2 & vector) { x = x / vector.x; y = y / vector.y; return *this; }

	inline FORCEINLINE SIMD_Vector2 operator+=(const SIMD_float & f) {                                        x = x + f;      y = y + f;      return *this; }
	inline FORCEINLINE SIMD_Vector2 operator-=(const SIMD_float & f) {                                        x = x - f;      y = y - f;      return *this; }
	inline FORCEINLINE SIMD_Vector2 operator*=(const SIMD_float & f) {                                        x = x * f;      y = y * f;      return *this; }
	inline FORCEINLINE SIMD_Vector2 operator/=(const SIMD_float & f) { SIMD_float inv_f = SIMD_float::rcp(f); x = x * inv_f;  y = y * inv_f;  return *this; }
};

inline FORCEINLINE SIMD_Vector2 operator-(const SIMD_Vector2 & vector) { 
	SIMD_float zero(0.0f);
	return SIMD_Vector2(zero - vector.x, zero - vector.y); 
}

inline FORCEINLINE SIMD_Vector2 operator+(const SIMD_Vector2 & left, const SIMD_Vector2 & right) { return SIMD_Vector2(left.x + right.x, left.y + right.y); }
inline FORCEINLINE SIMD_Vector2 operator-(const SIMD_Vector2 & left, const SIMD_Vector2 & right) { return SIMD_Vector2(left.x - right.x, left.y - right.y); }
inline FORCEINLINE SIMD_Vector2 operator*(const SIMD_Vector2 & left, const SIMD_Vector2 & right) { return SIMD_Vector2(left.x * right.x, left.y * right.y); }
inline FORCEINLINE SIMD_Vector2 operator/(const SIMD_Vector2 & left, const SIMD_Vector2 & right) { return SIMD_Vector2(left.x / right.x, left.y / right.y); }

inline FORCEINLINE SIMD_Vector2 operator+(const SIMD_Vector2 & vector, const SIMD_float & f) {                                        return SIMD_Vector2(vector.x + f,      vector.y + f); }
inline FORCEINLINE SIMD_Vector2 operator-(const SIMD_Vector2 & vector, const SIMD_float & f) {                                        return SIMD_Vector2(vector.x - f,      vector.y - f); }
inline FORCEINLINE SIMD_Vector2 operator*(const SIMD_Vector2 & vector, const SIMD_float & f) {                                        return SIMD_Vector2(vector.x * f,      vector.y * f); }
inline FORCEINLINE SIMD_Vector2 operator/(const SIMD_Vector2 & vector, const SIMD_float & f) { SIMD_float inv_f = SIMD_float::rcp(f); return SIMD_Vector2(vector.x * inv_f,  vector.y * inv_f); }

inline FORCEINLINE SIMD_Vector2 operator+(SIMD_float f, const SIMD_Vector2 & vector) {                                        return SIMD_Vector2(vector.x + f,      vector.y + f); }
inline FORCEINLINE SIMD_Vector2 operator-(SIMD_float f, const SIMD_Vector2 & vector) {                                        return SIMD_Vector2(vector.x - f,      vector.y - f); }
inline FORCEINLINE SIMD_Vector2 operator*(SIMD_float f, const SIMD_Vector2 & vector) {                                        return SIMD_Vector2(vector.x * f,      vector.y * f); }
inline FORCEINLINE SIMD_Vector2 operator/(SIMD_float f, const SIMD_Vector2 & vector) { SIMD_float inv_f = SIMD_float::rcp(f); return SIMD_Vector2(vector.x * inv_f,  vector.y * inv_f); }

inline FORCEINLINE SIMD_float operator==(const SIMD_Vector2 & left, const SIMD_Vector2 & right) { return (left.x == right.x) & (left.y == right.y); }
inline FORCEINLINE SIMD_float operator!=(const SIMD_Vector2 & left, const SIMD_Vector2 & right) { return (left.x != right.x) | (left.y != right.y); }
