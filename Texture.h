#pragma once
#include <cassert>
#include <unordered_map>

#include <stb_image/stb_image.h>

#include "Vector3.h"

struct Texture {
private:
	unsigned * data;

	int   width,   height;
	float width_f, height_f;

public:
	Vector3 sample(float u, float v) const;

	static const Texture * load(const char * file_path);
};