#pragma once
#include "Vector3.h"
#include "Material.h"

struct RayHit {
	bool  hit      = false;
	float distance = INFINITY;

	Vector3 point;  // Coordinates of the hit in World Space
	Vector3 normal; // Normal      of the hit in World Space

	const Material * material;
	float u, v;
};
