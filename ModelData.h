#pragma once
#include "MeshData.h"

struct ModelData {
	const MeshData * mesh_data       = nullptr;
	int              mesh_data_count = 0;

	static const ModelData * load(const char * file_path);
};
