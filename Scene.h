#pragma once
#include "Sphere.h"
#include "Plane.h"
#include "Mesh.h"
#include "PrimitiveList.h"

#include "PointLight.h"
#include "SpotLight.h"
#include "DirectionalLight.h"

#include "Camera.h"

#include "Skybox.h"

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
	
	Skybox skybox;

	void trace_primitives    (const Ray & ray, RayHit & ray_hit) const;
	bool intersect_primitives(const Ray & ray, float max_distance) const;
	
	struct BounceResult {
		Vector3 colour;
		float   distance; // Used for Beer's Law
	};
	
	BounceResult bounce(const Ray & ray, int bounces_left) const;

public:
	Camera camera;

	Scene();
	~Scene();

	void update(float delta);
	
	void render_tile(const Window & window, int x, int y) const;
};
