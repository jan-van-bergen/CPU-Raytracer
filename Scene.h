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
	PrimitiveList<Sphere> spheres;
	PrimitiveList<Plane>  planes;
	PrimitiveList<Mesh>   meshes;
	
	// Lights
	PointLight * point_lights      = nullptr;
	int          point_light_count = 0;

	SpotLight * spot_lights      = nullptr;
	int         spot_light_count = 0;

	DirectionalLight * directional_lights      = nullptr;
	int                directional_light_count = 0;

	Vector3 ambient_lighting = Vector3(0.1f);
	
	void trace_primitives    (const Ray & ray, RayHit & ray_hit) const;
	bool intersect_primitives(const Ray & ray, float max_distance) const;

public:
	Camera camera;

	Scene();
	~Scene();

	void update(float delta);

	void render(const Window & window) const;
};
