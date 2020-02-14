#pragma once
#include "Texture.h"

struct Material {
	Vector3 diffuse = 1.0f;	
	const Texture * texture = nullptr;

	Vector3 reflection = 0.0f;

	Vector3 transmittance       = 0.0f;
	float   index_of_refraction = 1.0f;

	inline Vector3 get_albedo(float u, float v, float ds_dx, float ds_dy, float dt_dx, float dt_dy) const {
		if (texture) {
			return diffuse * texture->sample(u, v, ds_dx, ds_dy, dt_dx, dt_dy);
		}

		return diffuse;
	}
	
	inline static const float air_index_of_refraction = 1.0f;
	
};

namespace MaterialBuffer {
	inline int      material_count = 0;
	inline Material materials[MAX_MATERIALS];

	inline int reserve() {
		return material_count++;
	}

	inline void add(const Material & material) {
		materials[material_count++] = material;
	}
	
	inline void init() {
		Material default_material;
		default_material.diffuse = 0.0f;
		default_material.texture = nullptr;
		default_material.reflection    = 0.0f;
		default_material.transmittance = 0.0f;

		add(default_material);
	}
}
