#pragma once
#include "Primitive.h"
#include "MeshData.h"

#include "Ray.h"
#include "RayHit.h"

struct Mesh : Primitive {
	const MeshData * mesh_data;

	inline Mesh(const char * file_path) : mesh_data(MeshData::load(file_path)) { }

	void trace(const Ray & ray, RayHit & ray_hit) const;
	bool intersect(const Ray & ray, float max_distance) const;
};
