#pragma once
#include "Vector3.h"

struct Light {
	Vector3 colour;

	inline Light(const Vector3 & colour) : colour(colour) { }

	// Calculate lighting using Blinn-Phong model
	inline Vector3 calc_lighting(const Vector3 & normal, const Vector3 & to_light, const Vector3 & to_camera) const {
		static const float specular_power = 128.0f;

		float diffuse_factor = Vector3::dot(normal, to_light);
        float intensity = 0.0f;

        if (diffuse_factor > 0.0f) {
            intensity = diffuse_factor;

            Vector3 half_angle = Vector3::normalize(0.5f * (to_light + to_camera));

            float specular_factor = Vector3::dot(normal, half_angle);
            if (specular_factor > 0.0f) {
                intensity += powf(specular_factor, specular_power);
			}
        }

        return intensity * colour;
	}
};