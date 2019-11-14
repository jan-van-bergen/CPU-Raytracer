#pragma once
#include <math.h>

struct Vector2 {
	float x, y;

	inline Vector2() : x(0.0f), y(0.0f) { }
	inline Vector2(float x, float y) : x(x), y(y) { }

	inline float length_squared() const {
		return dot(*this, *this);
	}

	inline float length() const {
		return sqrtf(length_squared());
	}

	inline static Vector2 normalize(const Vector2 & vector) {
		float inv_length = 1.0f / vector.length();
		return vector * inv_length;
	}

	inline static float dot(const Vector2 & left, const Vector2 & right) {
		return left.x * right.x + left.y * right.y;
	}
};

inline Vector2 operator+(const Vector2 & left, const Vector2 & right) { return Vector2(left.x + right.x, left.y + right.y); }
inline Vector2 operator-(const Vector2 & left, const Vector2 & right) { return Vector2(left.x - right.x, left.y - right.y); }
inline Vector2 operator*(const Vector2 & left, const Vector2 & right) { return Vector2(left.x * right.x, left.y * right.y); }
inline Vector2 operator/(const Vector2 & left, const Vector2 & right) { return Vector2(left.x / right.x, left.y / right.y); }

inline Vector2 operator+(const Vector2 & vector, float scalar) {                                  return Vector2(vector.x + scalar,     vector.y + scalar); }
inline Vector2 operator-(const Vector2 & vector, float scalar) {                                  return Vector2(vector.x - scalar,     vector.y - scalar); }
inline Vector2 operator*(const Vector2 & vector, float scalar) {                                  return Vector2(vector.x * scalar,     vector.y * scalar); }
inline Vector2 operator/(const Vector2 & vector, float scalar) { float inv_scalar = 1.0 / scalar; return Vector2(vector.x * inv_scalar, vector.y * inv_scalar);  }
