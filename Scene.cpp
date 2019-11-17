#include "Scene.h"

Scene::Scene() : camera(110.0f), spheres(2), planes(1), meshes(1) {
	spheres[0].init(Vector3( 2.0f, 0.0f, 10.0f), 1.0f);
	spheres[1].init(Vector3(-2.0f, 0.0f, 10.0f), 1.0f);
	spheres[0].material.colour  = Vector3(1.0f, 1.0f, 0.0f);
	spheres[1].material.colour  = Vector3(0.0f, 1.0f, 1.0f);
	spheres[0].material.texture = Texture::load(DATA_PATH("Floor.png"));
	spheres[1].material.texture = Texture::load(DATA_PATH("Floor.png"));

	planes[0].init(Vector3(0.0f, 1.0f, 0.0f), 1.0f);
	planes[0].material.texture = Texture::load(DATA_PATH("Floor.png"));

	meshes[0].init(DATA_PATH("Cube.obj"));
	meshes[0].transform.position.y = 2.0f;
	meshes[0].transform.rotation = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), 0.25f * PI);
	meshes[0].material.texture = Texture::load(DATA_PATH("Floor.png"));

	point_lights = new PointLight[point_light_count = 1] {
		PointLight(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 8.0f))
	};

	spot_lights = new SpotLight[spot_light_count = 1] {
		SpotLight(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 12.0f), Vector3(0.0f, 0.0f, -1.0f), 0.5f)
	};

	directional_lights = new DirectionalLight[directional_light_count = 1] {
		DirectionalLight(Vector3(0.3f), Vector3::normalize(Vector3(0.0f, -1.0f, 1.0f)))
	};
}

Scene::~Scene() {
	delete[] point_lights;
	delete[] spot_lights;
	delete[] directional_lights;
}

void Scene::trace_primitives(const Ray & ray, RayHit & ray_hit) const {
	spheres.trace(ray, ray_hit);
	planes.trace(ray, ray_hit);
	meshes.trace(ray, ray_hit);
}

bool Scene::intersect_primitives(const Ray & ray, float max_distance) const {
	if (spheres.intersect(ray, max_distance)) return true;
	if (planes.intersect (ray, max_distance)) return true;
	if (meshes.intersect (ray, max_distance)) return true;

	return false;
}

void Scene::update(float delta) {
	meshes[0].transform.rotation = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), delta) * meshes[0].transform.rotation;

	spheres.update();
	planes.update();
	meshes.update();
}

void Scene::render(const Window & window) const {
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

			// Check Point Lights
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

			// Check Spot Lights
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

			// Check Directional Lights
			for (int i = 0; i < directional_light_count; i++) {			
				ray.direction = directional_lights[i].direction;

				if (!intersect_primitives(ray, INFINITY)) {
					colour += directional_lights[i].calc_lighting(closest_hit.normal, to_camera);
				}
			}

			window.plot(x, y, colour * closest_hit.material->get_colour(closest_hit.u, closest_hit.v));
		}
	}
}
