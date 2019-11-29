#include "Scene.h"

#include "Test.h"

#define NUMBER_OF_BOUNCES 2

Scene::Scene() : camera(110.0f), spheres(2), planes(1), meshes(1) {
	spheres[0].init(1.0f);
	spheres[1].init(1.0f);
	spheres[0].transform.position = Vector3(-2.0f, 0.0f, 10.0f);
	spheres[1].transform.position = Vector3(+2.0f, 0.0f, 10.0f);
	spheres[0].material.diffuse = Vector3(1.0f, 1.0f, 0.0f);
	spheres[1].material.diffuse = Vector3(0.0f, 1.0f, 1.0f);
	spheres[0].material.reflection = 0.2f;
	spheres[1].material.reflection = 0.2f;
	spheres[0].material.transmittance = 0.0f;
	spheres[1].material.transmittance = 0.0f;
	spheres[0].material.index_of_refraction = 1.33f;
	spheres[1].material.index_of_refraction = 1.68f;

	planes[0].transform.position.y = -1.0f;
	planes[0].transform.rotation   = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), 0.25f * PI);
	planes[0].material.texture    = Texture::load(DATA_PATH("Floor.png"));
	planes[0].material.reflection = 0.5f;

	meshes[0].init(DATA_PATH("Diamond.obj"));
	meshes[0].transform.position.y = 2.0f;
	meshes[0].transform.rotation   = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), 0.25f * PI);
	
	point_light_count = 0;
	/*point_lights = new PointLight[point_light_count = 1] {
		PointLight(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 8.0f))
	};*/

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
	planes.trace (ray, ray_hit);
	meshes.trace (ray, ray_hit);
}

bool Scene::intersect_primitives(const Ray & ray, __m128 max_distance) const {
	if (spheres.intersect(ray, max_distance)) return true;
	if (planes.intersect (ray, max_distance)) return true;
	if (meshes.intersect (ray, max_distance)) return true;

	return false;
}

SIMD_Vector3 Scene::bounce(const Ray & ray, int bounces_left, __m128 & distance) const {
	SIMD_Vector3 result;

	RayHit closest_hit;
	trace_primitives(ray, closest_hit);

	int hit_mask = _mm_movemask_ps(closest_hit.hit);

	// If any of the Rays did not hit
	if (hit_mask != 0xf) {
		//result   = skybox.sample(ray.direction);
		__m128 distance = _mm_set1_ps(INFINITY);

		// If none of the Rays hit, early out
		if (hit_mask == 0) return result;
	}

	distance = _mm_blendv_ps(distance, closest_hit.distance, closest_hit.hit);
	
	float us[4]; _mm_store_ps(us, closest_hit.u);
	float vs[4]; _mm_store_ps(vs, closest_hit.v);
	
	SIMD_Vector3 material_diffuse(
		closest_hit.material[3] ? closest_hit.material[3]->get_colour(us[3], vs[3]) : Vector3(0.0f),
		closest_hit.material[2] ? closest_hit.material[2]->get_colour(us[2], vs[2]) : Vector3(0.0f),
		closest_hit.material[1] ? closest_hit.material[1]->get_colour(us[1], vs[1]) : Vector3(0.0f),
		closest_hit.material[0] ? closest_hit.material[0]->get_colour(us[0], vs[0]) : Vector3(0.0f)
	);
	__m128 mask_diffuse = _mm_cmpgt_ps(SIMD_Vector3::length_squared(material_diffuse), _mm_set1_ps(0.0f));

	if (_mm_movemask_ps(mask_diffuse) != 0x0) {
		SIMD_Vector3 diffuse = SIMD_Vector3(ambient_lighting);

		// Secondary Ray starts at hit location
		Ray secondary_ray;
		secondary_ray.origin = closest_hit.point;

		SIMD_Vector3 to_camera = SIMD_Vector3::normalize(SIMD_Vector3(camera.position) - closest_hit.point);

		// Check Point Lights
		for (int i = 0; i < point_light_count; i++) {
			SIMD_Vector3 to_light = SIMD_Vector3(point_lights[i].position) - closest_hit.point;
			__m128 distance_to_light_squared = SIMD_Vector3::length_squared(to_light);
			__m128 distance_to_light         = _mm_sqrt_ps(distance_to_light_squared);

			to_light /= distance_to_light;
			secondary_ray.direction = to_light;

			if (!intersect_primitives(secondary_ray, distance_to_light)) {
				diffuse += point_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared);
			}
		}

		// Check Spot Lights
		for (int i = 0; i < spot_light_count; i++) {
			SIMD_Vector3 to_light = SIMD_Vector3(spot_lights[i].position) - closest_hit.point;
			__m128 distance_to_light_squared = SIMD_Vector3::length_squared(to_light);
			__m128 distance_to_light         = _mm_sqrt_ps(distance_to_light_squared);

			to_light /= distance_to_light;
			secondary_ray.direction = to_light;

			if (!intersect_primitives(secondary_ray, distance_to_light)) {
				diffuse += spot_lights[i].calc_lighting(closest_hit.normal, to_light, to_camera, distance_to_light_squared);
			}
		}

		// Check Directional Lights
		__m128 inf = _mm_set1_ps(INFINITY);

		for (int i = 0; i < directional_light_count; i++) {			
			secondary_ray.direction = SIMD_Vector3(directional_lights[i].negative_direction);

			if (!intersect_primitives(secondary_ray, inf)) {
				diffuse += directional_lights[i].calc_lighting(closest_hit.normal, to_camera);
			}
		}

		result += diffuse * material_diffuse;
	}

	// If we have bounces left to do, recurse one level deeper
	if (bounces_left > 0) {	
		SIMD_Vector3 colour_reflection;
		SIMD_Vector3 colour_refraction;

		SIMD_Vector3 material_reflection(
			closest_hit.material[3] ? closest_hit.material[3]->reflection : Vector3(0.0f),
			closest_hit.material[2] ? closest_hit.material[2]->reflection : Vector3(0.0f),
			closest_hit.material[1] ? closest_hit.material[1]->reflection : Vector3(0.0f),
			closest_hit.material[0] ? closest_hit.material[0]->reflection : Vector3(0.0f)
		);
		__m128 mask_reflection = _mm_cmpgt_ps(SIMD_Vector3::length_squared(material_reflection), _mm_set1_ps(0.0f));

		if (_mm_movemask_ps(mask_reflection) != 0x0) {
			Ray reflected_ray;
			reflected_ray.origin    = closest_hit.point;
			reflected_ray.direction = Math::reflect(ray.direction, closest_hit.normal);

			__m128 reflection_distance;
			colour_reflection = material_reflection * bounce(reflected_ray, bounces_left - 1, reflection_distance) * material_diffuse;
		}

		SIMD_Vector3 material_transmittance(
			closest_hit.material[3] ? closest_hit.material[3]->transmittance : Vector3(0.0f),
			closest_hit.material[2] ? closest_hit.material[2]->transmittance : Vector3(0.0f),
			closest_hit.material[1] ? closest_hit.material[1]->transmittance : Vector3(0.0f),
			closest_hit.material[0] ? closest_hit.material[0]->transmittance : Vector3(0.0f)
		);
		__m128 mask_refraction = _mm_cmpgt_ps(SIMD_Vector3::length_squared(material_transmittance), _mm_set1_ps(0.0f));

		if (_mm_movemask_ps(mask_refraction) != 0x0) {		
			__m128 dot = SIMD_Vector3::dot(ray.direction, closest_hit.normal);
			__m128 dot_mask = _mm_cmplt_ps(dot, _mm_set1_ps(0.0f));

			__m128 air = _mm_set1_ps(Material::AIR_INDEX_OF_REFRACTION);
			__m128 ior = _mm_set_ps(
				closest_hit.material[3] ? closest_hit.material[3]->index_of_refraction : 0.0f,
				closest_hit.material[2] ? closest_hit.material[2]->index_of_refraction : 0.0f,
				closest_hit.material[1] ? closest_hit.material[1]->index_of_refraction : 0.0f,
				closest_hit.material[0] ? closest_hit.material[0]->index_of_refraction : 0.0f
			);

			__m128 n_1 = _mm_blendv_ps(ior, air, dot_mask);
			__m128 n_2 = _mm_blendv_ps(air, ior, dot_mask);

			__m128       cos_theta = _mm_blendv_ps(dot, _mm_sub_ps(_mm_set1_ps(0.0f), dot), dot_mask);
			SIMD_Vector3 normal    = SIMD_Vector3::blend(-closest_hit.normal, closest_hit.normal, dot_mask);

			__m128 eta = _mm_div_ps(n_1, n_2);
			__m128 k = _mm_sub_ps(_mm_set1_ps(1.0f), _mm_mul_ps(_mm_mul_ps(eta, eta), _mm_sub_ps(_mm_set1_ps(1.0f), _mm_mul_ps(cos_theta, cos_theta))));
			
			// In case of Total Internal Reflection, return only the reflection component
			__m128 tir_mask = _mm_and_ps(closest_hit.hit, _mm_cmplt_ps(k, _mm_set1_ps(0.0f)));
			if (_mm_movemask_ps(tir_mask) == _mm_movemask_ps(mask_refraction)) {
				return SIMD_Vector3::blend(result, result + colour_reflection, mask_reflection);
			}

			Ray refracted_ray;
			refracted_ray.origin    = closest_hit.point;
			refracted_ray.direction = Math::refract(ray.direction, normal, eta, cos_theta, k);

			// Make sure that Snell's Law is correctly obeyed
			assert(Test::test_refraction(n_1, n_2, ray.direction, normal, refracted_ray.direction, _mm_and_ps(closest_hit.hit, (k, _mm_set1_ps(0.0f)))));

			__m128 refraction_distance;
			colour_refraction = bounce(refracted_ray, bounces_left - 1, refraction_distance);

			// Apply Beer's Law
			SIMD_Vector3 material_transmittance(
				closest_hit.material[3] ? closest_hit.material[3]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[2] ? closest_hit.material[2]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[1] ? closest_hit.material[1]->transmittance - Vector3(1.0f) : Vector3(0.0f),
				closest_hit.material[0] ? closest_hit.material[0]->transmittance - Vector3(1.0f) : Vector3(0.0f)
			);
			
			__m128 beer_x = Math::exp(_mm_mul_ps(material_transmittance.x, refraction_distance));
			__m128 beer_y = Math::exp(_mm_mul_ps(material_transmittance.y, refraction_distance));
			__m128 beer_z = Math::exp(_mm_mul_ps(material_transmittance.z, refraction_distance));
			
			colour_refraction.x = _mm_blendv_ps(colour_refraction.x, _mm_mul_ps(colour_refraction.x, beer_x), dot_mask);
			colour_refraction.y = _mm_blendv_ps(colour_refraction.y, _mm_mul_ps(colour_refraction.y, beer_y), dot_mask);
			colour_refraction.z = _mm_blendv_ps(colour_refraction.z, _mm_mul_ps(colour_refraction.z, beer_z), dot_mask);

			// Use Schlick's Approximation to simulate the Fresnel effect
			__m128 r_0 = _mm_div_ps(_mm_sub_ps(n_1, n_2), _mm_add_ps(n_1, n_2));
			r_0 = _mm_mul_ps(r_0, r_0);

			// In case n_1 is larger than n_2, theta should be the angle
			// between the normal and the refracted Ray direction
			cos_theta = _mm_blendv_ps(cos_theta, _mm_sub_ps(_mm_set1_ps(0.0f), SIMD_Vector3::dot(refracted_ray.direction, normal)), _mm_cmpgt_ps(n_1, n_2));

			// Calculate (1 - cos(theta))^5 efficiently, without using pow
			__m128 one_minus_cos         = _mm_sub_ps(_mm_set1_ps(1.0f), cos_theta);
			__m128 one_minus_cos_squared = _mm_mul_ps(one_minus_cos, one_minus_cos);

			__m128 F_r = _mm_add_ps(r_0, _mm_mul_ps(_mm_mul_ps(_mm_sub_ps(_mm_set1_ps(1.0f), r_0), one_minus_cos_squared), _mm_mul_ps(one_minus_cos_squared, one_minus_cos))); // r_0 + (1 - r_0) * (1 - cos)^5
			__m128 F_t = _mm_sub_ps(_mm_set1_ps(1.0f), F_r);
			
			SIMD_Vector3 blend = SIMD_Vector3::blend(F_r * colour_reflection + F_t * colour_refraction, colour_reflection, tir_mask);

			return SIMD_Vector3::blend(result, result + blend, mask_refraction);
		} else {
			return SIMD_Vector3::blend(result, result + colour_reflection, mask_reflection);
		}
	}

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
	Ray ray;
	ray.origin.x = _mm_set1_ps(camera.position.x);
	ray.origin.y = _mm_set1_ps(camera.position.y);
	ray.origin.z = _mm_set1_ps(camera.position.z);

	for (int j = y; j < y + window.tile_height; j += 2) {
		for (int i = x; i < x + window.tile_width; i += 2) {
			float i_f = float(i);
			float j_f = float(j);

			__m128 is = _mm_set_ps(i_f, i_f + 1.0f, i_f,        i_f + 1.0f);
			__m128 js = _mm_set_ps(j_f, j_f,        j_f + 1.0f, j_f + 1.0f);

			ray.direction = camera.get_ray_direction(is, js);

			__m128 distance;
			SIMD_Vector3 colour = bounce(ray, NUMBER_OF_BOUNCES, distance);

			float xs[4]; _mm_store_ps(xs, colour.x);
			float ys[4]; _mm_store_ps(ys, colour.y);
			float zs[4]; _mm_store_ps(zs, colour.z);

			window.plot(i,     j,     Vector3(xs[3], ys[3], zs[3]));
			window.plot(i + 1, j,     Vector3(xs[2], ys[2], zs[2]));
			window.plot(i,     j + 1, Vector3(xs[1], ys[1], zs[1]));
			window.plot(i + 1, j + 1, Vector3(xs[0], ys[0], zs[0]));
		}
	}
}
