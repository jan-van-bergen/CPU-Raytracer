#include "Texture.h"

#include <algorithm>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include "Math.h"

static std::unordered_map<std::string, Texture *> cache;

static Vector3 colour_unpack(unsigned colour) {
	const float one_over_255 = 0.00392156862f;
	float r = float((colour)       & 0xff) * one_over_255;
	float g = float((colour >> 8)  & 0xff) * one_over_255;
	float b = float((colour >> 16) & 0xff) * one_over_255;

	return Vector3(r, g, b);
}

static unsigned colour_pack(const Vector3 & colour) {
	unsigned r = Util::float_to_int(colour.x * 255.0f);
	unsigned g = Util::float_to_int(colour.y * 255.0f) << 8;
	unsigned b = Util::float_to_int(colour.z * 255.0f) << 16;

	return r | g | b;
}

const Texture * Texture::load(const char * file_path) {
	Texture *& texture = cache[file_path];

	// If the cache already contains this Texture simply return it
	if (texture) return texture;

	// Otherwise, load new Texture
	texture = new Texture();

	int channels;
	const unsigned * data = reinterpret_cast<unsigned *>(stbi_load(file_path, &texture->width, &texture->height, &channels, STBI_rgb_alpha));

	// Check if the Texture is valid
	if (texture->width == 0 || texture->height == 0) {
		printf("An error occured while loading Texture '%s'!\n", file_path);

		abort();
	}
	
#if TEXTURE_SAMPLE_MODE == TEXTURE_SAMPLE_MODE_MIPMAP
	bool use_mipmapping = Math::is_power_of_two(texture->width) && texture->width == texture->height;
#else
	bool use_mipmapping = false;
#endif

	texture->mipmapped = use_mipmapping;

	if (use_mipmapping) {
		texture->data = new unsigned[texture->width * texture->height + (texture->width * texture->height) / 3];
	} else {
		texture->data = new unsigned[texture->width * texture->height];
	}

	// Copy the data over into Mipmap level 0, and convert it to linear colour space
	for (int i = 0; i < texture->width * texture->height; i++) {
		Vector3 colour = colour_unpack(data[i]);

		texture->data[i] = colour_pack(Vector3(
			Math::gamma_to_linear(colour.x), 
			Math::gamma_to_linear(colour.y), 
			Math::gamma_to_linear(colour.z)
		));
	}

	delete [] data;

	if (use_mipmapping) {
		texture->mip_levels  = 1 + (int)log2f(texture->width);
		texture->mip_offsets = new int[texture->mip_levels];

		texture->mip_offsets[0] = 0;

		int offset      = texture->width * texture->height;
		int offset_prev = 0;

		int size      = texture->width >> 1;
		int size_prev = texture->width;
	
		int level = 1;

		// Obtain each subsequent Mipmap level by applying a Box Filter to the previous level
		while (size >= 1) {
			for (int j = 0; j < size; j++) {
				for (int i = 0; i < size; i++) {
					int i_prev = i << 1;
					int j_prev = j << 1;

					unsigned colour0 = texture->data[offset_prev +  i_prev     + j_prev    * size_prev];
					unsigned colour1 = texture->data[offset_prev + (i_prev+1) +  j_prev    * size_prev];
					unsigned colour2 = texture->data[offset_prev +  i_prev    + (j_prev+1) * size_prev];
					unsigned colour3 = texture->data[offset_prev + (i_prev+1) + (j_prev+1) * size_prev];

					unsigned sum_rb = (colour0 & 0xff00ff) + (colour1 & 0xff00ff) + (colour2 & 0xff00ff) + (colour3 & 0xff00ff);
					unsigned sum_g  = (colour0 & 0x00ff00) + (colour1 & 0x00ff00) + (colour2 & 0x00ff00) + (colour3 & 0x00ff00);

					unsigned average = ((sum_rb >> 2) & 0xff00ff) | ((sum_g >> 2) & 0x00ff00);

					texture->data[offset + i + j * size] = average;
				}
			}

			texture->mip_offsets[level++] = offset;

			offset_prev = offset;
			offset += size * size;

			size_prev = size;
			size >>= 1;
		}
	} else {
		texture->mip_levels  = 1;
		texture->mip_offsets = new int(0);
	}

	texture->width_f  = float(texture->width);
	texture->height_f = float(texture->height);

	return texture;
}

Vector3 Texture::fetch_texel(int x, int y, int level) const {
	//static Vector3 colours[] = { Vector3(1,0,0), Vector3(1,1,0), Vector3(0,1,0), Vector3(0,1,1), Vector3(0,0,1), Vector3(1,0,1), Vector3(1,1,1) };
	//return colours[level < 6 ? level : 6];

	int offset = mip_offsets[level];
	int size   = width >> level;

	x = Math::mod(x, size);
	y = Math::mod(y, size);

	assert(x >= 0 && x < width  >> level);
	assert(y >= 0 && y < height >> level);

	assert(data);
	return colour_unpack(data[offset + x + y * size]);
}

Vector3 Texture::sample_nearest(float u, float v) const {
	return fetch_texel(u * width_f, v * height_f);
}

Vector3 Texture::sample_bilinear(float u, float v, int level) const {
	int size = width >> level;

	// Convert normalized (u,v) to pixel space
	u = u * size - 0.5f;
	v = v * size - 0.5f;

	// Calculate bilinear weights
	float fractional_u = u - floor(u);
	float fractional_v = v - floor(v);

	float one_minus_fractional_u = 1.0f - fractional_u;
	float one_minus_fractional_v = 1.0f - fractional_v;

	float w0 = one_minus_fractional_u * one_minus_fractional_v;
	float w1 =           fractional_u * one_minus_fractional_v;
	float w2 = one_minus_fractional_u *           fractional_v;
	float w3 = 1.0f - w0 - w1 - w2;
	
	// Convert pixel coordinates to integers
	int u0_i = Util::float_to_int(u);
	int v0_i = Util::float_to_int(v);
	
	// Blend everything together using the weights
	return 
		w0 * fetch_texel(u0_i,     v0_i,     level) +
		w1 * fetch_texel(u0_i + 1, v0_i,     level) +
		w2 * fetch_texel(u0_i,     v0_i + 1, level) +
		w3 * fetch_texel(u0_i + 1, v0_i + 1, level);
}

// Code based on PBRT chapter 10.4
Vector3 Texture::sample_mipmap(float u, float v, float ds_dx, float ds_dy, float dt_dx, float dt_dy) const {
	if (!mipmapped) return sample_bilinear(u, v);

#if MIPMAP_FILTER == MIPMAP_FILTER_TRILINEAR
	float width = 2.0f * std::max(
		std::max(std::abs(ds_dx), std::abs(ds_dy)),
		std::max(std::abs(dt_dx), std::abs(dt_dy))
	);
	
	float lambda = float(mip_levels) - 1.0f + log2f(std::max(width, 1e-8f));
	int   level  = Util::float_to_int(lambda);

	if (level < 0)               return sample_bilinear(u, v);
	if (level >= mip_levels - 1) return fetch_texel(0, 0, mip_levels - 1);

	float t = lambda - floorf(lambda);

	return (1.0f - t) * sample_bilinear(u, v, level) + t * sample_bilinear(u, v, level + 1);
#elif MIPMAP_FILTER == MIPMAP_FILTER_EWA
	Vector2 major_axis = Vector2(ds_dx, dt_dx);
	Vector2 minor_axis = Vector2(ds_dy, dt_dy);
	
	float major_length = Vector2::length(major_axis);
	float minor_length = Vector2::length(minor_axis);
	
	if (minor_length > major_length) {
		std::swap(minor_axis,   major_axis);
		std::swap(minor_length, major_length);
	}

	if (minor_length < 0.00001f) return sample_bilinear(u, v);

	// Clamp ellipse eccentricity when it is too large
	if (minor_length * MAX_ANISOTROPY < major_length) {
		float scale = major_length / (minor_length * MAX_ANISOTROPY);

		minor_axis   *= scale;
		minor_length *= scale;
	}

	float lambda = std::max(0.0f, mip_levels - 1.0f + log2f(minor_length));
	int   level  = Util::float_to_int(lambda);

	return sample_ewa(u, v, level, major_axis, minor_axis);
#endif
}

Vector3 Texture::sample_ewa(float u, float v, int level, const Vector2 & major_axis, const Vector2 & minor_axis) const {
	if (level >= mip_levels - 1) return fetch_texel(0, 0, mip_levels - 1);

	float size = float(width >> level);

	// Convert EWA coordinates to appropriate scale for level
	u = u * size - 0.5f;
	v = v * size - 0.5f;

	Vector2 major_axis_scaled = major_axis * size;
	Vector2 minor_axis_scaled = minor_axis * size;

	// Compute ellipse coefficients to bound EWA filter region
	float a =  1.0f + (major_axis_scaled.y * major_axis_scaled.y + minor_axis_scaled.y * minor_axis_scaled.y);
	float b = -2.0f * (major_axis_scaled.x * major_axis_scaled.y + minor_axis_scaled.x * minor_axis_scaled.y);
	float c =  1.0f + (major_axis_scaled.x * major_axis_scaled.x + minor_axis_scaled.x * minor_axis_scaled.x);
	
	float one_over_f = 1.0f / (a * c - b * b * 0.25f);
	
	a *= one_over_f;
	b *= one_over_f;
	c *= one_over_f;

	// Compute the ellipse's bounding box in texture space
	float det     = -b * b + 4.0f * a * c;
	float inv_det = 1.0f / det;

	float sqrt_u = sqrtf(det * c);
	float sqrt_v = sqrtf(det * a);

	int s0 = Util::float_to_int(ceilf (u - 2.0f * inv_det * sqrt_u));
	int s1 = Util::float_to_int(floorf(u + 2.0f * inv_det * sqrt_u));
	int t0 = Util::float_to_int(ceilf (v - 2.0f * inv_det * sqrt_v));
	int t1 = Util::float_to_int(floorf(v + 2.0f * inv_det * sqrt_v));

	// Scan over ellipse bound and compute quadratic equation
	Vector3 sum(0.0f);
	float   sum_weights = 0.0f;

	for (int it = t0; it <= t1; ++it) {
		float tt = it - v;

		for (int is = s0; is <= s1; ++is) {
			float ss = is - u;

			// Compute squared radius and filter texel if inside ellipse
			float r2 = a * ss * ss + b * ss * tt + c * tt * tt;
			if (r2 < 1.0f) {
				int   index  = std::min(Util::float_to_int(r2 * ewa_weight_table_size), ewa_weight_table_size - 1);
				float weight = ewa_weight_table[index];

				sum         += weight * fetch_texel(is, it, level);
				sum_weights += weight;
			}
		}
	}

	return sum / sum_weights;
}
