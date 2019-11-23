#pragma once
#include "Vector3.h"
#include "Texture.h"

struct Material {
	Vector3 colour = Vector3(1.0f);	
	const Texture * texture = nullptr;

	float reflectiveness = 0.0f;

	float refractiveness   = 0.0f;
	float refractive_index = 1.0f;

	inline static const float AIR_REFRACTIVE_INDEX = 1.0003f;

	inline Vector3 get_colour(float u, float v) const {
		if (texture) {
			return colour * texture->sample_bilinear(u, v);
		}

		return colour;
	}
};
