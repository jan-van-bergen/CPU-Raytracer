#pragma once
#include "Math.h"
#include "Vector3.h"

struct Light {
	Vector3 colour;

	inline Light(const Vector3 & colour) : colour(colour) { }

	// Calculate lighting using Blinn-Phong model
	inline Vector3 calc_lighting(const Vector3 & normal, const Vector3 & to_light, const Vector3 & to_camera) const {
        float intensity = Vector3::dot(normal, to_light);

        if (intensity <= 0.0f) return Vector3(0.0f);

        Vector3 half_angle = Vector3::normalize(0.5f * (to_light + to_camera));

        float specular_factor = Vector3::dot(normal, half_angle);
        if (specular_factor > 0.0f) {
            intensity += Math::pow2<128>(specular_factor);
		}

        return intensity * colour;
	}
};
