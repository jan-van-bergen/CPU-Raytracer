#pragma once
#include "Primitive.h"
#include "ModelData.h"
#include "Mesh.h"

// A Model can consist of a set of Meshes
struct Model : Primitive {
	Mesh * meshes;
	int    mesh_count;

	inline void init(const char * file_path) {
		const ModelData * model_data = ModelData::load(file_path);

		mesh_count = model_data->mesh_data_count;
		meshes = new Mesh[mesh_count];

		for (int m = 0; m < mesh_count; m++) {
			meshes[m].init(model_data->mesh_data + m);
		}
	}

	inline void update() {
		transform.calc_world_matrix();

		for (int m = 0; m < mesh_count; m++) {
			meshes[m].update(transform.world_matrix);
		}
	}

	inline void trace(const Ray & ray, RayHit & ray_hit) const {
		for (int m = 0; m < mesh_count; m++) {
			meshes[m].trace(ray, ray_hit);
		}
	}

	inline SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const {
		SIMD_float result(0.0f);

		for (int m = 0; m < mesh_count; m++) {
			result = result | meshes[m].intersect(ray, max_distance);

			if (SIMD_float::all_true(result)) break;
		}

		return result;
	}
};
