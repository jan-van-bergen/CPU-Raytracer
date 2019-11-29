#pragma once
#include "PointLight.h"

#include "Util.h"

struct SpotLight : PointLight {
	Vector3 negative_direction;

	float inner_cutoff; // Cosine of half the angle that describes the inner cone of the SpotLight. Angles inside this cone will receive full SpotLight intensity
	float outer_cutoff; // Cosine of half the angle that describes the outer cone of the SpotLight. Angles outside this cone will receive no SpotLight intensity

	inline SpotLight(const Vector3 & colour, const Vector3 & position, const Vector3 & direction, float inner_angle, float outer_angle) : PointLight(colour, position), negative_direction(-direction) {
		inner_cutoff = cosf(DEG_TO_RAD(0.5f * inner_angle));
		outer_cutoff = cosf(DEG_TO_RAD(0.5f * outer_angle));
	}

	inline SIMD_Vector3 calc_lighting(const SIMD_Vector3 & normal, const SIMD_Vector3 & to_light, const SIMD_Vector3 & to_camera, SIMD_float distance_squared) const {
		const SIMD_float one(1.0f);

		SIMD_float dot = SIMD_Vector3::dot(to_light, SIMD_Vector3(negative_direction));

		SIMD_float inner(inner_cutoff);
		SIMD_float outer(outer_cutoff);

		SIMD_float mask = dot > outer;
		if (SIMD_float::all_false(mask)) return SIMD_Vector3(0.0f);

		// Modulate light intensity as a function of the angle
		SIMD_float radial_falloff = (dot - outer) / (inner - outer);
		radial_falloff = SIMD_float::blend(radial_falloff, one, radial_falloff > one);
		
		return SIMD_float::blend(SIMD_float(0.0f), radial_falloff, mask) * PointLight::calc_lighting(normal, to_light, to_camera, distance_squared);
	}
};
