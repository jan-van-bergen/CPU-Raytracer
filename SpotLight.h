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

	inline SIMD_Vector3 calc_lighting(const SIMD_Vector3 & normal, const SIMD_Vector3 & to_light, const SIMD_Vector3 & to_camera, __m128 distance_squared) const {
		const __m128 one = _mm_set1_ps(1.0f);

		__m128 dot = SIMD_Vector3::dot(to_light, SIMD_Vector3(negative_direction));

		__m128 inner = _mm_set1_ps(inner_cutoff);
		__m128 outer = _mm_set1_ps(outer_cutoff);

		__m128 mask = _mm_cmpgt_ps(dot, outer);
		if (_mm_movemask_ps(mask) == 0x0) return SIMD_Vector3(0.0f);

		// Modulate light intensity as a function of the angle
		__m128 radial_falloff = _mm_div_ps(_mm_sub_ps(dot, outer), _mm_sub_ps(inner, outer));
		radial_falloff = _mm_blendv_ps(radial_falloff, one, _mm_cmpgt_ps(radial_falloff, one));
		
		return _mm_blendv_ps(_mm_set1_ps(0.0f), radial_falloff, mask) * PointLight::calc_lighting(normal, to_light, to_camera, distance_squared);
	}
};
