#pragma once
#include "Light.h"
#include "Quaternion.h"

struct SpotLight : Light {
	Vector3 position;
	float   range;

	Vector3 direction;
	float   cutoff;
};
