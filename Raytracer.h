#pragma once
#include "Scene.h"

struct PerformanceStats {
	int num_primary_rays;
	int num_shadow_rays;
	int num_reflection_rays;
	int num_refraction_rays;
};

struct Raytracer {
	const Scene * scene;
	
	void render_tile(const Window & window, int tile_x, int tile_y, int tile_width, int tile_height, PerformanceStats & stats) const;

private:
	SIMD_Vector3 bounce(const Ray & ray, int bounces_left, SIMD_float & distance, PerformanceStats & stats) const;
};
