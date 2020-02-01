#pragma once
#include "Material.h"
#include "Transform.h"

struct Primitive {
	int material_id = Material::add_material();
	Transform transform;
};
