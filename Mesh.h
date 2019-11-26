#pragma once
#include "Primitive.h"
#include "ModelData.h"

#include "Ray.h"
#include "RayHit.h"

struct Mesh {
	const MeshData * mesh_data = nullptr;

	float * world_positions_x = nullptr;
	float * world_positions_y = nullptr;
	float * world_positions_z = nullptr;
	Vector3 * world_normals   = nullptr;

	inline void init(const MeshData * data)  {
		//assert(world_positions == nullptr);
		assert(world_normals   == nullptr);

		mesh_data = data;
		world_positions_x = new float[mesh_data->vertex_count];
		world_positions_y = new float[mesh_data->vertex_count];
		world_positions_z = new float[mesh_data->vertex_count];
		world_normals     = new Vector3[mesh_data->vertex_count];
	}

	inline ~Mesh() {
		delete[] world_positions_x;
		delete[] world_positions_y;
		delete[] world_positions_z;
		delete[] world_normals;
	}

	void update(const Matrix4 & world_matrix);

	void trace    (const Ray & ray, RayHit & ray_hit) const;
	bool intersect(const Ray & ray, float max_distance) const;
};
