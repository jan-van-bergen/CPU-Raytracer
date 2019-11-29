#pragma once
#include "Math.h"
#include "SIMD.h"
#include "Vector3.h"

struct Light {
	Vector3 colour;

	inline Light(const Vector3 & colour) : colour(colour) { }

	// Calculate lighting using Blinn-Phong model
	inline SIMD_Vector3 calc_lighting(const SIMD_Vector3 & normal, const SIMD_Vector3 & to_light, const SIMD_Vector3 & to_camera) const {
		const SIMD_float zero(0.0f);

        SIMD_float intensity = SIMD_Vector3::dot(normal, to_light);

		SIMD_float mask = intensity > zero;
        if (SIMD_float::all_false(mask)) return SIMD_Vector3(0.0f);

        SIMD_Vector3 half_angle = SIMD_Vector3::normalize(SIMD_float(0.5f) * (to_light + to_camera));

        SIMD_float specular_factor = SIMD_Vector3::dot(normal, half_angle);
        intensity = intensity + Math::pow2<128>(specular_factor);

        return SIMD_float::blend(zero, intensity, mask) * SIMD_Vector3(colour);
	}
};
