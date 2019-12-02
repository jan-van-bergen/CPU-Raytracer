#pragma once
#include "Vector3.h"
#include "Texture.h"

struct Material {
	Vector3 diffuse = 1.0f;	
	const Texture * texture = nullptr;

	Vector3 reflection = 0.0f;

	Vector3 transmittance       = 0.0f;
	float   index_of_refraction = 1.0f;

	inline static const float AIR_INDEX_OF_REFRACTION = 1.0f;

	static Material default_material;

	inline Vector3 get_colour(float u, float v) const {
		if (texture) {
			return diffuse * texture->sample_bilinear(u, v);
		}

		return diffuse;
	}
};

inline Material Material::default_material = {
	Vector3(0.0f),
	nullptr,
	Vector3(0.0f),
	Vector3(0.0f),
	1.0f
};
