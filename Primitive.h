#pragma once
#include "Material.h"
#include "Transform.h"

struct Primitive {
	int material_id = MaterialBuffer::reserve();
	Transform transform;
};
