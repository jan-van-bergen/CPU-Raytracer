#pragma once
#include "Vector2.h"
#include "Vector3.h"

struct MeshData {
	Vector3 * positions;
	Vector2 * tex_coords;
	Vector3 * normals;
	int       vertex_count;

	static const MeshData * load(const char * file_path);
};