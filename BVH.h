#pragma once
#include <cassert>
#include <algorithm>

#include "AABB.h"

#include "PrimitiveList.h"

#include "ScopedTimer.h"

#define TRAVERSE_BRUTE_FORCE  0 // Doesn't use the tree structure of the BVH, checks every Primitive for every Ray
#define TRAVERSE_TREE_NAIVE   1 // Traverses the BVH in a naive way, always checking the left Node before the right Node
#define TRAVERSE_TREE_ORDERED 2 // Traverses the BVH based on the split axis and the direction of the Ray

#define TRAVERSAL_STRATEGY TRAVERSE_TREE_ORDERED

#define AXIS_X_BITS 0x40000000 // 01 00 zeroes...
#define AXIS_Y_BITS 0x80000000 // 10 00 zeroes...
#define AXIS_Z_BITS 0xc0000000 // 11 00 zeroes...
#define AXIS_MASK   0xc0000000 // 11 00 zeroes...

inline static int dimension_bits[3] = { AXIS_X_BITS, AXIS_Y_BITS, AXIS_Z_BITS };

template<typename PrimitiveType>
inline AABB calculate_bounds(const PrimitiveType * primitives, const int * indices, int first, int last) {
	AABB aabb;
	aabb.min = Vector3(+INFINITY);
	aabb.max = Vector3(-INFINITY);

	// Iterate over relevant Primitives
	for (int i = first; i < last; i++) {
		aabb.expand(primitives[indices[i]].aabb);
	}

	// Make sure the AABB is non-zero along every dimension
	for (int d = 0; d < 3; d++) {
		if (aabb.max[d] - aabb.min[d] < 0.001f) {
			aabb.max[d] += 0.005f;
		}
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

	inline int partition_median(const PrimitiveType * primitives, int * indices, int first_index, int index_count) const {
		float max_axis_length = -INFINITY;
		int min_split_dimension = -1;

		// Check splits along all 3 dimensions
		for (int dimension = 0; dimension < 3; dimension++) {
			// Sort along current dimension
			std::sort(indices + first_index, indices + first_index + index_count, [&](int a, int b) {
				return primitives[a].get_position()[dimension] < primitives[b].get_position()[dimension];	
			});

			float min = primitives[indices[first_index                  ]].get_position()[dimension];
			float max = primitives[indices[first_index + index_count - 1]].get_position()[dimension];

			float axis_length = max - min;
			if (axis_length > max_axis_length) {
				max_axis_length = axis_length;
				min_split_dimension = dimension;
			}
		}
		
		// Sort indices so that they are sorted along the dimension that we want to split in
		// This is only required if we didn't split along z
		if (min_split_dimension != 2) {
			std::sort(indices + first_index, indices + first_index + index_count, [&](int a, int b) {
				return primitives[a].get_position()[min_split_dimension] < primitives[b].get_position()[min_split_dimension];	
			});
		}

		return first_index + (index_count >> 1);
	}

	inline int partition_sah(const PrimitiveType * primitives, int ** indices, int first_index, int index_count, float parent_cost, float * sah, int * temp, int & split_dimension) const {
		float min_cost = INFINITY;
		int   min_split_index     = -1;
		int   min_split_dimension = -1;

		// Check splits along all 3 dimensions
		for (int dimension = 0; dimension < 3; dimension++) {
			// First traverse left to right along the current dimension to evaluate first half of the SAH
			AABB aabb_left;
			aabb_left.min = Vector3(+INFINITY);
			aabb_left.max = Vector3(-INFINITY);
			for (int i = 0; i < index_count - 1; i++) {
				aabb_left.expand(primitives[indices[dimension][first_index + i]].aabb);
				
				sah[i] = aabb_left.surface_area() * float(i + 1);
			}

			// Then traverse right to left along the current dimension to evaluate second half of the SAH
			AABB aabb_right;
			aabb_right.min = Vector3(+INFINITY);
			aabb_right.max = Vector3(-INFINITY);
			for (int i = index_count - 1; i > 0; i--) {
				aabb_right.expand(primitives[indices[dimension][first_index + i]].aabb);

				sah[i - 1] += aabb_right.surface_area() * float(index_count - i);
			}

			// Find the minimum of the SAH
			for (int i = 0; i < index_count - 1; i++) {
				float cost = sah[i];
				if (cost < min_cost) {
					min_cost = cost;
					min_split_index = first_index + i + 1;
					min_split_dimension = dimension;
				}
			}
		}

		if (min_split_index == -1) abort();

		// Check SAH termination condition
		if (min_cost >= parent_cost) return -1;

		float split = primitives[indices[min_split_dimension][min_split_index]].get_position()[min_split_dimension];

		// Sort indices so that they are sorted along the dimension that we want to split in
		// This is only required if we didn't split along z
		for (int dimension = 0; dimension < 3; dimension++) {
			if (dimension != min_split_dimension) {
				int left  = first_index;
				int right = min_split_index;

				for (int i = first_index; i < first_index + index_count; i++) {
					bool goes_left = primitives[indices[dimension][i]].get_position()[min_split_dimension] < split;

					if (primitives[indices[dimension][i]].get_position()[min_split_dimension] == split) {
						// In case the current primitive has the same coordianate as the one we split on along the split dimension,
						// We don't know whether the primitive should go left or right.
						// In this case check all primitive indices on the left side of the split that 
						// have the same split coordinate for equality with the current primitive index i

						int j = min_split_index - 1;
						// While we can go left and the left primitive has the same coordinate along the split dimension as the split itself
						while (j >= 0 && primitives[indices[min_split_dimension][j]].get_position()[min_split_dimension] == split) {
							if (indices[min_split_dimension][j] == indices[dimension][i]) {
								goes_left = true;

								break;
							}

							j--;
						}
					}

					if (goes_left) {					
						temp[left++]  = indices[dimension][i];
					} else {
						temp[right++] = indices[dimension][i];
					}
				}

				assert(left  == min_split_index);
				assert(right == first_index + index_count);

				memcpy(indices[dimension] + first_index, temp + first_index, index_count * sizeof(int));
			}
		}

		split_dimension = (min_split_dimension + 1) << 30;

		return min_split_index;
	}

	inline void subdivide(const PrimitiveType * primitives, int ** indices, BVHNode nodes[], int & node_index, int first_index, int index_count, float * sah, int * temp) {
		aabb = calculate_bounds(primitives, indices[0], first_index, first_index + index_count);
		
		if (index_count < 3) {
			// Leaf Node, terminate recursion
			first = first_index;
			count = index_count;

			return;
		}
		
		left = node_index;
		node_index += 2;
		
		//count = 0;

		// Calculate cost of the current Node, used to determine termination
		float cost = aabb.surface_area() * float(index_count); 

		int split_dimension;
		int split_index = partition_sah(primitives, indices, first_index, index_count, cost, sah, temp, split_dimension);
		//int split_index = partition_median(primitives, indices, first_index, index_count);

		if (split_index == -1) {
			// Leaf Node, terminate recursion
			first = first_index;
			count = index_count;

			return;
		}
		
		count = split_dimension;

		int n_left  = split_index - first_index;
		int n_right = first_index + index_count - split_index;

		nodes[left    ].subdivide(primitives, indices, nodes, node_index, first_index,          n_left,  sah, temp);
		nodes[left + 1].subdivide(primitives, indices, nodes, node_index, first_index + n_left, n_right, sah, temp);
	}

	inline bool is_leaf() const {
		return (count & (~AXIS_MASK)) > 0;
	}

	inline bool should_visit_left_first(const Ray & ray) const {
#if TRAVERSAL_STRATEGY == TRAVERSE_TREE_NAIVE
		return true;
#elif TRAVERSAL_STRATEGY == TRAVERSE_TREE_ORDERED
		switch (count & AXIS_MASK) {
			case AXIS_X_BITS: return ray.direction.x[0] > 0.0f;
			case AXIS_Y_BITS: return ray.direction.y[0] > 0.0f;
			case AXIS_Z_BITS: return ray.direction.z[0] > 0.0f;

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
#if TRAVERSAL_STRATEGY == TRAVERSE_BRUTE_FORCE
		for (int i = 0; i < primitive_count; i++) {
			primitives[i].trace(ray, ray_hit, 0);
		}
#else
		nodes[0].trace(primitives, indices_x, nodes, ray, ray_hit, 0);
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
		return nodes[0].intersect(primitives, indices_x, nodes, ray, max_distance);
#endif
	}
};
