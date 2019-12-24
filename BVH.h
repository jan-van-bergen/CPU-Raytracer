#pragma once
#include <algorithm>

#include "BVHBuilders.h"

#define BVH_TRAVERSE_BRUTE_FORCE  0 // Doesn't use the tree structure of the BVH, checks every Primitive for every Ray
#define BVH_TRAVERSE_TREE_NAIVE   1 // Traverses the BVH in a naive way, always checking the left Node before the right Node
#define BVH_TRAVERSE_TREE_ORDERED 2 // Traverses the BVH based on the split axis and the direction of the Ray

#define BVH_TRAVERSAL_STRATEGY BVH_TRAVERSE_TREE_ORDERED

#define BVH_AXIS_X_BITS 0x40000000 // 01 00 zeroes...
#define BVH_AXIS_Y_BITS 0x80000000 // 10 00 zeroes...
#define BVH_AXIS_Z_BITS 0xc0000000 // 11 00 zeroes...
#define BVH_AXIS_MASK   0xc0000000 // 11 00 zeroes...

template<typename PrimitiveType>
struct BVHNode {
	AABB aabb;
	union {  // A Node can either be a leaf or have children. A leaf Node means count = 0
		int left;  // Left contains index of left child if the current Node is not a leaf Node
		int first; // First constains index of first primtive if the current Node is a leaf Node
	};
	int count; // Stores split axis in its 2 highest bits, count in its lowest 30 bits

	inline bool is_leaf() const {
		return (count & (~BVH_AXIS_MASK)) > 0;
	}

	inline bool should_visit_left_first(const Ray & ray) const {
#if BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_TREE_NAIVE
		return true;
#elif BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_TREE_ORDERED
		switch (count & BVH_AXIS_MASK) {
			case BVH_AXIS_X_BITS: return ray.direction.x[0] > 0.0f;
			case BVH_AXIS_Y_BITS: return ray.direction.y[0] > 0.0f;
			case BVH_AXIS_Z_BITS: return ray.direction.z[0] > 0.0f;

			default: abort();
		}
#endif
	}
};

template<typename PrimitiveType>
struct BVH {
	PrimitiveType * primitives;
	int             primitive_count;

	int * indices_x;
	int * indices_y;
	int * indices_z;

	BVHNode<PrimitiveType> * nodes;

	inline void init(int count) {
		assert(count > 0);

		primitive_count = count; 
		primitives = new PrimitiveType[primitive_count];

		int overallocation = 2; // SBVH requires more space

		// Construct index array
		int * all_indices  = new int[3 * overallocation * primitive_count];
		indices_x = all_indices;
		indices_y = all_indices + primitive_count * overallocation;
		indices_z = all_indices + primitive_count * overallocation * 2;

		for (int i = 0; i < primitive_count; i++) {
			indices_x[i] = i;
			indices_y[i] = i;
			indices_z[i] = i;
		}

		// Construct Node pool
		nodes = reinterpret_cast<BVHNode<PrimitiveType> *>(ALLIGNED_MALLOC(2 * primitive_count * sizeof(BVHNode<PrimitiveType>), 64));
		assert((unsigned long long)nodes % 64 == 0);
	}

	inline void build_bvh() {
		float * sah = new float[primitive_count];
		
		std::sort(indices_x, indices_x + primitive_count, [&](int a, int b) { return primitives[a].get_position().x < primitives[b].get_position().x; });
		std::sort(indices_y, indices_y + primitive_count, [&](int a, int b) { return primitives[a].get_position().y < primitives[b].get_position().y; });
		std::sort(indices_z, indices_z + primitive_count, [&](int a, int b) { return primitives[a].get_position().z < primitives[b].get_position().z; });
		
		int * indices[3] = { indices_x, indices_y, indices_z };

		int * temp = new int[primitive_count];

		int node_index = 2;
		BVHBuilders::build_bvh(nodes[0], primitives, indices, nodes, node_index, 0, primitive_count, sah, temp);

		assert(node_index <= 2 * primitive_count);

		delete [] temp;
		delete [] sah;
	}
	
	inline void build_sbvh() {
		float * sah = new float[primitive_count];
		
		std::sort(indices_x, indices_x + primitive_count, [&](int a, int b) { return primitives[a].get_position().x < primitives[b].get_position().x; });
		std::sort(indices_y, indices_y + primitive_count, [&](int a, int b) { return primitives[a].get_position().y < primitives[b].get_position().y; });
		std::sort(indices_z, indices_z + primitive_count, [&](int a, int b) { return primitives[a].get_position().z < primitives[b].get_position().z; });
		
		int * indices[3] = { indices_x, indices_y, indices_z };

		int * temp[2] = { new int[primitive_count], new int[primitive_count] };

		AABB root_aabb = BVHPartitions::calculate_bounds(primitives, indices[0], 0, primitive_count);

		int node_index = 2;
		int leaf_count = BVHBuilders::build_sbvh(nodes[0], primitives, indices, nodes, node_index, 0, primitive_count, sah, temp, 1.0f / root_aabb.surface_area(), root_aabb);

		printf("Leaf count: %i\n", leaf_count);

		assert(node_index <= 2 * primitive_count);

		delete [] temp[0];
		delete [] temp[1];
		delete [] sah;
	}

	inline void update() const {
		for (int i = 0; i < primitive_count; i++) {
			primitives[i].update();
		}
	}

	inline void trace(const Ray & ray, RayHit & ray_hit, const Matrix4 & world) const {
#if BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_BRUTE_FORCE
		for (int i = 0; i < primitive_count; i++) {
			primitives[i].trace(ray, ray_hit, world, 0);
		}
#elif BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_TREE_NAIVE || BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_TREE_ORDERED
		const BVHNode<PrimitiveType> * stack[128];
		int                            stack_size = 1;

		stack[0] = &nodes[0];

		int step = 0;

		while (stack_size > 0) {
			// Pop Node of the stack
			const BVHNode<PrimitiveType> * node = stack[--stack_size];

			SIMD_float mask = node->aabb.intersect(ray, ray_hit.distance);
			if (SIMD_float::all_false(mask)) continue;

			if (node->is_leaf()) {
				for (int i = node->first; i < node->first + node->count; i++) {
					primitives[indices_x[i]].trace(ray, ray_hit, world, step);
				}
			} else {
				if (node->should_visit_left_first(ray)) {
					// Visit left Node first, then visit right Node
					stack[stack_size++] = &nodes[node->left];
					stack[stack_size++] = &nodes[node->left+1];
				} else {
					// Visit right Node first, then visit left Node
					stack[stack_size++] = &nodes[node->left+1];
					stack[stack_size++] = &nodes[node->left];
				}
			}

			step++;
		}
#endif
	}

	inline SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const {
#if BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_BRUTE_FORCE
		SIMD_float result(0.0f);

		for (int i = 0; i < primitive_count; i++) {
			result = result | primitives[i].intersect(ray, max_distance);

			if (SIMD_float::all_true(result)) break;
		}

		return result;
#elif BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_TREE_NAIVE || BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_TREE_ORDERED
		const BVHNode<PrimitiveType> * stack[128];
		int                            stack_size = 1;

		stack[0] = &nodes[0];

		int step = 0;

		SIMD_float hit(0.0f);

		while (stack_size > 0) {
			// Pop Node of the stack
			const BVHNode<PrimitiveType> * node = stack[--stack_size];

			SIMD_float mask = node->aabb.intersect(ray, max_distance);
			if (SIMD_float::all_false(mask)) continue;

			if (node->is_leaf()) {
				for (int i = node->first; i < node->first + node->count; i++) {
					hit = hit | primitives[indices_x[i]].intersect(ray, max_distance);

					if (SIMD_float::all_true(hit)) return hit;
				}
			} else {
				if (node->should_visit_left_first(ray)) {
					stack[stack_size++] = &nodes[node->left];
					stack[stack_size++] = &nodes[node->left+1];
				} else {
					stack[stack_size++] = &nodes[node->left+1];
					stack[stack_size++] = &nodes[node->left];
				}
			}

			step++;
		}

		return hit;
#endif
	}
};
