#pragma once
#include "Texture.h"

#include "Vector3.h"
#include "SIMD.h"

struct Sky {
private:
	int size;
	Vector3 * data;

public:
	Sky(const char * file_name);

	SIMD_Vector3 sample(const SIMD_Vector3 & direction) const;
};
