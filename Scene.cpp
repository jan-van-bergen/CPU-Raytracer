#include "Scene.h"

Scene::Scene() : camera(110.0f) {
	spheres = new Sphere[sphere_count = 2] {
		Sphere(Vector3( 2.0f, 0.0f, 10.0f), 1.0f),
		Sphere(Vector3(-2.0f, 0.0f, 10.0f), 1.0f)
	};
	spheres[0].material.colour = Vector3(1.0f, 1.0f, 0.0f);
	spheres[1].material.colour = Vector3(0.0f, 1.0f, 1.0f);

	planes = new Plane[plane_count = 1] {
		Plane(Vector3(0.0f, 1.0f, 0.0f), 1.0f)
	};

	point_lights = new PointLight[point_light_count = 1] {
		PointLight(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 8.0f))
	};

	spot_lights = new SpotLight[spot_light_count = 1] {
		SpotLight(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 12.0f), Vector3(0.0f, 0.0f, -1.0f), 0.5f)
	};

	directional_lights = new DirectionalLight[directional_light_count = 1] {
		DirectionalLight(Vector3(0.7f), Vector3::normalize(Vector3(0.0f, -1.0f, 1.0f)))
	};
}

Scene::~Scene() {
	delete[] spheres;
	delete[] planes;

	delete[] point_lights;
	delete[] spot_lights;
	delete[] directional_lights;
}

void Scene::trace_primitives(const Ray & ray, RayHit & ray_hit) const {
	// Trace spheres
	for (int i = 0; i < sphere_count; i++) {
		spheres[i].trace(ray, ray_hit);
	}

	// Trace planes
	for (int i = 0; i < plane_count; i++) {
		planes[i].trace(ray, ray_hit);
	}
}

bool Scene::intersect_primitives(const Ray & ray, float max_distance) const {
	// Intersect spheres
	for (int i = 0; i < sphere_count; i++) {
		if (spheres[i].intersect(ray, max_distance)) {
			return true;
		}
	}

	// Intersect planes
	for (int i = 0; i < plane_count; i++) {
		if (planes[i].intersect(ray, max_distance)) {
			return true;
		}
	}

	return false;
}

void Scene::update(const Window & window) const {
	for (int y = 0; y < window.height; y++) {
		for (int x = 0; x < window.width; x++) {
			Ray ray = camera.get_ray(float(x), float(y));

			RayHit closest_hit;
			trace_primitives(ray, closest_hit);

			// If the Ray hit nothing, leave the pixel black
			if (!closest_hit.hit) continue;
			
			Vector3 colour = ambient_lighting;

			// Secondary Ray starts at hit location
			ray.origin = closest_hit.point;
			Vector3 to_camera = Vector3::normalize(camera.position - closest_hit.point);

			for (int i = 0; i < point_light_count; i++) {
				Vector3 to_light = point_lights[i].position - closest_hit.point;
				float distance_to_light_squared = Vector3::length_squared(to_light);
				float distance_to_light         = sqrtf(distance_to_light_squared);

				to_light /= distance_to_light;
				ray.direction = to_light;

				if (!intersect_primitives(ray, distance_to_light)) {
					colour += point_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared);
				}
			}

			for (int i = 0; i < spot_light_count; i++) {
				Vector3 to_light = spot_lights[i].position - closest_hit.point;
				float distance_to_light_squared = Vector3::length_squared(to_light);
				float distance_to_light         = sqrtf(distance_to_light_squared);

				to_light /= distance_to_light;
				ray.direction = to_light;

				if (!intersect_primitives(ray, distance_to_light)) {
					colour += spot_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared);
				}
			}

			for (int i = 0; i < directional_light_count; i++) {			
				ray.direction = directional_lights[i].direction;

				if (!intersect_primitives(ray, INFINITY)) {
					colour += directional_lights[i].calc_lighting(closest_hit.normal, to_camera);
				}
			}

			window.plot(x, y, colour * closest_hit.colour);
		}
	}
}
