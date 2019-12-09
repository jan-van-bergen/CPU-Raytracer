#pragma once
#include <cassert>
#include <algorithm>

#include "AABB.h"

#include "PrimitiveList.h"

#include "ScopedTimer.h"

#define TRAVERSE_BRUTE_FORCE 0
#define TRAVERSE_TREE_NAIVE  1
#define TRAVERSE_ORDERED     2

#define TRAVERSAL_STRATEGY TRAVERSE_TREE_NAIVE

template<typename PrimitiveType>
inline AABB calculate_bounds(const PrimitiveType * primitives, const int * indices, int first, int last) {
	AABB aabb;
	aabb.min = Vector3(+INFINITY);
	aabb.max = Vector3(-INFINITY);

	// Iterate over relevant Primitives
	for (int i = first; i < last; i++) {
		primitives[indices[i]].expand(aabb);
	}

	return aabb;
} 

template<typename PrimitiveType>
struct BVHNode {
	AABB aabb;
	union {  // A Node can either be a leaf or have children. A leaf Node means count = 0
		int left;  // Left contains index of left child if the current Node is not a leaf Node
		int first; // First constains index of first primtive if the current Node is a leaf Node
	};
	int count;

	inline int partition(const PrimitiveType * primitives, int * indices, int first_index, int index_count, float parent_cost) {
		float min_cost = INFINITY;
		int   min_split_index     = -1;
		int   min_split_dimension = -1;

		// Check splits along all 3 dimensions
		for (int dimension = 0; dimension < 3; dimension++) {
			// Sort along current dimension
			std::sort(indices + first_index, indices + first_index + index_count, [&](int a, int b) {
				return primitives[a].get_position()[dimension] < primitives[b].get_position()[dimension];	
			});

			for (int i = first_index + 1; i < first_index + index_count; i++) {
				AABB aabb_left  = calculate_bounds(primitives, indices, first_index, i);
				AABB aabb_right = calculate_bounds(primitives, indices, i, first_index + index_count);

				float area_left  = aabb_left.surface_area();
				float area_right = aabb_right.surface_area();

				int n_left  = i - first_index;
				int n_right = first_index + index_count - i;

				float cost = area_left * float(n_left) + area_right * float(n_right);
				if (cost < min_cost) {
					min_cost = cost;
					min_split_index = i;
					min_split_dimension = dimension;
				}
			}
		}
		
		if (min_split_index == -1) abort();

		// Check SAH termination condition
		if (min_cost >= parent_cost) return -1;

		// Sort indices so that they are sorted along the dimension that we want to split in
		std::sort(indices + first_index, indices + first_index + index_count, [&](int a, int b) {
			return primitives[a].get_position()[min_split_dimension] < primitives[b].get_position()[min_split_dimension];	
		});

		return min_split_index;
	}

	inline void subdivide(const PrimitiveType * primitives, int * indices, BVHNode nodes[], int & node_index, int first_index, int index_count) {
		aabb = calculate_bounds(primitives, indices, first_index, first_index + index_count);
		
		if (index_count < 3) {
			// Leaf Node, terminate recursion
			first = first_index;
			count = index_count;

			return;
		}
		
		left = node_index;
		node_index += 2;
		
		count = 0;

		// Calculate cost of the current Node, used to determine termination
		float cost = aabb.surface_area() * float(index_count); 

		int split_index = partition(primitives, indices, first_index, index_count, cost);
		
		if (split_index == -1) {
			// Leaf Node, terminate recursion
			first = first_index;
			count = index_count;

			return;
		}

		int n_left  = split_index - first_index;
		int n_right = first_index + index_count - split_index;

		nodes[left    ].subdivide(primitives, indices, nodes, node_index, first_index,          n_left);
		nodes[left + 1].subdivide(primitives, indices, nodes, node_index, first_index + n_left, n_right);
	}

	inline void trace(const PrimitiveType * primitives, const int * indices, const BVHNode nodes[], const Ray & ray, RayHit & ray_hit) const {
		SIMD_float mask = aabb.intersect(ray, ray_hit.distance);
		if (SIMD_float::all_false(mask)) return;

		// Check if the current Node is a leaf
		if (count > 0) {
			for (int i = first; i < first + count; i++) {
				primitives[indices[i]].trace(ray, ray_hit);
			}
		} else {
			nodes[left    ].trace(primitives, indices, nodes, ray, ray_hit);
			nodes[left + 1].trace(primitives, indices, nodes, ray, ray_hit);
		}
	}

	inline SIMD_float intersect(const PrimitiveType * primitives, const int * indices, const BVHNode nodes[], const Ray & ray, SIMD_float max_distance) const {
		SIMD_float mask = aabb.intersect(ray, max_distance);
		if (SIMD_float::all_false(mask)) return mask;

		// Check if the current Node is a leaf
		if (count > 0) {
			SIMD_float hit(0.0f);

			for (int i = first; i < first + count; i++) {
				hit = hit | primitives[indices[i]].intersect(ray, max_distance);

				if (SIMD_float::all_true(hit)) return hit;
			}

			return hit;
		} else {
			SIMD_float hit = nodes[left].intersect(primitives, indices, nodes, ray, max_distance);

			if (SIMD_float::all_true(hit)) return hit;

			return hit | nodes[left + 1].intersect(primitives, indices, nodes, ray, max_distance);
		}
	}
};

template<typename PrimitiveType>
struct BVH {
	PrimitiveType * primitives;
	int             primitive_count;

	int * indices;

	BVHNode<PrimitiveType> * nodes;

	static_assert(sizeof(BVHNode<PrimitiveType>) == 32);

	inline void init(int count) {
		assert(count > 0);

		primitive_count = count; 
		primitives = new PrimitiveType[primitive_count];

		// Construct index array
		indices = new int[primitive_count];
		for (int i = 0; i < primitive_count; i++) {
			indices[i] = i;
		}

		// Construct Node pool
		nodes = reinterpret_cast<BVHNode<PrimitiveType> *>(ALLIGNED_MALLOC(2 * primitive_count * sizeof(BVHNode<PrimitiveType>), 64));
		assert((unsigned long long)nodes % 64 == 0);
	}

	inline void build() {
		ScopedTimer timer("BVH Construction");

		int node_index = 2;
		nodes[0].subdivide(primitives, indices, nodes, node_index, 0, primitive_count);

		assert(node_index <= 2 * primitive_count);
	}
	
	inline void update() const {
		for (int i = 0; i < primitive_count; i++) {
			primitives[i].update();
		}
	}

	inline void trace(const Ray & ray, RayHit & ray_hit) const {
#if TRAVERSAL_STRATEGY == TRAVERSE_BRUTE_FORCE
		for (int i = 0; i < primitive_count; i++) {
			primitives[i].trace(ray, ray_hit);
		}
#else
		nodes[0].trace(primitives, indices, nodes, ray, ray_hit);
#endif
	}

	inline SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const {
#if TRAVERSAL_STRATEGY == TRAVERSE_BRUTE_FORCE
		SIMD_float result(0.0f);

		for (int i = 0; i < primitive_count; i++) {
			result = result | primitives[i].intersect(ray, max_distance);

			if (SIMD_float::all_true(result)) break;
		}

		return result;
#else
		return nodes[0].intersect(primitives, indices, nodes, ray, max_distance);
#endif
	}
};
