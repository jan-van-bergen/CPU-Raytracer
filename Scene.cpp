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
				RayHit hit = spheres[k].trace(ray, closest_hit.distance);

				if (hit.distance < closest_hit.distance) {
					closest_hit = hit;
				}
			}

			// Trace planes
			for (int k = 0; k < plane_count; k++) {
				RayHit hit = planes[k].trace(ray, closest_hit.distance);

				if (hit.distance < closest_hit.distance) {
					closest_hit = hit;
				}
			}

			if (closest_hit.distance < INFINITY) {
				window.plot(i, j, 0xff0000);
			}
		}
	}
}
