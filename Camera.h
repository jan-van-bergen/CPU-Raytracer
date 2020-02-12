#pragma once
#include "SIMD.h"

#include "Vector3.h"
#include "Quaternion.h"

#include "Ray.h"

#include "Util.h"

struct Camera {
	Vector3    position;	
	Quaternion rotation;

	float fov; // Field of View in radians

	Vector3 top_left_corner;
	Vector3 x_axis;
	Vector3 y_axis;

	SIMD_Vector3 rotated_top_left_corner;
	SIMD_Vector3 rotated_x_axis;
	SIMD_Vector3 rotated_y_axis;

	inline Camera(float fov) : fov(fov) { }

	void resize(int width, int height);

	void update(float delta, const unsigned char * keys);
};
