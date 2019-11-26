#pragma once
#include "Primitive.h"
#include "ModelData.h"
#include "Mesh.h"

// A Model can consist of a set of Meshes
struct Model : Primitive {
	const ModelData * model_data = nullptr;

	Mesh * meshes;

	inline void init(const char * file_path) {
		model_data = ModelData::load(file_path);

		meshes = new Mesh[model_data->mesh_data_count];
		for (int m = 0; m < model_data->mesh_data_count; m++) {
			meshes[m].init(model_data->mesh_data + m);
		}
	}

	inline void update() {
		transform.calc_world_matrix();

		for (int m = 0; m < model_data->mesh_data_count; m++) {
			meshes[m].update(transform.world_matrix);
		}
	}

	inline void trace(const Ray & ray, RayHit & ray_hit) const {
		for (int m = 0; m < model_data->mesh_data_count; m++) {
			meshes[m].trace(ray, ray_hit);
		}
	}

	inline bool intersect(const Ray & ray, float max_distance) const {
		for (int m = 0; m < model_data->mesh_data_count; m++) {
			if (meshes[m].intersect(ray, max_distance)) {
				return true;
			}
		}

		return false;
	}
};
