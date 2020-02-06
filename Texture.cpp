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
	unsigned r = int(colour.x * 255.0f);
	unsigned g = int(colour.y * 255.0f) << 8;
	unsigned b = int(colour.z * 255.0f) << 16;

	return r | g | b;
}

const Texture * Texture::load(const char * file_path) {
	Texture *& texture = cache[file_path];

	// If the cache already contains this Texture simply return it
	if (texture) return texture;

	// Otherwise, load new Texture
	texture = new Texture();

	int channels;
	unsigned * data = reinterpret_cast<unsigned *>(stbi_load(file_path, &texture->width, &texture->height, &channels, STBI_rgb_alpha));

	// Check if the Texture is valid
	if (texture->width == 0 || texture->height == 0) {
		printf("An error occured while loading Texture '%s'!\n", file_path);

		abort();
	}
	
	texture->data = new unsigned[2 * texture->width * texture->height];

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

	if (!Math::is_power_of_two(texture->width) || texture->width != texture->height) {
		texture->mipmapped = false;

		texture->mip_levels  = 1;
		texture->mip_offsets = new int(0);
	} else {
		texture->mipmapped = true;

		int offset      = texture->width * texture->height;
		int offset_prev = 0;

		int size      = texture->width >> 1;
		int size_prev = texture->width;
	
		texture->mip_levels = 1 + (int)log2f(texture->width);
		texture->mip_offsets  = new int[texture->mip_levels];

		texture->mip_offsets[0] = 0;
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
	}

	texture->width_f  = float(texture->width);
	texture->height_f = float(texture->height);

	return texture;
}

Vector3 Texture::fetch_texel(int x, int y, int level) const {
	int offset = mip_offsets[level];
	int size   = width >> level;

	assert(x >= 0 && x < width  >> level);
	assert(y >= 0 && y < height >> level);

	assert(data);
	unsigned colour = data[offset + x + y * size];

	const float one_over_255 = 0.00392156862f;
	float r = float((colour)       & 0xff) * one_over_255;
	float g = float((colour >> 8)  & 0xff) * one_over_255;
	float b = float((colour >> 16) & 0xff) * one_over_255;

	return Vector3(r, g, b);
}

Vector3 Texture::sample_nearest(float u, float v) const {
	int x = Math::mod(int(u * width_f  + 0.5f), width);
	int y = Math::mod(int(v * height_f + 0.5f), height);

	return fetch_texel(x, y);
}

Vector3 Texture::sample_bilinear(float u, float v, int level) const {
	int size = width >> level;

	// Convert normalized (u,v) to pixel space
	u = u * size - 1.5f;
	v = v * size - 1.5f;

	// Convert pixel coordinates to integers
	int u_i = int(u);
	int v_i = int(v);
	
	int u0_i = Math::mod(u_i,     size);
	int u1_i = Math::mod(u_i + 1, size);
	int v0_i = Math::mod(v_i,     size);
	int v1_i = Math::mod(v_i + 1, size);

	// Calculate bilinear weights
	float fractional_u = u - floor(u);
	float fractional_v = v - floor(v);

	float one_minus_fractional_u = 1.0f - fractional_u;
	float one_minus_fractional_v = 1.0f - fractional_v;

	float w0 = one_minus_fractional_u * one_minus_fractional_v;
	float w1 =           fractional_u * one_minus_fractional_v;
	float w2 = one_minus_fractional_u *           fractional_v;
	float w3 = 1.0f - w0 - w1 - w2;

	// Blend everything together using the weights
	return 
		w0 * fetch_texel(u0_i, v0_i, level) +
		w1 * fetch_texel(u1_i, v0_i, level) +
		w2 * fetch_texel(u0_i, v1_i, level) +
		w3 * fetch_texel(u1_i, v1_i, level);
}

Vector3 Texture::sample_mipmap(float u, float v, float ds_dx, float ds_dy, float dt_dx, float dt_dy) const {
	if (!mipmapped) return sample_bilinear(u, v);

	ds_dx *= width_f;
	ds_dy *= width_f;
	dt_dx *= height_f;
	dt_dy *= height_f;

	float rho = std::max(sqrtf(ds_dx*ds_dx + dt_dx*dt_dx), sqrtf(ds_dy*ds_dy + dt_dy*dt_dy));

	float lambda = 1.0f + log2f(rho);

	int level = lambda;

	if (level < 0)               return sample_bilinear(u, v);
	if (level >= mip_levels - 1) return fetch_texel(0, 0, mip_levels - 1);

	float t = lambda - floorf(lambda);

	return (1.0f - t) * sample_bilinear(u, v, level) + t * sample_bilinear(u, v, level + 1);
}
