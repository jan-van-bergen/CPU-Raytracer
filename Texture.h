#pragma once
#include <cassert>
#include <unordered_map>

#include "Vector3.h"

struct Texture {
private:
	unsigned * data;

	int   width,   height;
	float width_f, height_f;

	Vector3 get_texel(int x, int y) const;

public:
	Vector3 sample         (float u, float v) const;
	Vector3 sample_bilinear(float u, float v) const;

	static const Texture * load(const char * file_path);
};