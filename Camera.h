#pragma once
#include "Vector3.h"
#include "Quaternion.h"

#include "Ray.h"

#include "Util.h"

struct Camera {
public:
	Vector3    position;	
	Quaternion rotation;

	float fov;

private:
	Vector3 top_left_corner;
	Vector3 u_axis;
	Vector3 v_axis;

public:
	inline Camera(float fov) : fov(DEG_TO_RAD(fov)) { }

	void resize(int width, int height);

	Ray get_ray(float x, float y) const; 

	void update(float delta, const unsigned char * keys);
};
