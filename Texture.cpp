#include "Texture.h"

static std::unordered_map<const char *, Texture *> cache;

const Texture * Texture::load(const char * file_path) {
	Texture *& texture = cache[file_path];

	if (texture == NULL) {
		texture = new Texture();

		int channels;
		texture->data = reinterpret_cast<unsigned *>(stbi_load(file_path, &texture->width, &texture->height, &channels, STBI_rgb_alpha));

		texture->width_f  = float(texture->width);
		texture->height_f = float(texture->height);
	}

	return texture;
}

Vector3 Texture::sample(float u, float v) const {
	u = u > 0.0f ? fmodf(u, 1.0f) : fmodf(u, 1.0f) + 1.0f;
	v = v > 0.0f ? fmodf(v, 1.0f) : fmodf(v, 1.0f) + 1.0f;

	int x = int(u * width_f);
	int y = int(v * height_f);

	assert(data);
	unsigned colour = data[x + y * width];

	static const float one_over_255 = 0.00392156862f;
	float r = float((colour)       & 0xff) * one_over_255;
	float g = float((colour >> 8)  & 0xff) * one_over_255;
	float b = float((colour >> 16) & 0xff) * one_over_255;

	return Vector3(r, g, b);
}
