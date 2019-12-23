#pragma once
#include "BVH.h"
#include "Material.h"

struct MeshData {
	BVH<Triangle> triangle_bvh;

	Material * materials;

	static const MeshData * load(const char * file_path);
};
