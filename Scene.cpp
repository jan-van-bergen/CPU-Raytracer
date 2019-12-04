#include "Scene.h"

#include "Test.h"

#define NUMBER_OF_BOUNCES 3

#define SCENE_TEST    0
#define SCENE_WHITTED 1

#define CURRENT_SCENE SCENE_WHITTED

#if CURRENT_SCENE == SCENE_TEST
Scene::Scene() : camera(110.0f), spheres(2), planes(1), meshes(1), skybox(DATA_PATH("Sky_Probes/rnl_probe.float"), 900, 900) {
	spheres[0].init(1.0f);
	spheres[1].init(1.0f);
	spheres[0].transform.position = Vector3(-2.0f, 0.0f, 10.0f);
	spheres[1].transform.position = Vector3(+2.0f, 0.0f, 10.0f);
	spheres[0].material.diffuse = Vector3(1.0f, 1.0f, 0.0f);
	spheres[1].material.diffuse = Vector3(0.0f, 1.0f, 1.0f);
	spheres[0].material.reflection = 0.2f;
	spheres[1].material.reflection = 0.2f;
	spheres[0].material.transmittance = 0.6f;
	spheres[1].material.transmittance = 0.6f;
	spheres[0].material.index_of_refraction = 1.33f;
	spheres[1].material.index_of_refraction = 1.68f;

	planes[0].transform.position.y = -1.0f;
	planes[0].transform.rotation   = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), 0.25f * PI);
	planes[0].material.texture    = Texture::load(DATA_PATH("Floor.png"));
	planes[0].material.reflection = 0.5f;

	meshes[0].init(DATA_PATH("Cube.obj"));
	meshes[0].transform.position.y = 2.0f;
	meshes[0].transform.rotation   = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), 0.25f * PI);
	
	point_lights = new PointLight[point_light_count = 1] {
		PointLight(Vector3(10.0f, 0.0f, 10.0f), Vector3(0.0f, 0.0f, 6.0f))
	};

	spot_lights = new SpotLight[spot_light_count = 1] {
		SpotLight(Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 10.0f), Quaternion::axis_angle(Vector3(1.0f, 0.0f, 0.0f), DEG_TO_RAD(70.0f)) * Vector3(0.0f, 0.0f, 1.0f), 70.0f, 80.0f)
	};

	directional_lights = new DirectionalLight[directional_light_count = 1] {
		DirectionalLight(Vector3(0.3f), Vector3::normalize(Vector3(0.0f, -1.0f, 1.0f)))
	};

	camera.position = Vector3(0.0f, 2.0f, -2.0f);
#elif CURRENT_SCENE == SCENE_WHITTED
Scene::Scene() : camera(110.0f), spheres(2), planes(0), meshes(1), skybox(DATA_PATH("Sky_Probes/rnl_probe.float"), 900, 900) {
	spheres[0].init(1.0f);
	spheres[0].transform.position = Vector3(0.0f, 0.0f, 0.0f);
	spheres[0].material.diffuse = 1.0f;
	spheres[0].material.reflection = 1.0f;
	spheres[0].material.transmittance = 0.0f;
	spheres[0].material.index_of_refraction = 1.0f;

	spheres[1].init(1.0f);
	spheres[1].transform.position = Vector3(+2.0f, 1.0f, -2.0f);
	spheres[1].material.diffuse = 0.0f;
	spheres[1].material.reflection = 0.1f;
	spheres[1].material.transmittance = 0.9f;
	spheres[1].material.index_of_refraction = 1.1f;

	//planes[0].transform.position.y = -1.0f;
	//planes[0].material.texture    = Texture::load(DATA_PATH("Floor.png"));
	//planes[0].material.reflection = 0.2f;

	meshes[0].init(DATA_PATH("Plane.obj"));
	meshes[0].transform.position.y = -1.0f;

	point_light_count = 0;
	spot_light_count  = 0;

	directional_lights = new DirectionalLight[directional_light_count = 1] {
		DirectionalLight(Vector3(0.3f), Vector3(0.0f, -1.0f, 0.0f))
	};

	camera.position = Vector3(1.0f, 2.0f, -6.0f);
#endif
}

Scene::~Scene() {
	delete[] point_lights;
	delete[] spot_lights;
	delete[] directional_lights;
}

void Scene::trace_primitives(const Ray & ray, RayHit & ray_hit) const {
	spheres.trace(ray, ray_hit);
	planes.trace (ray, ray_hit);
	meshes.trace (ray, ray_hit);
}

SIMD_float Scene::intersect_primitives(const Ray & ray, SIMD_float max_distance) const {
	SIMD_float result(0.0f);

	result = spheres.intersect(ray, max_distance);
	if (SIMD_float::all_true(result)) return result;

	result = result | planes.intersect (ray, max_distance);
	if (SIMD_float::all_true(result)) return result;

	result = result | meshes.intersect (ray, max_distance);
	return result;
}

SIMD_Vector3 Scene::bounce(const Ray & ray, int bounces_left, SIMD_float & distance) const {
	SIMD_Vector3 result;
	
	const SIMD_float zero(0.0f);
	const SIMD_float one (1.0f);
	const SIMD_float inf (INFINITY);

	RayHit closest_hit;
	trace_primitives(ray, closest_hit);

	// If any of the Rays did not hit
	if (!SIMD_float::all_true(closest_hit.hit)) {
		result   = SIMD_Vector3::blend(skybox.sample(ray.direction), result, closest_hit.hit);
		distance = inf;

		// If none of the Rays hit, early out
		if (SIMD_float::all_false(closest_hit.hit)) return result;
	}

	distance = SIMD_float::blend(distance, closest_hit.distance, closest_hit.hit);
	
	alignas(SIMD_float) float us[SIMD_LANE_SIZE]; SIMD_float::store(us, closest_hit.u);
	alignas(SIMD_float) float vs[SIMD_LANE_SIZE]; SIMD_float::store(vs, closest_hit.v);
	
	SIMD_Vector3 material_diffuse;
	
	int hit_mask = SIMD_float::mask(closest_hit.hit);
	
	for (int i = 0; i < SIMD_LANE_SIZE; i++) {
		if (hit_mask & (1 << i)) {
			Vector3 diffuse = closest_hit.material[i]->get_colour(us[i], vs[i]);

			material_diffuse.x[i] = diffuse.x;
			material_diffuse.y[i] = diffuse.y;
			material_diffuse.z[i] = diffuse.z;
		} else {
			closest_hit.material[i] = &Material::default_material;

			material_diffuse.x[i] = 0.0f;
			material_diffuse.y[i] = 0.0f;
			material_diffuse.z[i] = 0.0f;
		}
	}

	SIMD_float diffuse_mask = SIMD_Vector3::length_squared(material_diffuse) > zero;

	if (!SIMD_float::all_false(diffuse_mask)) {
		SIMD_Vector3 diffuse = SIMD_Vector3(ambient_lighting);

		// Secondary Ray starts at hit location
		Ray secondary_ray;
		secondary_ray.origin = closest_hit.point;

		SIMD_Vector3 to_camera = SIMD_Vector3::normalize(SIMD_Vector3(camera.position) - closest_hit.point);

		// Check Point Lights
		for (int i = 0; i < point_light_count; i++) {
			SIMD_Vector3 to_light = SIMD_Vector3(point_lights[i].position) - closest_hit.point;
			SIMD_float   distance_to_light_squared = SIMD_Vector3::length_squared(to_light);
			SIMD_float   distance_to_light         = SIMD_float::sqrt(distance_to_light_squared);

			to_light /= distance_to_light;
			secondary_ray.direction = to_light;

			SIMD_float shadow_mask = intersect_primitives(secondary_ray, distance_to_light);
			diffuse = SIMD_Vector3::blend(diffuse + point_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared), diffuse, shadow_mask);
		}

		// Check Spot Lights
		for (int i = 0; i < spot_light_count; i++) {
			SIMD_Vector3 to_light = SIMD_Vector3(spot_lights[i].position) - closest_hit.point;
			SIMD_float   distance_to_light_squared = SIMD_Vector3::length_squared(to_light);
			SIMD_float   distance_to_light         = SIMD_float::sqrt(distance_to_light_squared);

			to_light /= distance_to_light;
			secondary_ray.direction = to_light;

			SIMD_float shadow_mask = intersect_primitives(secondary_ray, distance_to_light);
			diffuse = SIMD_Vector3::blend(diffuse + spot_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared), diffuse, shadow_mask);
		}

		// Check Directional Lights
		for (int i = 0; i < directional_light_count; i++) {			
			secondary_ray.direction = SIMD_Vector3(directional_lights[i].negative_direction);

			SIMD_float shadow_mask = intersect_primitives(secondary_ray, inf);
			diffuse = SIMD_Vector3::blend(diffuse + directional_lights[i].calc_lighting(closest_hit.normal, to_camera), diffuse, shadow_mask);
		}

		result += diffuse * material_diffuse;
	}

	// If we have bounces left to do, recurse one level deeper
	if (bounces_left > 0) {	
		SIMD_Vector3 colour_reflection;
		SIMD_Vector3 colour_refraction;

#if SIMD_LANE_SIZE == 4
		SIMD_Vector3 material_reflection(
			closest_hit.material[3]->reflection,
			closest_hit.material[2]->reflection,
			closest_hit.material[1]->reflection,
			closest_hit.material[0]->reflection
		);
		SIMD_Vector3 material_transmittance(
			closest_hit.material[3]->transmittance, 
			closest_hit.material[2]->transmittance, 
			closest_hit.material[1]->transmittance, 
			closest_hit.material[0]->transmittance
		);
#elif SIMD_LANE_SIZE == 8
		SIMD_Vector3 material_reflection(
			closest_hit.material[7]->reflection,
			closest_hit.material[6]->reflection,
			closest_hit.material[5]->reflection,
			closest_hit.material[4]->reflection,
			closest_hit.material[3]->reflection,
			closest_hit.material[2]->reflection,
			closest_hit.material[1]->reflection,
			closest_hit.material[0]->reflection
		);
		SIMD_Vector3 material_transmittance(
			closest_hit.material[7]->transmittance,
			closest_hit.material[6]->transmittance,
			closest_hit.material[5]->transmittance,
			closest_hit.material[4]->transmittance,
			closest_hit.material[3]->transmittance,
			closest_hit.material[2]->transmittance,
			closest_hit.material[1]->transmittance,
			closest_hit.material[0]->transmittance
		);
#endif
		SIMD_float reflection_mask = SIMD_Vector3::length_squared(material_reflection)    > zero;
		SIMD_float refraction_mask = SIMD_Vector3::length_squared(material_transmittance) > zero;
		
		if (!SIMD_float::all_false(reflection_mask)) {
			Ray reflected_ray;
			reflected_ray.origin    = closest_hit.point;
			reflected_ray.direction = Math::reflect(ray.direction, closest_hit.normal);

			SIMD_float reflection_distance;
			colour_reflection = material_reflection * bounce(reflected_ray, bounces_left - 1, reflection_distance) * material_diffuse;
		}

		if (!SIMD_float::all_false(refraction_mask)) {		
			SIMD_float dot      = SIMD_Vector3::dot(ray.direction, closest_hit.normal);
			SIMD_float dot_mask = dot < zero;

			SIMD_float air(Material::air_index_of_refraction);
#if SIMD_LANE_SIZE == 4
			SIMD_float ior(
				closest_hit.material[3]->index_of_refraction,
				closest_hit.material[2]->index_of_refraction,
				closest_hit.material[1]->index_of_refraction,
				closest_hit.material[0]->index_of_refraction
			);
#elif SIMD_LANE_SIZE == 8
			SIMD_float ior(
				closest_hit.material[7]->index_of_refraction,
				closest_hit.material[6]->index_of_refraction,
				closest_hit.material[5]->index_of_refraction,
				closest_hit.material[4]->index_of_refraction,
				closest_hit.material[3]->index_of_refraction,
				closest_hit.material[2]->index_of_refraction,
				closest_hit.material[1]->index_of_refraction,
				closest_hit.material[0]->index_of_refraction
			);
#endif
			SIMD_float n_1 = SIMD_float::blend(ior, air, dot_mask);
			SIMD_float n_2 = SIMD_float::blend(air, ior, dot_mask);

			SIMD_float   cos_theta = SIMD_float::blend(dot, zero - dot, dot_mask);
			SIMD_Vector3 normal    = SIMD_Vector3::blend(-closest_hit.normal, closest_hit.normal, dot_mask);

			SIMD_float eta = n_1 / n_2;
			SIMD_float k = (one - (eta*eta * (one - (cos_theta * cos_theta))));
			
			// In case of Total Internal Reflection, return only the reflection component
			SIMD_float tir_mask = closest_hit.hit & (k < zero);

			if (SIMD_float::mask(tir_mask) == SIMD_float::mask(refraction_mask)) {
				return SIMD_Vector3::blend(result, result + colour_reflection, reflection_mask);
			}

			Ray refracted_ray;
			refracted_ray.origin    = closest_hit.point;
			refracted_ray.direction = Math::refract(ray.direction, normal, eta, cos_theta, k);

			// Make sure that Snell's Law is correctly obeyed
			assert(Test::test_refraction(n_1, n_2, ray.direction, normal, refracted_ray.direction, closest_hit.hit & (k >= zero)));

			SIMD_float refraction_distance;
			colour_refraction = bounce(refracted_ray, bounces_left - 1, refraction_distance);

			// Apply Beer's Law
#if SIMD_LANE_SIZE == 4
			SIMD_Vector3 material_transmittance(
				closest_hit.material[3] ? closest_hit.material[3]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[2] ? closest_hit.material[2]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[1] ? closest_hit.material[1]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[0] ? closest_hit.material[0]->transmittance - Vector3(1.0f) : Vector3(0.0f)
			);
#elif SIMD_LANE_SIZE == 8
			SIMD_Vector3 material_transmittance(
				closest_hit.material[7] ? closest_hit.material[7]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[6] ? closest_hit.material[6]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[5] ? closest_hit.material[5]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[4] ? closest_hit.material[4]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[3] ? closest_hit.material[3]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[2] ? closest_hit.material[2]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[1] ? closest_hit.material[1]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[0] ? closest_hit.material[0]->transmittance - Vector3(1.0f) : Vector3(0.0f)
			);
#endif
			SIMD_float beer_x = SIMD_float::exp(material_transmittance.x * refraction_distance);
			SIMD_float beer_y = SIMD_float::exp(material_transmittance.y * refraction_distance);
			SIMD_float beer_z = SIMD_float::exp(material_transmittance.z * refraction_distance);
			
			colour_refraction.x = SIMD_float::blend(colour_refraction.x, colour_refraction.x * beer_x, dot_mask);
			colour_refraction.y = SIMD_float::blend(colour_refraction.y, colour_refraction.y * beer_y, dot_mask);
			colour_refraction.z = SIMD_float::blend(colour_refraction.z, colour_refraction.z * beer_z, dot_mask);

			// Use Schlick's Approximation to simulate the Fresnel effect
			SIMD_float r_0 = (n_1 - n_2) / (n_1 + n_2);
			r_0 = r_0 * r_0;

			// In case n_1 is larger than n_2, theta should be the angle
			// between the normal and the refracted Ray direction
			cos_theta = SIMD_float::blend(cos_theta, zero - SIMD_Vector3::dot(refracted_ray.direction, normal), n_1 > n_2);

			// Calculate (1 - cos(theta))^5 efficiently, without using pow
			SIMD_float one_minus_cos         = one - cos_theta;
			SIMD_float one_minus_cos_squared = one_minus_cos * one_minus_cos;

			SIMD_float F_r = r_0 + ((one - r_0) * one_minus_cos_squared) * (one_minus_cos_squared * one_minus_cos); // r_0 + (1 - r_0) * (1 - cos)^5
			SIMD_float F_t = one - F_r;

			SIMD_Vector3 blend = SIMD_Vector3::blend(F_r * colour_reflection + F_t * colour_refraction, colour_reflection, tir_mask);

			return SIMD_Vector3::blend(result, result + blend, refraction_mask);
		} else {
			return SIMD_Vector3::blend(result, result + colour_reflection, reflection_mask);
		}
	}

	return result; 
}

void Scene::update(float delta) {
	camera.update(delta, SDL_GetKeyboardState(0));

	//meshes[0].transform.rotation = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), delta) * meshes[0].transform.rotation;

	spheres.update();
	planes.update();
	meshes.update();
}

void Scene::render_tile(const Window & window, int x, int y) const {
	Ray ray;
	ray.origin.x = SIMD_float(camera.position.x);
	ray.origin.y = SIMD_float(camera.position.y);
	ray.origin.z = SIMD_float(camera.position.z);

	SIMD_Vector3 camera_top_left_corner_rotated(camera.top_left_corner_rotated);
	SIMD_Vector3 camera_x_axis_rotated         (camera.x_axis_rotated);
	SIMD_Vector3 camera_y_axis_rotated         (camera.y_axis_rotated);

	for (int j = y; j < y + window.tile_height; j += 2) {
		for (int i = x; i < x + window.tile_width; i += SIMD_LANE_SIZE / 2) {
			float i_f = float(i);
			float j_f = float(j);

#if (SIMD_LANE_SIZE == 4) 
			SIMD_float is(i_f, i_f + 1.0f, i_f,        i_f + 1.0f);
			SIMD_float js(j_f, j_f,        j_f + 1.0f, j_f + 1.0f);
#elif (SIMD_LANE_SIZE == 8)
			SIMD_float is(i_f, i_f + 1.0f, i_f + 2.0f, i_f + 3.0f, i_f,        i_f + 1.0f, i_f + 2.0f, i_f + 3.0f);
			SIMD_float js(j_f, j_f,        j_f,        j_f,        j_f + 1.0f, j_f + 1.0f, j_f + 1.0f, j_f + 1.0f);
#endif
			ray.direction = SIMD_Vector3::normalize(
				camera_top_left_corner_rotated
				+ is * camera_x_axis_rotated
				+ js * camera_y_axis_rotated
			);
			
			SIMD_float   distance;
			SIMD_Vector3 colour = bounce(ray, NUMBER_OF_BOUNCES, distance);

			alignas(SIMD_float) float xs[SIMD_LANE_SIZE]; SIMD_float::store(xs, colour.x);
			alignas(SIMD_float) float ys[SIMD_LANE_SIZE]; SIMD_float::store(ys, colour.y);
			alignas(SIMD_float) float zs[SIMD_LANE_SIZE]; SIMD_float::store(zs, colour.z);

#if (SIMD_LANE_SIZE == 4) 
			window.plot(i,     j,     Vector3(xs[3], ys[3], zs[3]));
			window.plot(i + 1, j,     Vector3(xs[2], ys[2], zs[2]));
			window.plot(i,     j + 1, Vector3(xs[1], ys[1], zs[1]));
			window.plot(i + 1, j + 1, Vector3(xs[0], ys[0], zs[0]));
#elif (SIMD_LANE_SIZE == 8)
			window.plot(i,     j,     Vector3(xs[7], ys[7], zs[7]));
			window.plot(i + 1, j,     Vector3(xs[6], ys[6], zs[6]));
			window.plot(i + 2, j,     Vector3(xs[5], ys[5], zs[5]));
			window.plot(i + 3, j,     Vector3(xs[4], ys[4], zs[4]));
			window.plot(i,     j + 1, Vector3(xs[3], ys[3], zs[3]));
			window.plot(i + 1, j + 1, Vector3(xs[2], ys[2], zs[2]));
			window.plot(i + 2, j + 1, Vector3(xs[1], ys[1], zs[1]));
			window.plot(i + 3, j + 1, Vector3(xs[0], ys[0], zs[0]));
#endif
		}
	}
}
