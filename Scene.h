#pragma once
#include "Sphere.h"
#include "Plane.h"
#include "Mesh.h"
#include "PrimitiveList.h"

#include "BVH.h"

#include "PointLight.h"
#include "SpotLight.h"
#include "DirectionalLight.h"

#include "Camera.h"

#include "Sky.h"

#include "Window.h"

struct Scene {
private:
	PrimitiveList<Sphere> spheres;
	PrimitiveList<Plane>  planes;

	BVH<Mesh> bvh_meshes;

	// Lights
	PointLight * point_lights      = nullptr;
	int          point_light_count = 0;

	SpotLight * spot_lights      = nullptr;
	int         spot_light_count = 0;

	DirectionalLight * directional_lights      = nullptr;
	int                directional_light_count = 0;

	Vector3 ambient_lighting = Vector3(0.1f);
	
	Sky skybox;

	void       trace_primitives    (const Ray & ray, RayHit & ray_hit) const;
	SIMD_float intersect_primitives(const Ray & ray, SIMD_float max_distance) const;
	
	SIMD_Vector3 bounce(const Ray & ray, int bounces_left, SIMD_float & distance) const;

public:
	Camera camera;

	Scene();
	~Scene();

	void update(float delta);
	
	void render_tile(const Window & window, int x, int y) const;
};
