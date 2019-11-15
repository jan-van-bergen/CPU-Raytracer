#pragma once
#include "Light.h"

struct PointLight : Light {
	Vector3 position;

	inline PointLight(const Vector3 & colour, const Vector3 & position) : Light(colour), position(position) { }

	inline Vector3 calc_lighting(const Vector3 & normal, const Vector3 & to_light, const Vector3 & to_camera, float distance_squared) const {
		return Light::calc_lighting(normal, to_light, to_camera) / distance_squared;
	}
};
