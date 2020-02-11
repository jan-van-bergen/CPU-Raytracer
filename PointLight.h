#pragma once
#include "Light.h"

struct PointLight : Light {
	SIMD_Vector3 position;

	inline PointLight(const Vector3 & colour, const Vector3 & position) : Light(colour), position(position) { }

	inline SIMD_Vector3 calc_lighting(const SIMD_Vector3 & normal, const SIMD_Vector3 & to_light, const SIMD_Vector3 & to_camera, SIMD_float distance_squared) const {
		return Light::calc_lighting(normal, to_light, to_camera) / distance_squared;
	}
};
