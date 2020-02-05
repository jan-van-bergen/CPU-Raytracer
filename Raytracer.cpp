#include "Raytracer.h"

#define NUMBER_OF_BOUNCES 3

#define BVH_HEATMAP false // Toggle to visualize number of traversal steps through BVH

void Raytracer::render_tile(const Window & window, int x, int y, int tile_width, int tile_height) const {
	Ray ray;
	ray.origin.x = SIMD_float(scene->camera.position.x);
	ray.origin.y = SIMD_float(scene->camera.position.y);
	ray.origin.z = SIMD_float(scene->camera.position.z);

	SIMD_Vector3 camera_top_left_corner_rotated(scene->camera.top_left_corner_rotated);
	SIMD_Vector3 camera_x_axis_rotated(scene->camera.x_axis_rotated);
	SIMD_Vector3 camera_y_axis_rotated(scene->camera.y_axis_rotated);
	
#if SIMD_LANE_SIZE == 1
	int step_y = 1;
	int step_x = 1;
#elif SIMD_LANE_SIZE == 4
	int step_y = 2;
	int step_x = 2;
#elif SIMD_LANE_SIZE == 8
	int step_y = 2;
	int step_x = 4;
#endif
	for (int j = y; j < y + tile_height; j += step_y) {
		for (int i = x; i < x + tile_width; i += step_x) {
			float i_f = float(i);
			float j_f = float(j);

#if SIMD_LANE_SIZE == 1
			SIMD_float is(i_f);
			SIMD_float js(j_f);
#elif SIMD_LANE_SIZE == 4
			SIMD_float is(i_f, i_f + 1.0f, i_f,        i_f + 1.0f);
			SIMD_float js(j_f, j_f,        j_f + 1.0f, j_f + 1.0f);
#elif SIMD_LANE_SIZE == 8
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

#if SIMD_LANE_SIZE == 1
			window.plot(i, j, Vector3(xs[0], ys[0], zs[0]));
#elif SIMD_LANE_SIZE == 4
			window.plot(i,     j,     Vector3(xs[3], ys[3], zs[3]));
			window.plot(i + 1, j,     Vector3(xs[2], ys[2], zs[2]));
			window.plot(i,     j + 1, Vector3(xs[1], ys[1], zs[1]));
			window.plot(i + 1, j + 1, Vector3(xs[0], ys[0], zs[0]));
#elif SIMD_LANE_SIZE == 8
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

SIMD_Vector3 Raytracer::bounce(const Ray & ray, int bounces_left, SIMD_float & distance) const {
	SIMD_Vector3 result;
	
	const SIMD_float zero(0.0f);
	const SIMD_float one (1.0f);
	const SIMD_float inf (INFINITY);

	RayHit closest_hit;
	scene->trace_primitives(ray, closest_hit);

#if BVH_HEATMAP
	SIMD_Vector3 debug;
	for (int i = 0; i < SIMD_LANE_SIZE; i++) {
		Vector3 colour = Debug::heat_palette->sample(Math::clamp(closest_hit.bvh_steps[i] / 32, 0.0f, 1.0f), 0.0f);

		debug.x[i] = colour.x;
		debug.y[i] = colour.y;
		debug.z[i] = colour.z;
	}
	return debug;
#endif

	// If any of the Rays did not hit
	if (!SIMD_float::all_true(closest_hit.hit)) {
		result = SIMD_Vector3::blend(scene->sky.sample(ray.direction), result, closest_hit.hit);
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
			Vector3 diffuse = Material::materials[closest_hit.material_id[i]].get_colour(us[i], vs[i]);

			material_diffuse.x[i] = diffuse.x;
			material_diffuse.y[i] = diffuse.y;
			material_diffuse.z[i] = diffuse.z;
		} else {
			closest_hit.material_id[i] = 0;

			material_diffuse.x[i] = 0.0f;
			material_diffuse.y[i] = 0.0f;
			material_diffuse.z[i] = 0.0f;
		}
	}

	SIMD_float diffuse_mask = SIMD_Vector3::length_squared(material_diffuse) > zero;

	if (!SIMD_float::all_false(diffuse_mask)) {
		SIMD_Vector3 diffuse = SIMD_Vector3(scene->ambient_lighting);

		// Secondary Ray starts at hit location
		Ray secondary_ray;
		secondary_ray.origin = closest_hit.point;

		SIMD_Vector3 to_camera = SIMD_Vector3::normalize(SIMD_Vector3(scene->camera.position) - closest_hit.point);

		// Check Point Lights
		for (int i = 0; i < scene->point_light_count; i++) {
			SIMD_Vector3 to_light = SIMD_Vector3(scene->point_lights[i].position) - closest_hit.point;
			SIMD_float   distance_to_light_squared = SIMD_Vector3::length_squared(to_light);
			SIMD_float   distance_to_light         = SIMD_float::sqrt(distance_to_light_squared);

			to_light /= distance_to_light;
			secondary_ray.direction = to_light;

			SIMD_float shadow_mask = scene->intersect_primitives(secondary_ray, distance_to_light);
			diffuse = SIMD_Vector3::blend(diffuse + scene->point_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared), diffuse, shadow_mask);
		}

		// Check Spot Lights
		for (int i = 0; i < scene->spot_light_count; i++) {
			SIMD_Vector3 to_light = SIMD_Vector3(scene->spot_lights[i].position) - closest_hit.point;
			SIMD_float   distance_to_light_squared = SIMD_Vector3::length_squared(to_light);
			SIMD_float   distance_to_light         = SIMD_float::sqrt(distance_to_light_squared);

			to_light /= distance_to_light;
			secondary_ray.direction = to_light;

			SIMD_float shadow_mask = scene->intersect_primitives(secondary_ray, distance_to_light);
			diffuse = SIMD_Vector3::blend(diffuse + scene->spot_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared), diffuse, shadow_mask);
		}

		// Check Directional Lights
		for (int i = 0; i < scene->directional_light_count; i++) {			
			secondary_ray.direction = SIMD_Vector3(scene->directional_lights[i].negative_direction);

			SIMD_float shadow_mask = scene->intersect_primitives(secondary_ray, inf);
			diffuse = SIMD_Vector3::blend(diffuse + scene->directional_lights[i].calc_lighting(closest_hit.normal, to_camera), diffuse, shadow_mask);
		}

		result += diffuse * material_diffuse;
	}
	
	// If we have bounces left to do, recurse one level deeper
	if (bounces_left > 0) {	
		SIMD_Vector3 colour_reflection;
		SIMD_Vector3 colour_refraction;

#if SIMD_LANE_SIZE == 1
		SIMD_Vector3 material_reflection   (Material::materials[closest_hit.material_id[0]].reflection);
		SIMD_Vector3 material_transmittance(Material::materials[closest_hit.material_id[0]].transmittance);
#elif SIMD_LANE_SIZE == 4
		SIMD_Vector3 material_reflection(
			Material::materials[closest_hit.material_id[3]].reflection,
			Material::materials[closest_hit.material_id[2]].reflection,
			Material::materials[closest_hit.material_id[1]].reflection,
			Material::materials[closest_hit.material_id[0]].reflection
		);
		SIMD_Vector3 material_transmittance(
			Material::materials[closest_hit.material_id[3]].transmittance, 
			Material::materials[closest_hit.material_id[2]].transmittance, 
			Material::materials[closest_hit.material_id[1]].transmittance, 
			Material::materials[closest_hit.material_id[0]].transmittance
		);
#elif SIMD_LANE_SIZE == 8
		SIMD_Vector3 material_reflection(
			Material::materials[closest_hit.material_id[7]].reflection,
			Material::materials[closest_hit.material_id[6]].reflection,
			Material::materials[closest_hit.material_id[5]].reflection,
			Material::materials[closest_hit.material_id[4]].reflection,
			Material::materials[closest_hit.material_id[3]].reflection,
			Material::materials[closest_hit.material_id[2]].reflection,
			Material::materials[closest_hit.material_id[1]].reflection,
			Material::materials[closest_hit.material_id[0]].reflection
		);
		SIMD_Vector3 material_transmittance(
			Material::materials[closest_hit.material_id[7]].transmittance,
			Material::materials[closest_hit.material_id[6]].transmittance,
			Material::materials[closest_hit.material_id[5]].transmittance,
			Material::materials[closest_hit.material_id[4]].transmittance,
			Material::materials[closest_hit.material_id[3]].transmittance,
			Material::materials[closest_hit.material_id[2]].transmittance,
			Material::materials[closest_hit.material_id[1]].transmittance,
			Material::materials[closest_hit.material_id[0]].transmittance
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
			
#if SIMD_LANE_SIZE == 1
			SIMD_float ior(Material::materials[closest_hit.material_id[0]].index_of_refraction);
#elif SIMD_LANE_SIZE == 4
			SIMD_float ior(
				Material::materials[closest_hit.material_id[3]].index_of_refraction,
				Material::materials[closest_hit.material_id[2]].index_of_refraction,
				Material::materials[closest_hit.material_id[1]].index_of_refraction,
				Material::materials[closest_hit.material_id[0]].index_of_refraction
			);
#elif SIMD_LANE_SIZE == 8
			SIMD_float ior(
				Material::materials[closest_hit.material_id[7]].index_of_refraction,
				Material::materials[closest_hit.material_id[6]].index_of_refraction,
				Material::materials[closest_hit.material_id[5]].index_of_refraction,
				Material::materials[closest_hit.material_id[4]].index_of_refraction,
				Material::materials[closest_hit.material_id[3]].index_of_refraction,
				Material::materials[closest_hit.material_id[2]].index_of_refraction,
				Material::materials[closest_hit.material_id[1]].index_of_refraction,
				Material::materials[closest_hit.material_id[0]].index_of_refraction
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
			assert(Debug::test_refraction(n_1, n_2, ray.direction, normal, refracted_ray.direction, closest_hit.hit & (k >= zero)));

			SIMD_float refraction_distance;
			colour_refraction = bounce(refracted_ray, bounces_left - 1, refraction_distance);

			// Apply Beer's Law
#if SIMD_LANE_SIZE == 1
			SIMD_Vector3 material_absorption(Material::materials[closest_hit.material_id[0]].transmittance - Vector3(1.0f));
#elif SIMD_LANE_SIZE == 4
			SIMD_Vector3 material_absorption(
				Material::materials[closest_hit.material_id[3]].transmittance - Vector3(1.0f),
				Material::materials[closest_hit.material_id[2]].transmittance - Vector3(1.0f),
				Material::materials[closest_hit.material_id[1]].transmittance - Vector3(1.0f),
				Material::materials[closest_hit.material_id[0]].transmittance - Vector3(1.0f)
			);
#elif SIMD_LANE_SIZE == 8
			SIMD_Vector3 material_absorption(
				Material::materials[closest_hit.material_id[7]].transmittance - Vector3(1.0f),
				Material::materials[closest_hit.material_id[6]].transmittance - Vector3(1.0f),
				Material::materials[closest_hit.material_id[5]].transmittance - Vector3(1.0f),
				Material::materials[closest_hit.material_id[4]].transmittance - Vector3(1.0f),
				Material::materials[closest_hit.material_id[3]].transmittance - Vector3(1.0f),
				Material::materials[closest_hit.material_id[2]].transmittance - Vector3(1.0f),
				Material::materials[closest_hit.material_id[1]].transmittance - Vector3(1.0f),
				Material::materials[closest_hit.material_id[0]].transmittance - Vector3(1.0f)
			);
#endif
			SIMD_float beer_x = SIMD_float::exp(material_absorption.x * refraction_distance);
			SIMD_float beer_y = SIMD_float::exp(material_absorption.y * refraction_distance);
			SIMD_float beer_z = SIMD_float::exp(material_absorption.z * refraction_distance);
			
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
