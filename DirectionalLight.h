#pragma once
#include "Light.h"

struct DirectionalLight : Light {
	SIMD_Vector3 negative_direction;

	inline DirectionalLight(const Vector3 & colour, const Vector3 & direction) : Light(colour), negative_direction(-direction) { }

	inline SIMD_Vector3 calc_lighting(const SIMD_Vector3 & normal, const SIMD_Vector3 & to_camera) const {
		return Light::calc_lighting(normal, negative_direction, to_camera);
	}
};
