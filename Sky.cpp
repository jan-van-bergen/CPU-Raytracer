#include "Sky.h"

#include <cassert>
#include <fstream>
#include <algorithm>

#include "Util.h"

Sky::Sky(const char * file_path, int width, int height) : width(width), height(height) {
	std::ifstream file(file_path, std::ios::in | std::ios::binary); 
	if (!file.is_open()) abort();

	data = new Vector3[width * height];
	file.read(reinterpret_cast<char *>(data), width * height * sizeof(Vector3));
	file.close();
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

#if SIMD_LANE_SIZE == 4
	__m128i x = _mm_cvtps_epi32((u * data_width).data);
	__m128i y = _mm_cvtps_epi32((v * data_height).data);

	union { __m128i index; int indices[4]; };
	index = _mm_add_epi32(_mm_mullo_epi32(y, _mm_set1_epi32(height)), x);
	index = _mm_max_epi32(index, _mm_set1_epi32(0));
	index = _mm_min_epi32(index, _mm_set1_epi32(width * height));

	return one_over_pi * SIMD_Vector3(
		data[indices[3]],
		data[indices[2]],
		data[indices[1]],
		data[indices[0]]
	);
#elif SIMD_LANE_SIZE == 8
	__m256i x = _mm256_cvtps_epi32((u * data_width).data);
	__m256i y = _mm256_cvtps_epi32((v * data_height).data);

	union { __m256i index; int indices[8]; };
	index = _mm256_add_epi32(_mm256_mullo_epi32(y, _mm256_set1_epi32(height)), x);
	index = _mm256_max_epi32(index, _mm256_set1_epi32(0));
	index = _mm256_min_epi32(index, _mm256_set1_epi32(width * height));

	return one_over_pi * SIMD_Vector3(
		data[indices[7]],
		data[indices[6]],
		data[indices[5]],
		data[indices[4]],
		data[indices[3]],
		data[indices[2]],
		data[indices[1]],
		data[indices[0]]
	);
#endif
}
