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

	inline bool intersect(const Ray & ray, float max_distance) const {
		for (int i = 0; i < primitive_count; i++) {
			if (primitives[i].intersect(ray, max_distance)) {
				return true;
			}
		}

		return false;
	}

	inline PrimitiveType & operator[](int index) { 
		assert(index >= 0 && index < primitive_count);
		return primitives[index]; 
	}
};
