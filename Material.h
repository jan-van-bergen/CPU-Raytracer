#pragma once
#include <vector>

#include "Texture.h"

struct Material {
	Vector3 diffuse = 1.0f;	
	const Texture * texture = nullptr;

	Vector3 reflection = 0.0f;

	Vector3 transmittance       = 0.0f;
	float   index_of_refraction = 1.0f;

	inline Vector3 get_colour(float u, float v) const {
		if (texture) {
			return diffuse * texture->sample_bilinear(u, v);
		}

		return diffuse;
	}
	
	inline static const float air_index_of_refraction = 1.0f;
	
	inline static std::vector<Material> materials;

	inline static void init() {
		if (materials.size() != 0) abort();

		materials.push_back({
			Vector3(0.0f),
			nullptr,      
			Vector3(0.0f),
			Vector3(0.0f),
			1.0f		  
		});
	}

	inline static int add_material() {
		int index = materials.size();
		materials.push_back(Material());

		return index;
	}
};
