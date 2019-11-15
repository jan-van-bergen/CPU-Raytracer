#pragma once
#include "Sphere.h"
#include "Plane.h"
#include "Mesh.h"

#include "PointLight.h"
#include "SpotLight.h"
#include "DirectionalLight.h"

#include "Camera.h"

#include "Window.h"

struct Scene {
private:
	// Primitives
	Sphere * spheres      = nullptr;
	int      sphere_count = 0;

	Plane * planes      = nullptr;
	int     plane_count = 0;

	Mesh * meshes     = nullptr;
	int    mesh_count = 0;

	// Lights
	PointLight * point_lights      = nullptr;
	int          point_light_count = 0;

	SpotLight * spot_lights      = nullptr;
	int         spot_light_count = 0;

	DirectionalLight * directional_lights      = nullptr;
	int                directional_light_count = 0;

	Vector3 ambient_lighting = Vector3(0.1f);

public:
	Camera camera;

	Scene();
	~Scene();

	void trace_primitives(const Ray & ray, RayHit & ray_hit) const;
	bool intersect_primitives(const Ray & ray, float max_distance) const;

	void update(const Window & window) const;
};
