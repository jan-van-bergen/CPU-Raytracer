#pragma once
#include "Triangle.h"
#include "Material.h"

struct MeshData {
	int        triangle_count;
	Triangle * triangles;

	Material * materials;

	static const MeshData * load(const char * file_path);
};
