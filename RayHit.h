#pragma once
#include "Vector3.h"
#include "Material.h"

struct RayHit {
	bool  hit      = false;
	float distance = INFINITY;

	Vector3 point;
	Vector3 normal;

	const Material * material;
	float u, v;
};
