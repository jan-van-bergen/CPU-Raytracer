#pragma once
#include <new>

#include "Material.h"
#include "Transform.h"

struct Primitive {
	Material material;
	Transform transform;
};
