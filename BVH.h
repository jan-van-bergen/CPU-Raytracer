#pragma once
#include <cassert>
#include <algorithm>

#include "AABB.h"

#include "PrimitiveList.h"

template<typename PrimitiveType>
inline AABB calculate_bounds(const PrimitiveList<PrimitiveType> & primitives, const int * indices, int first, int last);

template<>
inline AABB calculate_bounds(const PrimitiveList<Model> & primitives, const int * indices, int first, int last) {
	AABB aabb;
	aabb.min = Vector3(+INFINITY);
	aabb.max = Vector3(-INFINITY);

	// Iterate over relevant Primitives
	for (int i = first; i < last; i++) {
		int index = indices[i];

		// Iterate over sub-Meshes of the current Model
		for (int m = 0; m < primitives[index].mesh_count; m++) {
			// Iterate over Triangles of current sub-Mesh
			for (int t = 0; t < primitives[index].meshes[m].mesh_data->vertex_count; t += 3) {
				Vector3 position0 = primitives[index].meshes[m].world_positions[t];
				Vector3 position1 = primitives[index].meshes[m].world_positions[t+1];
				Vector3 position2 = primitives[index].meshes[m].world_positions[t+2];
				
 				aabb.min = Vector3::min(Vector3::min(aabb.min, position0), Vector3::min(position1, position2));
				aabb.max = Vector3::max(Vector3::max(aabb.max, position0), Vector3::max(position1, position2));
			}
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

	inline int partition(const PrimitiveList<PrimitiveType> & primitives, int * indices, int first_index, int index_count, float parent_cost) {
		float min_cost = INFINITY;
		int   min_split_index     = -1;
		int   min_split_dimension = -1;

		// Check splits along all 3 dimensions
		for (int dimension = 0; dimension < 3; dimension++) {
			// Sort along current dimension
			std::sort(indices + first_index, indices + first_index + index_count, [&](int a, int b) {
				return primitives[a].transform.position[dimension] < primitives[b].transform.position[dimension];	
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

		std::sort(indices + first_index, indices + first_index + index_count, [&](int a, int b) {
			return primitives[a].transform.position[min_split_dimension] < primitives[b].transform.position[min_split_dimension];	
		});

		return min_split_index;
	}

	inline void subdivide(const PrimitiveList<PrimitiveType> & primitives, int * indices, BVHNode nodes[], int & node_index, int first_index, int index_count) {
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

	inline void trace(const PrimitiveList<PrimitiveType> & primitives, const int * indices, const BVHNode nodes[], const Ray & ray, RayHit & ray_hit) const {
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

	inline SIMD_float intersect(const PrimitiveList<PrimitiveType> & primitives, const int * indices, const BVHNode nodes[], const Ray & ray, SIMD_float max_distance) const {
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
	PrimitiveList<PrimitiveType> primitives;

	int * indices;

	BVHNode<PrimitiveType> * nodes;

	inline BVH(int primitive_count) : primitives(primitive_count) { }

	inline void init() {
		// Construct index array
		indices = new int[primitives.primitive_count];
		for (int i = 0; i < primitives.primitive_count; i++) {
			indices[i] = i;
		}

		// Construct Node pool
		nodes = reinterpret_cast<BVHNode<PrimitiveType> *>(ALLIGNED_MALLOC(2 * primitives.primitive_count * sizeof(BVHNode<PrimitiveType>), 64));
		
		assert((unsigned long long)nodes % 64 == 0);

		int node_index = 2;
		nodes[0].subdivide(primitives, indices, nodes, node_index, 0, primitives.primitive_count);
	}

	inline void trace(const Ray & ray, RayHit & ray_hit) const {
		nodes[0].trace(primitives, indices, nodes, ray, ray_hit);
	}

	inline SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const {
		return nodes[0].intersect(primitives, indices, nodes, ray, max_distance);
	}
};
