#pragma once
#include <cassert>

#include "ModelData.h"
#include "Transform.h"

#include "Ray.h"
#include "RayHit.h"

struct Mesh {
	const MeshData * mesh_data = nullptr;

	Vector3 * world_positions = nullptr;
	Vector3 * world_normals   = nullptr;

	inline void init(const MeshData * data)  {
		assert(world_positions == nullptr);
		assert(world_normals   == nullptr);

		mesh_data = data;
		world_positions = new Vector3[mesh_data->vertex_count];
		world_normals   = new Vector3[mesh_data->vertex_count];
	}

	inline ~Mesh() {
		delete[] world_positions;
		delete[] world_normals;
	}

	void update(const Matrix4 & world_matrix);

	void trace    (const Ray & ray, RayHit & ray_hit) const;
	bool intersect(const Ray & ray, __m128 max_distance) const;
};
