#pragma once

#include "Ray.h"
#include "RayHit.h"

template<typename PrimitiveType>
struct PrimitiveList {
	PrimitiveType * primitives = nullptr;
	int             primitive_count;

	inline PrimitiveList(int count) : primitive_count(count) { 
		if (primitive_count > 0) {
			primitives = new PrimitiveType[primitive_count];
		}
	}

	inline ~PrimitiveList() {
		if (primitives) {
			delete[] primitives;
		}
	}

	inline void update() const {
		for (int i = 0; i < primitive_count; i++) {
			primitives[i].update();
		}
	}

	inline void trace(const Ray & ray, RayHit & ray_hit) const {
		for (int i = 0; i < primitive_count; i++) {
			primitives[i].trace(ray, ray_hit);
		}
	}

	inline SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const {
		SIMD_float result(0.0f);

		for (int i = 0; i < primitive_count; i++) {
			result = result | primitives[i].intersect(ray, max_distance);

			if (SIMD_float::all_true(result)) break;
		}

		return result;
	}

	inline PrimitiveType & operator[](int index) { 
		assert(index >= 0 && index < primitive_count);
		return primitives[index]; 
	}
};
