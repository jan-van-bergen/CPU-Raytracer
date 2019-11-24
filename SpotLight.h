#pragma once
#include "PointLight.h"

#include "Util.h"

struct SpotLight : PointLight {
	Vector3 negative_direction;

	float outer_cutoff;
	float inner_cutoff;

	inline SpotLight(const Vector3 & colour, const Vector3 & position, const Vector3 & direction, float outer_angle, float inner_angle) : PointLight(colour, position), negative_direction(-direction) {
		outer_cutoff = cosf(DEG_TO_RAD(0.5f * outer_angle));
		inner_cutoff = cosf(DEG_TO_RAD(0.5f * inner_angle));
	}

	inline Vector3 calc_lighting(const Vector3 & normal, const Vector3 & to_light, const Vector3 & to_camera, float distance_squared) const {
		float dot = Vector3::dot(to_light, negative_direction);

		if (dot < outer_cutoff) return Vector3(0.0f);

		float intensity = Math3d::clamp((dot - outer_cutoff) / (inner_cutoff - outer_cutoff), 0.0f, 1.0f);    

		return intensity * PointLight::calc_lighting(normal, to_light, to_camera, distance_squared);
	}
};
