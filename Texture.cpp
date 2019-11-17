#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include "Math3d.h"

#include "Hash.h"

static std::unordered_map<const char *, Texture *, StringHash, StringCompare> cache;

const Texture * Texture::load(const char * file_path) {
	Texture *& texture = cache[file_path];

	// If the cache already contains this Texture simply return it
	if (texture) return texture;

	// Otherwise, load new Texture
	texture = new Texture();

	int channels;
	texture->data = reinterpret_cast<unsigned *>(stbi_load(file_path, &texture->width, &texture->height, &channels, STBI_rgb_alpha));

	texture->width_f  = float(texture->width);
	texture->height_f = float(texture->height);

	return texture;
}

Vector3 Texture::get_texel(int x, int y) const {
	x = Math3d::clamp(x, 0, width  - 1);
	y = Math3d::clamp(y, 0, height - 1);

	assert(data);
	unsigned colour = data[x + y * width];

	static const float one_over_255 = 0.00392156862f;
	float r = float((colour)       & 0xff) * one_over_255;
	float g = float((colour >> 8)  & 0xff) * one_over_255;
	float b = float((colour >> 16) & 0xff) * one_over_255;

	return Vector3(r, g, b);
}

Vector3 Texture::sample(float u, float v) const {
	u = u > 0.0f ? fmodf(u, 1.0f) : fmodf(u, 1.0f) + 1.0f;
	v = v > 0.0f ? fmodf(v, 1.0f) : fmodf(v, 1.0f) + 1.0f;

	int x = int(u * width_f);
	int y = int(v * height_f);

	return get_texel(x, y);
}

Vector3 Texture::sample_bilinear(float u, float v) const {
	u = u > 0.0f ? fmodf(u, 1.0f) : fmodf(u, 1.0f) + 1.0f;
	v = v > 0.0f ? fmodf(v, 1.0f) : fmodf(v, 1.0f) + 1.0f;

	// Convert normalized (u,v) to pixel space
	u *= width_f;
	v *= height_f;

	// Calculate bilinear weights
	float u0 = floor(u);
    float v0 = floor(v);

    float u1 = u0 + 1.0f;
    float v1 = v0 + 1.0f;

    float w0 = (u1 - u) * (v1 - v);
    float w1 = (u - u0) * (v1 - v);
    float w2 = (u1 - u) * (v - v0);
    float w3 = 1.0f - w0 - w1 - w2;

	// Convert pixel coordinates to integers
	int u0_i = int(u0);
	int u1_i = int(u1);
	int v0_i = int(v0);
	int v1_i = int(v1);

	// Blend everything together using the weights
	return 
		w0 * get_texel(u0_i, v0_i) +
		w1 * get_texel(u1_i, v0_i) +
		w2 * get_texel(u0_i, v1_i) +
		w3 * get_texel(u1_i, v1_i);
}
