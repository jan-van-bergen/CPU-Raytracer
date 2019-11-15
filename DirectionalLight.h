#pragma once
#include "Light.h"

struct DirectionalLight : Light {
	Vector3 direction; // NEGATIVE direction of the Directional Light

	inline DirectionalLight(const Vector3 & colour, const Vector3 & direction) : Light(colour), direction(-direction) { }

	inline Vector3 calc_lighting(const Vector3 & normal, const Vector3 & to_camera) const {
		return Light::calc_lighting(normal, direction, to_camera);
	}
};
