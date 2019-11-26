#pragma once
#include "Vector3.h"
#include "Texture.h"

struct Material {
	Vector3 colour = Vector3(1.0f);	
	const Texture * texture = nullptr;

	Vector3 specular = 0.0f;

	Vector3 transmittance       = 0.0f;
	float   index_of_refraction = 1.0f;

	inline static const float AIR_INDEX_OF_REFRACTION = 1.0003f;

	inline Vector3 get_colour(float u, float v) const {
		if (texture) {
			return colour * texture->sample_bilinear(u, v);
		}

		return colour;
	}
};
