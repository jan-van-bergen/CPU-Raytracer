#pragma once
#include "Vector3.h"
#include "Texture.h"

struct Material {
	Vector3 colour = Vector3(1.0f);	
	Texture texture;

	inline Vector3 get_colour(float u, float v) const {
		return colour * texture.sample(u, v);
	}
};
