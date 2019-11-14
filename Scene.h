#pragma once
#include "Sphere.h"
#include "Plane.h"

#include "Camera.h"

#include "Window.h"

struct Scene {
	Sphere * spheres      = nullptr;
	int      sphere_count = 0;

	Plane * planes      = nullptr;
	int     plane_count = 0;

	Camera camera;

	Scene();

	void trace(Window & window) const;
};
