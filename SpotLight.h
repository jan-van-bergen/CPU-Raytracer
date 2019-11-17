#pragma once
#include "PointLight.h"

#include "Util.h"

struct SpotLight : PointLight {
	Vector3 negative_direction;
	float   cutoff, one_over_one_minus_cutoff;

	inline SpotLight(const Vector3 & colour, const Vector3 & position, const Vector3 & direction, float angle) : PointLight(colour, position), negative_direction(-direction) {
		cutoff = cosf(DEG_TO_RAD(0.5f * angle));
		one_over_one_minus_cutoff = 1.0f / (1.0f - cutoff);
	}

	inline Vector3 calc_lighting(const Vector3 & normal, const Vector3 & to_light, const Vector3 & to_camera, float distance_squared) const {
		float dot = Vector3::dot(to_light, negative_direction);

		if (dot < cutoff) return Vector3(0.0f);

		return (dot - cutoff) * one_over_one_minus_cutoff * PointLight::calc_lighting(normal, to_light, to_camera, distance_squared);
	}
};
