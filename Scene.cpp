#include "Scene.h"

void Scene::init() {
	spheres = new Sphere[sphere_count = 2] {
		Sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f),
		Sphere(Vector3(0.0f, 0.0f, 0.0f), 1.0f)
	};

	planes = new Plane[plane_count = 3] {
		Plane { Vector3(0.0f, 0.0f, 0.0f), 1.0f },
		Plane { Vector3(0.0f, 0.0f, 0.0f), 1.0f },
		Plane { Vector3(0.0f, 0.0f, 0.0f), 1.0f }
	};
}
