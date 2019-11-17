#pragma once
#include "Primitive.h"
#include "MeshData.h"

#include "Ray.h"
#include "RayHit.h"

struct Mesh : Primitive {
	const MeshData * mesh_data;

	Vector3 * world_positions;
	Vector3 * world_normals;

	inline void init(const char * file_path)  {
		mesh_data = MeshData::load(file_path);
		world_positions = new Vector3[mesh_data->vertex_count];
		world_normals   = new Vector3[mesh_data->vertex_count];
	}

	inline ~Mesh() {
		delete[] world_positions;
		delete[] world_normals;
	}

	void update();

	void trace(const Ray & ray, RayHit & ray_hit) const;
	bool intersect(const Ray & ray, float max_distance) const;
};
