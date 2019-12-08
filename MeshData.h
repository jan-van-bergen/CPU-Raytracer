#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Material.h"

struct MeshData {
	Vector3 * positions;
	Vector2 * tex_coords;
	Vector3 * normals;
	int     * material_ids;
	int       vertex_count;

	Material * materials;

	static const MeshData * load(const char * file_path);
};
