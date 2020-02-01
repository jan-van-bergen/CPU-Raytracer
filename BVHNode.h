#pragma once
#include "AABB.h"
#include "Ray.h"

#define BVH_TRAVERSE_TREE_NAIVE   0 // Traverses the BVH in a naive way, always checking the left Node before the right Node
#define BVH_TRAVERSE_TREE_ORDERED 1 // Traverses the BVH based on the split axis and the direction of the Ray

#define BVH_TRAVERSAL_STRATEGY BVH_TRAVERSE_TREE_ORDERED

#define BVH_AXIS_X_BITS 0x40000000 // 01 00 zeroes...
#define BVH_AXIS_Y_BITS 0x80000000 // 10 00 zeroes...
#define BVH_AXIS_Z_BITS 0xc0000000 // 11 00 zeroes...
#define BVH_AXIS_MASK   0xc0000000 // 11 00 zeroes...

struct BVHNode {
	AABB aabb;
	union {  // A Node can either be a leaf or have children. A leaf Node means count = 0
		int left;  // Left contains index of left child if the current Node is not a leaf Node
		int first; // First constains index of first primtive if the current Node is a leaf Node
	};
	int count; // Stores split axis in its 2 highest bits, count in its lowest 30 bits

	inline int get_count() const {
		return count & ~BVH_AXIS_MASK;
	}

	inline int get_axis() const {
		return count & BVH_AXIS_MASK;
	}

	inline bool is_leaf() const {
		return get_count() > 0;
	}

	inline bool should_visit_left_first(const Ray & ray) const {
#if BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_TREE_NAIVE
		return true; // Naive always goes left first
#elif BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_TREE_ORDERED
		switch (get_axis()) {
			case BVH_AXIS_X_BITS: return ray.direction.x[0] > 0.0f;
			case BVH_AXIS_Y_BITS: return ray.direction.y[0] > 0.0f;
			case BVH_AXIS_Z_BITS: return ray.direction.z[0] > 0.0f;
		}
#endif
	}
};
