#pragma once
#include "Primitives.h"
#include "Camera.h"

struct Scene {
	Sphere * spheres      = nullptr;
	int      sphere_count = 0;

	Plane * planes      = nullptr;
	int     plane_count = 0;

	Camera camera;

	void init();
};
