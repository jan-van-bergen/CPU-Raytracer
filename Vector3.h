#pragma once
#include <math.h>

struct Vector3 {
	float x, y, z;
	
	inline Vector3() : x(0.0f), y(0.0f), z(0.0f) { }
	inline Vector3(float x, float y, float z) : x(x), y(y), z(z) { }

	inline float length_squared() const {
		return dot(*this, *this);
	}

	inline float length() const {
		return sqrtf(length_squared());
	}

	inline static Vector3 normalize(const Vector3 & vector) {
		float inv_length = 1.0f / vector.length();
		return Vector3(vector.x * inv_length, vector.y * inv_length, vector.z * inv_length);
	}

	inline static float dot(const Vector3 & left, const Vector3 & right) {
		return left.x * right.x + left.y * right.y + left.z * right.z;
	}

	inline static Vector3 cross(const Vector3 left, const Vector3 & right) {
		return Vector3(
			left.y * right.z - left.z * right.y,
			left.z * right.x - left.x * right.z,
			left.x * right.y - left.y * right.x
		);
	}

	inline Vector3 operator+=(const Vector3 & vector) { x += vector.x; y += vector.y; z += vector.z; return *this; }
	inline Vector3 operator-=(const Vector3 & vector) { x -= vector.x; y -= vector.y; z -= vector.z; return *this; }
	inline Vector3 operator*=(const Vector3 & vector) { x *= vector.x; y *= vector.y; z *= vector.z; return *this; }
	inline Vector3 operator/=(const Vector3 & vector) { x /= vector.x; y /= vector.y; z /= vector.z; return *this; }

	inline Vector3 operator+=(float scalar) {                                  x += scalar;     y += scalar;     z += scalar;     return *this; }
	inline Vector3 operator-=(float scalar) {                                  x -= scalar;     y -= scalar;     z -= scalar;     return *this; }
	inline Vector3 operator*=(float scalar) {                                  x *= scalar;     y *= scalar;     z *= scalar;     return *this; }
	inline Vector3 operator/=(float scalar) { float inv_scalar = 1.0 / scalar; x *= inv_scalar; y *= inv_scalar; z *= inv_scalar; return *this; }
};

inline Vector3 operator+(const Vector3 & left, const Vector3 & right) { return Vector3(left.x + right.x, left.y + right.y, left.z + right.z); }
inline Vector3 operator-(const Vector3 & left, const Vector3 & right) { return Vector3(left.x - right.x, left.y - right.y, left.z - right.z); }
inline Vector3 operator*(const Vector3 & left, const Vector3 & right) { return Vector3(left.x * right.x, left.y * right.y, left.z * right.z); }
inline Vector3 operator/(const Vector3 & left, const Vector3 & right) { return Vector3(left.x / right.x, left.y / right.y, left.z / right.z); }

inline Vector3 operator+(const Vector3 & vector, float scalar) {                                  return Vector3(vector.x + scalar,     vector.y + scalar,     vector.z + scalar); }
inline Vector3 operator-(const Vector3 & vector, float scalar) {                                  return Vector3(vector.x - scalar,     vector.y - scalar,     vector.z - scalar); }
inline Vector3 operator*(const Vector3 & vector, float scalar) {                                  return Vector3(vector.x * scalar,     vector.y * scalar,     vector.z * scalar); }
inline Vector3 operator/(const Vector3 & vector, float scalar) { float inv_scalar = 1.0 / scalar; return Vector3(vector.x * inv_scalar, vector.y * inv_scalar, vector.z * inv_scalar); }

inline Vector3 operator+(float scalar, const Vector3 & vector) {                                  return Vector3(vector.x + scalar,     vector.y + scalar,     vector.z + scalar); }
inline Vector3 operator-(float scalar, const Vector3 & vector) {                                  return Vector3(vector.x - scalar,     vector.y - scalar,     vector.z - scalar); }
inline Vector3 operator*(float scalar, const Vector3 & vector) {                                  return Vector3(vector.x * scalar,     vector.y * scalar,     vector.z * scalar); }
inline Vector3 operator/(float scalar, const Vector3 & vector) { float inv_scalar = 1.0 / scalar; return Vector3(vector.x * inv_scalar, vector.y * inv_scalar, vector.z * inv_scalar); }
