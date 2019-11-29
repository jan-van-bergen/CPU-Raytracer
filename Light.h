#pragma once
#include "Math.h"
#include "Vector3.h"
#include "SIMDUtil.h"

struct Light {
	Vector3 colour;

	inline Light(const Vector3 & colour) : colour(colour) { }

	// Calculate lighting using Blinn-Phong model
	inline SIMD_Vector3 calc_lighting(const SIMD_Vector3 & normal, const SIMD_Vector3 & to_light, const SIMD_Vector3 & to_camera) const {
		const __m128 zero = _mm_set1_ps(0.0f);

        __m128 intensity = SIMD_Vector3::dot(normal, to_light);

		__m128 mask = _mm_cmpgt_ps(intensity, zero);
        if (_mm_movemask_ps(mask) == 0x0) return SIMD_Vector3(0.0f);

        SIMD_Vector3 half_angle = SIMD_Vector3::normalize(_mm_set1_ps(0.5f) * (to_light + to_camera));

        __m128 specular_factor = SIMD_Vector3::dot(normal, half_angle);
        intensity = _mm_add_ps(intensity, Math::pow2<128>(specular_factor));

        return _mm_blendv_ps(zero, intensity, mask) * SIMD_Vector3(colour);
	}
};
