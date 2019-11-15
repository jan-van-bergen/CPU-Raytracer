#pragma once
#include "Vector3.h"
#include "Material.h"

#define EPSILON 0.001f

struct Ray {
	Vector3 origin;
	Vector3 direction;
};

struct RayHit {
	bool  hit      = false;
	float distance = INFINITY;

	Vector3 point;
	Vector3 normal;

	const Material * material;
	float u, v;
};
