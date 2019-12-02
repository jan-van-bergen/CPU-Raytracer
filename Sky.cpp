#include "Sky.h"

#include <cstdio>
#include <stdlib.h>

#include "Util.h"

Sky::Sky(const char * file_path, int width, int height) : width(width), height(height) {
	FILE * file; fopen_s(&file, file_path, "rb");

	if (!file) abort();

	data = new Vector3[width * height];
	fread(reinterpret_cast<char *>(data), sizeof(Vector3), width * height, file);
	fclose(file);
}

SIMD_Vector3 Sky::sample(const SIMD_Vector3 & direction) const {
	const SIMD_float one_over_pi(ONE_OVER_PI);
	const SIMD_float half(0.5f);
	const SIMD_float data_width (width);
	const SIMD_float data_height(height);

	// Formula as described on https://www.pauldebevec.com/Probes/
    SIMD_float r = half * one_over_pi * SIMD_float::acos(direction.z) * SIMD_float::inv_sqrt(direction.x*direction.x + direction.y*direction.y);

	SIMD_float u = SIMD_float::madd(direction.x, r, half);
	SIMD_float v = SIMD_float::madd(direction.y, r, half);

	// Convert to pixel coordinates
	SIMD_int x = SIMD_float_to_int(u * data_width);
	SIMD_int y = SIMD_float_to_int(v * data_height);

	SIMD_int index = y * SIMD_int(height) + x;
	index = SIMD_int::max(index, SIMD_int(0));
	index = SIMD_int::min(index, SIMD_int(width * height));

#if SIMD_LANE_SIZE == 4
	return one_over_pi * SIMD_Vector3(
		data[index[3]],
		data[index[2]],
		data[index[1]],
		data[index[0]]
	);
#elif SIMD_LANE_SIZE == 8
	return one_over_pi * SIMD_Vector3(
		data[index[7]],
		data[index[6]],
		data[index[5]],
		data[index[4]],
		data[index[3]],
		data[index[2]],
		data[index[1]],
		data[index[0]]
	);
#endif
}
