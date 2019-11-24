#include "Scene.h"

#include "Test.h"

#define NUMBER_OF_BOUNCES 2

Scene::Scene() : camera(110.0f), spheres(2), planes(1), meshes(1) {
	spheres[0].init(1.0f);
	spheres[1].init(1.0f);
	spheres[0].transform.position = Vector3(-2.0f, 0.0f, 10.0f);
	spheres[1].transform.position = Vector3(+2.0f, 0.0f, 10.0f);
	spheres[0].material.colour = Vector3(1.0f, 1.0f, 0.0f);
	spheres[1].material.colour = Vector3(0.0f, 1.0f, 1.0f);
	spheres[0].material.reflectiveness = 0.0f;
	spheres[1].material.reflectiveness = 0.0f;
	spheres[0].material.refractiveness = 0.0f;
	spheres[1].material.refractiveness = 0.0f;
	spheres[0].material.refractive_index = 1.33f;
	spheres[1].material.refractive_index = 1.68f;

	planes[0].transform.position.y = -1.0f;
	planes[0].transform.rotation   = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), 0.25f * PI);
	planes[0].material.texture        = Texture::load(DATA_PATH("Floor.png"));
	planes[0].material.reflectiveness = 1.0f;

	meshes[0].init(DATA_PATH("Cube.obj"));
	meshes[0].transform.position.y = 2.0f;
	meshes[0].transform.rotation   = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), 0.25f * PI);
	//meshes[0].material.texture = Texture::load(DATA_PATH("Floor.png"));
	meshes[0].material.reflectiveness   = 1.0f;
	meshes[0].material.refractiveness   = 1.0f;
	meshes[0].material.refractive_index = 2.4;
	meshes[0].material.absorbtion       = Vector3(0.1f, 0.5f, 0.1f);

	point_lights = new PointLight[point_light_count = 1] {
		PointLight(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 8.0f))
	};

	spot_lights = new SpotLight[spot_light_count = 1] {
		SpotLight(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 10.0f), Quaternion::axis_angle(Vector3(1.0f, 0.0f, 0.0f), DEG_TO_RAD(70.0f)) * Vector3(0.0f, 0.0f, 1.0f), 70.0f, 80.0f)
	};

	directional_lights = new DirectionalLight[directional_light_count = 1] {
		DirectionalLight(Vector3(0.3f), Vector3::normalize(Vector3(0.0f, -1.0f, 1.0f)))
	};

	camera.position = Vector3(0.0f, 2.0f, -2.0f);
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

Scene::BounceResult Scene::bounce(const Ray & ray, int bounces_left) const {
	BounceResult result;

	RayHit closest_hit;
	trace_primitives(ray, closest_hit);

	// If the Ray hit nothing, leave the pixel black
	if (!closest_hit.hit) return result;
	
	result.colour   = ambient_lighting;
	result.distance = closest_hit.distance;
	
	// Secondary Ray starts at hit location
	Ray secondary_ray;
	secondary_ray.origin = closest_hit.point;

	Vector3 to_camera = Vector3::normalize(camera.position - closest_hit.point);

	// Check Point Lights
	for (int i = 0; i < point_light_count; i++) {
		Vector3 to_light = point_lights[i].position - closest_hit.point;
		float distance_to_light_squared = Vector3::length_squared(to_light);
		float distance_to_light         = sqrtf(distance_to_light_squared);

		to_light /= distance_to_light;
		secondary_ray.direction = to_light;

		if (!intersect_primitives(secondary_ray, distance_to_light)) {
			result.colour += point_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared);
		}
	}

	// Check Spot Lights
	for (int i = 0; i < spot_light_count; i++) {
		Vector3 to_light = spot_lights[i].position - closest_hit.point;
		float distance_to_light_squared = Vector3::length_squared(to_light);
		float distance_to_light         = sqrtf(distance_to_light_squared);

		to_light /= distance_to_light;
		secondary_ray.direction = to_light;

		if (!intersect_primitives(secondary_ray, distance_to_light)) {
			result.colour += spot_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared);
		}
	}

	// Check Directional Lights
	for (int i = 0; i < directional_light_count; i++) {			
		secondary_ray.direction = directional_lights[i].negative_direction;

		if (!intersect_primitives(secondary_ray, INFINITY)) {
			result.colour += directional_lights[i].calc_lighting(closest_hit.normal, to_camera);
		}
	}

	// If we have bounces left to do, recurse one level deeper
	if (bounces_left > 0) {	
		Vector3 colour_reflection;
		Vector3 colour_refraction;

		if (closest_hit.material->reflectiveness > 0.0f) {
			Ray reflected_ray;
			reflected_ray.origin    = closest_hit.point;
			reflected_ray.direction = Math3d::reflect(ray.direction, closest_hit.normal);

			colour_reflection = closest_hit.material->reflectiveness * bounce(reflected_ray, bounces_left - 1).colour;
		}

		if (closest_hit.material->refractiveness > 0.0f) {		
			Vector3 normal;
			float cos_theta;

			float n_1;
			float n_2;

			float dot = Vector3::dot(ray.direction, closest_hit.normal);
			if (dot < 0.0f) { // Entering material		
				n_1 = Material::AIR_REFRACTIVE_INDEX;
				n_2 = closest_hit.material->refractive_index;

				normal    =  closest_hit.normal;
				cos_theta = -dot;
			} else { // Leaving material
				n_1 = closest_hit.material->refractive_index;
				n_2 = Material::AIR_REFRACTIVE_INDEX;

				normal    = -closest_hit.normal;
				cos_theta =  dot;
			}

			float eta = n_1 / n_2;
			float k = 1.0f - eta*eta * (1.0f - cos_theta*cos_theta);
			
			// In case of Total Internal Reflection, return only the reflection component
			if (k < 0.0f) {
				result.colour = (result.colour + colour_reflection) * closest_hit.material->get_colour(closest_hit.u, closest_hit.v);
				return result;
			}

			Ray refracted_ray;
			refracted_ray.origin    = closest_hit.point;
			refracted_ray.direction = Math3d::refract(ray.direction, normal, eta, cos_theta, k);

			assert(test_refraction(n_1, n_2, ray.direction, normal, refracted_ray.direction));

			BounceResult refraction_result = bounce(refracted_ray, bounces_left - 1);
			colour_refraction = closest_hit.material->refractiveness * refraction_result.colour;

			// Apply Beer's Law
			colour_refraction.x *= expf(-closest_hit.material->absorbtion.x * refraction_result.distance);
			colour_refraction.y *= expf(-closest_hit.material->absorbtion.y * refraction_result.distance);
			colour_refraction.z *= expf(-closest_hit.material->absorbtion.z * refraction_result.distance);

			// Use Schlick's Approximation to simulate the Fresnel effect
			float r_0 = (n_1 - n_2) / (n_1 + n_2);
			r_0 *= r_0;

			if (n_1 > n_2) {
				cos_theta = -Vector3::dot(refracted_ray.direction, normal);
			}

			float one_minus_cos         = 1.0f - cos_theta;
			float one_minus_cos_squared = one_minus_cos * one_minus_cos;

			float F_r = r_0 + ((1.0f - r_0) * one_minus_cos_squared) * (one_minus_cos_squared * one_minus_cos); // r_0 + (1 - r_0) * (1 - cos)^5
			float F_t = 1.0f - F_r;

			result.colour += F_r * colour_reflection + F_t * colour_refraction;
		} else {
			result.colour += colour_reflection;
		}
	}

	result.colour *= closest_hit.material->get_colour(closest_hit.u, closest_hit.v);
	return result; 
}

void Scene::update(float delta) {
	camera.update(delta, SDL_GetKeyboardState(0));

	meshes[0].transform.rotation = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), delta) * meshes[0].transform.rotation;

	spheres.update();
	planes.update();
	meshes.update();
}

void Scene::render_tile(const Window & window, int x, int y) const {
	for (int j = y; j < y + window.tile_height; j++) {
		for (int i = x; i < x + window.tile_width; i++) {
			Ray ray = camera.get_ray(float(i), float(j));

			window.plot(i, j, bounce(ray, NUMBER_OF_BOUNCES).colour);
		}
	}
}
