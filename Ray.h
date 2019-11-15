#pragma once
#include "Vector3.h"

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

	Vector3 colour;
};
