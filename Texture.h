#pragma once
#include "Vector3.h"

#define TEXTURE_SAMPLE_MODE_NEAREST  0
#define TEXTURE_SAMPLE_MODE_BILINEAR 1
#define TEXTURE_SAMPLE_MODE_MIPMAP   2

#define TEXTURE_SAMPLE_MODE TEXTURE_SAMPLE_MODE_MIPMAP

struct Texture {
//private:
	unsigned * data = nullptr;

	int   mip_levels = 0;
	int * mip_offsets;

	bool mipmapped = false;

	int   width,   height;
	float width_f, height_f;

	Vector3 fetch_texel(int x, int y, int level = 0) const;

	Vector3 sample_nearest (float u, float v) const;
	Vector3 sample_bilinear(float u, float v, int level = 0) const;
	Vector3 sample_mipmap  (float u, float v, float ds_dx, float ds_dy, float dt_dx, float dt_dy) const;

public:
	inline Vector3 sample(float u, float v, float ds_dx, float ds_dy, float dt_dx, float dt_dy) const {
#if TEXTURE_SAMPLE_MODE == TEXTURE_SAMPLE_MODE_NEAREST
		return sample_nearest(u, v);
#elif TEXTURE_SAMPLE_MODE == TEXTURE_SAMPLE_MODE_BILINEAR
		return sample_bilinear(u, v);
#elif TEXTURE_SAMPLE_MODE == TEXTURE_SAMPLE_MODE_MIPMAP
		return sample_mipmap(u, v, ds_dx, ds_dy, dt_dx, dt_dy);
#endif
	}

	static const Texture * load(const char * file_path);
};
