#pragma once
#include "Scene.h"

struct Raytracer {
	const Scene * scene;
	
	void render_tile(const Window & window, int x, int y, int tile_width, int tile_height) const;

private:
	SIMD_Vector3 bounce(const Ray & ray, int bounces_left, SIMD_float & distance) const;
};
