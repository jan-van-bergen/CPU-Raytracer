#pragma once
#include "SIMD.h"

#include "Vector3.h"
#include "Quaternion.h"

#include "Ray.h"

#include "Util.h"

struct Camera {
public:
	Vector3    position;	
	Quaternion rotation;

	float fov; // Field of View in radians

private:
	Vector3 top_left_corner, top_left_corner_rotated;
	Vector3 x_axis,          x_axis_rotated;
	Vector3 y_axis,          y_axis_rotated;

public:
	inline Camera(float fov) : fov(DEG_TO_RAD(fov)) { }

	void resize(int width, int height);

	SIMD_Vector3 get_ray_direction(__m128 x, __m128 y) const; 

	void update(float delta, const unsigned char * keys);
};
