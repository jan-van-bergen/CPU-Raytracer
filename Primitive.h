#pragma once
#include "Material.h"

#include "Ray.h"
#include "RayHit.h"

struct Primitive {
	Material material;
};

template<typename PrimitiveType>
struct PrimitiveList {
	PrimitiveType * primitives;
	int             primitive_count;

	inline PrimitiveList(int count) : primitive_count(count) { 
		// Allocate as a byte array in order to avoid default constructor issues
		primitives = reinterpret_cast<PrimitiveType *>(new unsigned char[primitive_count * sizeof(PrimitiveType)]);
	}

	inline ~PrimitiveList() {
		delete[] primitives;
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

	inline PrimitiveType & operator[](int index) { return primitives[index]; }
};
