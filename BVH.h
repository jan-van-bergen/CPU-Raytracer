#pragma once
#include <cassert>
#include <algorithm>

#include "BVHConstructors.h"

#include "ScopedTimer.h"

#define BVH_TRAVERSE_BRUTE_FORCE  0 // Doesn't use the tree structure of the BVH, checks every Primitive for every Ray
#define BVH_TRAVERSE_TREE_NAIVE   1 // Traverses the BVH in a naive way, always checking the left Node before the right Node
#define BVH_TRAVERSE_TREE_ORDERED 2 // Traverses the BVH based on the split axis and the direction of the Ray

#define BVH_TRAVERSAL_STRATEGY BVH_TRAVERSE_TREE_NAIVE

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

	inline void subdivide(const PrimitiveType * primitives, int * indices[3], BVHNode nodes[], int & node_index, int first_index, int index_count, float * sah, int * temp) {
		aabb = BVHConstructors::calculate_bounds(primitives, indices[0], first_index, first_index + index_count);
		
		if (index_count < 3) {
			// Leaf Node, terminate recursion
			first = first_index;
			count = index_count;

			return;
		}
		
		left = node_index;
		node_index += 2;
		
		int split_dimension;
		float split_cost;
		int split_index = BVHConstructors::partition_object(primitives, indices, first_index, index_count, sah, temp, split_dimension, split_cost);

		// Check SAH termination condition
		float parent_cost = aabb.surface_area() * float(index_count); 
		if (split_cost >= parent_cost) {
			first = first_index;
			count = index_count;

			return;
		}

		float split = primitives[indices[split_dimension][split_index]].get_position()[split_dimension];
		BVHConstructors::split_indices(primitives, indices, first_index, index_count, temp, split_dimension, split_index, split);

		count = (split_dimension + 1) << 30;

		int n_left  = split_index - first_index;
		int n_right = first_index + index_count - split_index;

		nodes[left    ].subdivide(primitives, indices, nodes, node_index, first_index,          n_left,  sah, temp);
		nodes[left + 1].subdivide(primitives, indices, nodes, node_index, first_index + n_left, n_right, sah, temp);
	}

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

	inline void trace(const PrimitiveType * primitives, const int * indices, const BVHNode nodes[], const Ray & ray, RayHit & ray_hit, int step) const {
		SIMD_float mask = aabb.intersect(ray, ray_hit.distance);
		if (SIMD_float::all_false(mask)) return;

		if (is_leaf()) {
			for (int i = first; i < first + count; i++) {
				primitives[indices[i]].trace(ray, ray_hit, step + count);
			}
		} else {
			if (should_visit_left_first(ray)) {
				// Visit left Node first, then visit right Node
				nodes[left    ].trace(primitives, indices, nodes, ray, ray_hit, step + 1);
				nodes[left + 1].trace(primitives, indices, nodes, ray, ray_hit, step + 1);
			} else {
				// Visit right Node first, then visit left Node
				nodes[left + 1].trace(primitives, indices, nodes, ray, ray_hit, step + 1);
				nodes[left    ].trace(primitives, indices, nodes, ray, ray_hit, step + 1);
			}
		}
	}

	inline SIMD_float intersect(const PrimitiveType * primitives, const int * indices, const BVHNode nodes[], const Ray & ray, SIMD_float max_distance) const {
		SIMD_float mask = aabb.intersect(ray, max_distance);
		if (SIMD_float::all_false(mask)) return mask;

		if (is_leaf()) {
			SIMD_float hit(0.0f);

			for (int i = first; i < first + count; i++) {
				hit = hit | primitives[indices[i]].intersect(ray, max_distance);

				if (SIMD_float::all_true(hit)) return hit;
			}

			return hit;
		} else {
			if (should_visit_left_first(ray)) {
				SIMD_float hit = nodes[left].intersect(primitives, indices, nodes, ray, max_distance);

				if (SIMD_float::all_true(hit)) return hit;

				return hit | nodes[left + 1].intersect(primitives, indices, nodes, ray, max_distance);
			} else {
				SIMD_float hit = nodes[left + 1].intersect(primitives, indices, nodes, ray, max_distance);

				if (SIMD_float::all_true(hit)) return hit;

				return hit | nodes[left].intersect(primitives, indices, nodes, ray, max_distance);
			}
		}
	}

	inline void debug(FILE * file, const BVHNode nodes[], int & index) const {
		aabb.debug(file, index++);

		if (!is_leaf()) {
			nodes[left  ].debug(file, nodes, index);
			nodes[left+1].debug(file, nodes, index);
		}
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

		// Construct index array
		int * all_indices  = new int[3 * primitive_count];
		indices_x = all_indices;
		indices_y = all_indices + primitive_count;
		indices_z = all_indices + primitive_count * 2;

		for (int i = 0; i < primitive_count; i++) {
			indices_x[i] = i;
			indices_y[i] = i;
			indices_z[i] = i;
		}

		// Construct Node pool
		nodes = reinterpret_cast<BVHNode<PrimitiveType> *>(ALLIGNED_MALLOC(2 * primitive_count * sizeof(BVHNode<PrimitiveType>), 64));
		assert((unsigned long long)nodes % 64 == 0);
	}

	inline void build() {
		ScopedTimer timer("BVH Construction");

		float * sah = new float[primitive_count];
		
		std::sort(indices_x, indices_x + primitive_count, [&](int a, int b) { return primitives[a].get_position().x < primitives[b].get_position().x; });
		std::sort(indices_y, indices_y + primitive_count, [&](int a, int b) { return primitives[a].get_position().y < primitives[b].get_position().y; });
		std::sort(indices_z, indices_z + primitive_count, [&](int a, int b) { return primitives[a].get_position().z < primitives[b].get_position().z; });
		
		int * indices[3] = { indices_x, indices_y, indices_z };

		int * temp = new int[primitive_count];

		int node_index = 2;
		nodes[0].subdivide(primitives, indices, nodes, node_index, 0, primitive_count, sah, temp);

		assert(node_index <= 2 * primitive_count);

		delete [] temp;
		delete [] sah;
	}
	
	inline void update() const {
		for (int i = 0; i < primitive_count; i++) {
			primitives[i].update();
		}
	}

	inline void trace(const Ray & ray, RayHit & ray_hit) const {
#if BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_BRUTE_FORCE
		for (int i = 0; i < primitive_count; i++) {
			primitives[i].trace(ray, ray_hit, 0);
		}
#elif BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_TREE_NAIVE || BVH_TRAVERSAL_STRATEGY == BVH_TRAVERSE_TREE_ORDERED
		nodes[0].trace(primitives, indices_x, nodes, ray, ray_hit, 0);
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
		return nodes[0].intersect(primitives, indices_x, nodes, ray, max_distance);
#endif
	}

	inline void debug() const {
		FILE * file = nullptr;
		fopen_s(&file, DATA_PATH("debug.obj"), "w");

		if (file == nullptr) abort(); // Error opening file!

		int index = 0;
		nodes[0].debug(file, nodes, index);

		fclose(file);
	}
};
