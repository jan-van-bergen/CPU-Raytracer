#include "Texture.h"

#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include "Math.h"

static std::unordered_map<std::string, Texture *> cache;

const Texture * Texture::load(const char * file_path) {
	Texture *& texture = cache[file_path];

	// If the cache already contains this Texture simply return it
	if (texture) return texture;

	// Otherwise, load new Texture
	texture = new Texture();

	int channels;
	texture->data = reinterpret_cast<unsigned *>(stbi_load(file_path, &texture->width, &texture->height, &channels, STBI_rgb_alpha));

	if (texture->width == 0 || texture->height == 0) abort();

	texture->width_f  = float(texture->width);
	texture->height_f = float(texture->height);

	return texture;
}

Vector3 Texture::fetch_texel(int x, int y) const {
	assert(x >= 0 && x < width);
	assert(y >= 0 && y < height);

	assert(data);
	unsigned colour = data[x + y * width];

	const float one_over_255 = 0.00392156862f;
	float r = float((colour)       & 0xff) * one_over_255;
	float g = float((colour >> 8)  & 0xff) * one_over_255;
	float b = float((colour >> 16) & 0xff) * one_over_255;

	return Vector3(r, g, b);
}

Vector3 Texture::sample(float u, float v) const {
	int x = Math::mod(int(u * width_f  + 0.5f), width);
	int y = Math::mod(int(v * height_f + 0.5f), height);

	return fetch_texel(x, y);
}

Vector3 Texture::sample_bilinear(float u, float v) const {
	// Convert normalized (u,v) to pixel space
	u *= width_f;
	v *= height_f;

	// Convert pixel coordinates to integers
	int u_i = int(u);
	int v_i = int(v);
	
	int u0_i = Math::mod(u_i,     width);
	int u1_i = Math::mod(u_i + 1, width);
	int v0_i = Math::mod(v_i,     height);
	int v1_i = Math::mod(v_i + 1, height);

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
		w0 * fetch_texel(u0_i, v0_i) +
		w1 * fetch_texel(u1_i, v0_i) +
		w2 * fetch_texel(u0_i, v1_i) +
		w3 * fetch_texel(u1_i, v1_i);
}
