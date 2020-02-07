#pragma once
#include "Vector2.h"
#include "Vector3.h"

#include "Config.h"

struct Texture {
private:
	unsigned * data = nullptr;
	
	int   width,   height;
	float width_f, height_f;

	bool mipmapped = false;

	int   mip_levels = 0;
	int * mip_offsets;

	inline static const int ewa_weight_table_size = 128;
	inline static float     ewa_weight_table[ewa_weight_table_size];

	Vector3 fetch_texel(int x, int y, int level = 0) const;

	Vector3 sample_nearest (float u, float v) const;
	Vector3 sample_bilinear(float u, float v, int level = 0) const;
	Vector3 sample_mipmap  (float u, float v, float ds_dx, float ds_dy, float dt_dx, float dt_dy) const;

	Vector3 sample_ewa(float u, float v, int level, const Vector2 & major_axis, const Vector2 & minor_axis) const;

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

	inline static void init(float alpha = 2.0f) {
		float denom = 1.0f / float(ewa_weight_table_size - 1);

		float exp_neg_alpha = expf(-alpha);

		for (int i = 0; i < ewa_weight_table_size; i++) {
            float r2 = float(i) * denom;
            ewa_weight_table[i] = expf(-alpha * r2) - exp_neg_alpha;
        }
	}
};
