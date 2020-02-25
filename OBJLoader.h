#pragma once
#include "BottomLevelBVH.h"

namespace OBJLoader {
	void load_mtl(BottomLevelBVH * bvh, const char * filename);

	const Triangle * load_obj(BottomLevelBVH * bvh, const char * filename);
}
