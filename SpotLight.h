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

	inline Vector3 calc_lighting(const Vector3 & normal, const Vector3 & to_light, const Vector3 & to_camera, float distance_squared) const {
		float dot = Vector3::dot(to_light, negative_direction);

		if (dot < outer_cutoff) return Vector3(0.0f);

		// Modulate light intensity as a function of the angle
		float radial_falloff = (dot - outer_cutoff) / (inner_cutoff - outer_cutoff);
		if (radial_falloff > 1.0f) {
			radial_falloff = 1.0f;
		}

		return radial_falloff * PointLight::calc_lighting(normal, to_light, to_camera, distance_squared);
	}
};
