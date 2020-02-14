#include "Raytracer.h"

void Raytracer::render_tile(const Window & window, int x, int y, int tile_width, int tile_height) const {
	Ray ray;
	ray.origin.x = SIMD_float(scene->camera.position.x);
	ray.origin.y = SIMD_float(scene->camera.position.y);
	ray.origin.z = SIMD_float(scene->camera.position.z);
	
#if RAY_DIFFERENTIALS_ENABLED
	ray.dO_dx = SIMD_Vector3(0.0f);
	ray.dO_dy = SIMD_Vector3(0.0f);
#endif

#if SIMD_LANE_SIZE == 1
	const int step_x = 1;
	const int step_y = 1;
#elif SIMD_LANE_SIZE == 4
	const int step_x = 2;
	const int step_y = 2;
#elif SIMD_LANE_SIZE == 8
	const int step_x = 4;
	const int step_y = 2;
#endif

	assert(tile_width  % step_x == 0);
	assert(tile_height % step_y == 0);
	
	for (int j = y; j < y + tile_height; j += step_y) {
		for (int i = x; i < x + tile_width; i += step_x) {
			float i_f = float(i);
			float j_f = float(j);

			// Calulcate pixel coordinates for all pixels in the current Ray Packet
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

			SIMD_Vector3 direction = 
				SIMD_Vector3::madd(scene->camera.rotated_x_axis, is, 
				SIMD_Vector3::madd(scene->camera.rotated_y_axis, js, scene->camera.rotated_top_left_corner));

			SIMD_float          d_dot_d = SIMD_Vector3::dot(direction, direction);
			SIMD_float inv_sqrt_d_dot_d = SIMD_float::inv_sqrt(d_dot_d);

			SIMD_float denom = inv_sqrt_d_dot_d / d_dot_d; // d_dot_d ^ -3/2

#if RAY_DIFFERENTIALS_ENABLED
			ray.dD_dx = (d_dot_d * scene->camera.rotated_x_axis - SIMD_Vector3::dot(direction, scene->camera.rotated_x_axis) * direction) * denom;
			ray.dD_dy = (d_dot_d * scene->camera.rotated_y_axis - SIMD_Vector3::dot(direction, scene->camera.rotated_y_axis) * direction) * denom;
#endif

			ray.direction = direction * inv_sqrt_d_dot_d; // Normalize direction

			SIMD_float distance;
			SIMD_Vector3 colour = bounce(ray, NUMBER_OF_BOUNCES, distance);

#if SIMD_LANE_SIZE == 1
			window.plot(i, j, Vector3(colour.x[0], colour.y[0], colour.z[0]));
#elif SIMD_LANE_SIZE == 4
			window.plot(i,     j,     Vector3(colour.x[3], colour.y[3], colour.z[3]));
			window.plot(i + 1, j,     Vector3(colour.x[2], colour.y[2], colour.z[2]));
			window.plot(i,     j + 1, Vector3(colour.x[1], colour.y[1], colour.z[1]));
			window.plot(i + 1, j + 1, Vector3(colour.x[0], colour.y[0], colour.z[0]));
#elif SIMD_LANE_SIZE == 8
			window.plot(i,     j,     Vector3(colour.x[7], colour.y[7], colour.z[7]));
			window.plot(i + 1, j,     Vector3(colour.x[6], colour.y[6], colour.z[6]));
			window.plot(i + 2, j,     Vector3(colour.x[5], colour.y[5], colour.z[5]));
			window.plot(i + 3, j,     Vector3(colour.x[4], colour.y[4], colour.z[4]));
			window.plot(i,     j + 1, Vector3(colour.x[3], colour.y[3], colour.z[3]));
			window.plot(i + 1, j + 1, Vector3(colour.x[2], colour.y[2], colour.z[2]));
			window.plot(i + 2, j + 1, Vector3(colour.x[1], colour.y[1], colour.z[1]));
			window.plot(i + 3, j + 1, Vector3(colour.x[0], colour.y[0], colour.z[0]));
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

#if BVH_VISUALIZE_HEATMAP
	const float one_over_32  = 1.0f / 32.0f;
	const float one_over_256 = 1.0f / 256.0f;
	const float one_over_512 = 1.0f / 512.0f;
	return SIMD_Vector3(Vector3(closest_hit.bvh_steps * one_over_32, closest_hit.bvh_steps * one_over_256, closest_hit.bvh_steps * one_over_512));
#endif

	// If any of the Rays did not hit
	if (!SIMD_float::all_true(closest_hit.hit)) {
		result = SIMD_Vector3::blend(scene->sky.sample(ray.direction), result, closest_hit.hit);
		distance = inf;

		// If none of the Rays hit, early out
		if (SIMD_float::all_false(closest_hit.hit)) return result;
	}

	distance = SIMD_float::blend(distance, closest_hit.distance, closest_hit.hit);
	
	SIMD_Vector3 material_diffuse;
	
	int hit_mask = SIMD_float::mask(closest_hit.hit);
	
	for (int i = 0; i < SIMD_LANE_SIZE; i++) {
		if (hit_mask & (1 << i)) {
#if RAY_DIFFERENTIALS_ENABLED
			Vector3 diffuse = Material::materials[closest_hit.material_id[i]].get_albedo(
				closest_hit.u[i],     closest_hit.v[i], 
				closest_hit.ds_dx[i], closest_hit.ds_dy[i], 
				closest_hit.dt_dx[i], closest_hit.dt_dy[i]
			);
#else
			Vector3 diffuse = Material::materials[closest_hit.material_id[i]].get_albedo(closest_hit.u[i], closest_hit.v[i], 0.0f, 0.0f, 0.0f, 0.0f);
#endif

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
			SIMD_Vector3 to_light = scene->point_lights[i].position - closest_hit.point;
			SIMD_float   distance_to_light_squared = SIMD_Vector3::length_squared(to_light);
			SIMD_float   distance_to_light         = SIMD_float::sqrt(distance_to_light_squared);

			to_light /= distance_to_light;
			secondary_ray.direction = to_light;

			SIMD_float shadow_mask = scene->intersect_primitives(secondary_ray, distance_to_light);
			if (SIMD_float::all_true(shadow_mask)) continue;

			diffuse = SIMD_Vector3::blend(diffuse + scene->point_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared), diffuse, shadow_mask);
		}

		// Check Spot Lights
		for (int i = 0; i < scene->spot_light_count; i++) {
			SIMD_Vector3 to_light = scene->spot_lights[i].position - closest_hit.point;
			SIMD_float   distance_to_light_squared = SIMD_Vector3::length_squared(to_light);
			SIMD_float   distance_to_light         = SIMD_float::sqrt(distance_to_light_squared);

			to_light /= distance_to_light;
			secondary_ray.direction = to_light;

			SIMD_float shadow_mask = scene->intersect_primitives(secondary_ray, distance_to_light);
			if (SIMD_float::all_true(shadow_mask)) continue;

			diffuse = SIMD_Vector3::blend(diffuse + scene->spot_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared), diffuse, shadow_mask);
		}

		// Check Directional Lights
		for (int i = 0; i < scene->directional_light_count; i++) {			
			secondary_ray.direction = scene->directional_lights[i].negative_direction;

			SIMD_float shadow_mask = scene->intersect_primitives(secondary_ray, inf);
			if (SIMD_float::all_true(shadow_mask)) continue;

			diffuse = SIMD_Vector3::blend(diffuse + scene->directional_lights[i].calc_lighting(closest_hit.normal, to_camera), diffuse, shadow_mask);
		}

		result = SIMD_Vector3::madd(diffuse, material_diffuse, result);
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

#if RAY_DIFFERENTIALS_ENABLED
			reflected_ray.dO_dx = closest_hit.dO_dx;
			reflected_ray.dO_dy = closest_hit.dO_dy;

			SIMD_float dDN_dx = SIMD_Vector3::dot(ray.dD_dx, closest_hit.normal) + SIMD_Vector3::dot(ray.direction, closest_hit.dN_dx);
			SIMD_float dDN_dy = SIMD_Vector3::dot(ray.dD_dy, closest_hit.normal) + SIMD_Vector3::dot(ray.direction, closest_hit.dN_dy);

			reflected_ray.dD_dx = ray.dD_dx - SIMD_float(2.0f) * (SIMD_Vector3::dot(ray.direction, closest_hit.normal) * closest_hit.dN_dx + dDN_dx * closest_hit.normal);
			reflected_ray.dD_dy = ray.dD_dy - SIMD_float(2.0f) * (SIMD_Vector3::dot(ray.direction, closest_hit.normal) * closest_hit.dN_dy + dDN_dy * closest_hit.normal);
#endif

			SIMD_float reflection_distance;
			colour_reflection = material_reflection * bounce(reflected_ray, bounces_left - 1, reflection_distance);

			result = SIMD_Vector3::blend(result, result + colour_reflection, reflection_mask);
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
			SIMD_float k   = one - (eta*eta * (one - (cos_theta * cos_theta)));
			
			// In case of Total Internal Reflection, return only the reflection component
			SIMD_float tir_mask = closest_hit.hit & (k < zero);

			if (SIMD_float::mask(tir_mask) == SIMD_float::mask(refraction_mask)) {
				return SIMD_Vector3::blend(result, result + colour_reflection, reflection_mask & refraction_mask);
			}

			Ray refracted_ray;
			refracted_ray.origin    = closest_hit.point;
			refracted_ray.direction = Math::refract(ray.direction, normal, eta, cos_theta, k);

			// Make sure that Snell's Law is correctly obeyed
			assert(Debug::test_refraction(n_1, n_2, ray.direction, normal, refracted_ray.direction, closest_hit.hit & (k >= zero)));
			
#if RAY_DIFFERENTIALS_ENABLED
			refracted_ray.dO_dx = closest_hit.dO_dx;
			refracted_ray.dO_dy = closest_hit.dO_dy;

			SIMD_float dDN_dx = SIMD_Vector3::dot(ray.dD_dx, closest_hit.normal) + SIMD_Vector3::dot(ray.direction, closest_hit.dN_dx);
			SIMD_float dDN_dy = SIMD_Vector3::dot(ray.dD_dy, closest_hit.normal) + SIMD_Vector3::dot(ray.direction, closest_hit.dN_dy);

			SIMD_float D_dot_N      = -cos_theta;
			SIMD_float Dprime_dot_N = -SIMD_float::sqrt(k);

			SIMD_float mu = -(eta * cos_theta + Dprime_dot_N);

			SIMD_float factor = (eta + (eta*eta * cos_theta) / Dprime_dot_N);
			SIMD_float dmu_dx = factor * dDN_dx;
			SIMD_float dmu_dy = factor * dDN_dy;

			refracted_ray.dD_dx = eta * ray.dD_dx - (mu * D_dot_N + closest_hit.dN_dx * closest_hit.normal) * dDN_dx;
			refracted_ray.dD_dy = eta * ray.dD_dy - (mu * D_dot_N + closest_hit.dN_dy * closest_hit.normal) * dDN_dy;
#endif

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
		}
	}

	return result; 
}
