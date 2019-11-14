#include "Scene.h"

Scene::Scene() : camera(110.0f) {
	spheres = new Sphere[sphere_count = 2] {
		Sphere(Vector3( 2.0f, 0.0f, 10.0f), 1.0f),
		Sphere(Vector3(-2.0f, 0.0f, 10.0f), 1.0f)
	};

	planes = new Plane[plane_count = 1] {
		Plane(Vector3(0.0f, 1.0f, 0.0f), 1.0f)
	};
}

void Scene::trace(Window & window) const {
	for (int j = 0; j < window.height; j++) {
		for (int i = 0; i < window.width; i++) {
			Ray ray = camera.get_ray(i, j);

			RayHit closest_hit;

			// Trace spheres
			for (int k = 0; k < sphere_count; k++) {
				spheres[k].trace(ray, closest_hit);
			}

			// Trace planes
			for (int k = 0; k < plane_count; k++) {
				planes[k].trace(ray, closest_hit);
			}

			if (closest_hit.hit) {
				window.plot(i, j, 0xff0000);
			}
		}
	}
}
