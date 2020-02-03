#pragma once
#include "Vector3.h"

#define TEXTURE_SAMPLE_MODE_NEAREST  0
#define TEXTURE_SAMPLE_MODE_BILINEAR 1

#define TEXTURE_SAMPLE_MODE TEXTURE_SAMPLE_MODE_NEAREST

struct Texture {
private:
	unsigned * data = nullptr;

	int   width,   height;
	float width_f, height_f;

	Vector3 fetch_texel(int x, int y) const;

public:
	Vector3 sample         (float u, float v) const;
	Vector3 sample_bilinear(float u, float v) const;

	static const Texture * load(const char * file_path);
};
