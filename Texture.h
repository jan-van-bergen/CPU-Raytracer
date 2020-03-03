#pragma once
#include "Vector2.h"
#include "Vector3.h"

#include "Config.h"

struct Texture {
private:
	Vector3 * data = nullptr;
	
	int   width,   height;
	float width_f, height_f;

	bool mipmapped = false;

	int   mip_levels   = 0;
	float mip_levels_f = 0.0f;
	int * mip_offsets  = nullptr;

	inline static const int ewa_weight_table_size = 128;
	inline static float     ewa_weight_table[ewa_weight_table_size];

	Vector3 fetch_texel(int x, int y, int level = 0) const;

	Vector3 sample_nearest (float s, float t)                const;
	Vector3 sample_bilinear(float s, float t, int level = 0) const;
	
	Vector3 sample_mipmap_trilinear  (float s, float t, float ds_dx, float ds_dy, float dt_dx, float dt_dy) const;
	Vector3 sample_mipmap_anisotropic(float s, float t, float ds_dx, float ds_dy, float dt_dx, float dt_dy) const;
	Vector3 sample_mipmap_ewa        (float s, float t, float ds_dx, float ds_dy, float dt_dx, float dt_dy) const;

public:
	inline Vector3 sample(float s, float t, float ds_dx, float ds_dy, float dt_dx, float dt_dy) const {
#if TEXTURE_SAMPLE_MODE == TEXTURE_SAMPLE_MODE_NEAREST
		return sample_nearest(s, t);
#elif TEXTURE_SAMPLE_MODE == TEXTURE_SAMPLE_MODE_BILINEAR
		return sample_bilinear(s, t);
#elif TEXTURE_SAMPLE_MODE == TEXTURE_SAMPLE_MODE_MIPMAP
		if (!mipmapped) return sample_bilinear(s, t);

	#if MIPMAP_FILTER == MIPMAP_FILTER_TRILINEAR
		return sample_mipmap_trilinear(s, t, ds_dx, ds_dy, dt_dx, dt_dy);
	#elif MIPMAP_FILTER == MIPMAP_FILTER_ANISOTROPIC
		return sample_mipmap_anisotropic(s, t, ds_dx, ds_dy, dt_dx, dt_dy);
	#elif MIPMAP_FILTER == MIPMAP_FILTER_EWA
		return sample_mipmap_ewa(s, t, ds_dx, ds_dy, dt_dx, dt_dy);
	#endif
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
