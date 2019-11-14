#pragma once
#include "Vector3.h"
#include "Quaternion.h"

struct Camera {
	Vector3    position;	
	Quaternion rotation;

	void update(float delta);
};
