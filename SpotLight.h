#pragma once
#include "PointLight.h"

struct SpotLight : PointLight {
	Vector3 direction; // NEGATIVE direction
	float   cutoff;

	inline SpotLight(const Vector3 & colour, const Vector3 & position, const Vector3 & direction, float cutoff) : PointLight(colour, position), direction(-direction), cutoff (cutoff) { }

	inline Vector3 calc_lighting(const Vector3 & normal, const Vector3 & to_light, const Vector3 & to_camera, float distance_squared) const {
		float dot = Vector3::dot(to_light, direction);

		if (dot < cutoff) return Vector3(0.0f);

		return (dot - cutoff) / (1.0f - cutoff) * PointLight::calc_lighting(normal, to_light, to_camera, distance_squared);
	}
};
