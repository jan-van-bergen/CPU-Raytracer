#pragma once
#include <algorithm>

#include "BVHConstructors.h"

#include "ScopedTimer.h"

#define SBVH_CONSTRUCT_MEDIAN   0 // Split using median Primitive along the longest axis
#define SBVH_CONSTRUCT_FULL_SAH 1 // Evaluate SAH for every Primtive to determine if we should split there

#define SBVH_CONSTRUCTION_STRATEGY SBVH_CONSTRUCT_FULL_SAH

#define SBVH_TRAVERSE_BRUTE_FORCE  0 // Doesn't use the tree structure of the BVH, checks every Primitive for every Ray
#define SBVH_TRAVERSE_TREE_NAIVE   1 // Traverses the BVH in a naive way, always checking the left Node before the right Node
#define SBVH_TRAVERSE_TREE_ORDERED 2 // Traverses the BVH based on the split axis and the direction of the Ray

#define SBVH_TRAVERSAL_STRATEGY SBVH_TRAVERSE_TREE_NAIVE

#define SBVH_AXIS_X_BITS 0x40000000 // 01 00 zeroes...
#define SBVH_AXIS_Y_BITS 0x80000000 // 10 00 zeroes...
#define SBVH_AXIS_Z_BITS 0xc0000000 // 11 00 zeroes...
#define SBVH_AXIS_MASK   0xc0000000 // 11 00 zeroes...

struct SBVHNode {
	AABB aabb;
	union {  // A Node can either be a leaf or have children. A leaf Node means count = 0
		int left;  // Left contains index of left child if the current Node is not a leaf Node
		int first; // First constains index of first primtive if the current Node is a leaf Node
	};
	int count; // Stores split axis in its 2 highest bits, count in its lowest 30 bits

	inline int subdivide(const Triangle * triangles, int * indices[3], SBVHNode nodes[], int & node_index, int first_index, int index_count, float * sah, int * temp) {
		aabb = BVHConstructors::calculate_bounds(triangles, indices[0], first_index, first_index + index_count);
		
		if (index_count < 3) {
			// Leaf Node, terminate recursion
			first = first_index;
			count = index_count;

			return count;
		}
		
		left = node_index;
		node_index += 2;

		int   full_sah_split_dimension;
		float full_sah_split_cost;
		int   full_sah_split_index = BVHConstructors::partition_full_sah(triangles, indices, first_index, index_count, sah, temp, full_sah_split_dimension, full_sah_split_cost);

		int   spatial_split_dimension;
		float spatial_split_cost;
		float spatial_split_plane_distance;
		int   spatial_split_index = BVHConstructors::partition_spatial(triangles, indices, first_index, index_count, sah, temp, spatial_split_dimension, spatial_split_cost, spatial_split_plane_distance);

		int n_left  = -1;
		int n_right = -1;

		if (true) { //full_sah_split_cost < spatial_split_cost) {
			// Check SAH termination condition
			float parent_cost = aabb.surface_area() * float(index_count); 
			if (full_sah_split_cost >= parent_cost) {
				first = first_index;
				count = index_count;

				return count;
			}
			
			count = (full_sah_split_dimension + 1) << 30;

			float split = triangles[indices[full_sah_split_dimension][full_sah_split_index]].get_position()[full_sah_split_dimension];
			BVHConstructors::split_indices(triangles, indices, first_index, index_count, temp, full_sah_split_dimension, full_sah_split_index, split);
			
			n_left  = full_sah_split_index - first_index;
			n_right = first_index + index_count - full_sah_split_index;
		} else {
			// Check SAH termination condition
			float parent_cost = aabb.surface_area() * float(index_count); 
			if (spatial_split_cost >= parent_cost) {
				first = first_index;
				count = index_count;

				return count;
			}
			
			count = (spatial_split_dimension + 1) << 30;

			// @TODO: left is not needed and right can use the 'temp' array
			int * children_left [3];
			int * children_right[3];

			children_left[0]  = new int[index_count];
			children_left[1]  = new int[index_count];
			children_left[2]  = new int[index_count];
			children_right[0] = new int[index_count];
			children_right[1] = new int[index_count];
			children_right[2] = new int[index_count];

			int children_left_count [3] = { 0, 0, 0 };
			int children_right_count[3] = { 0, 0, 0 };

			Vector3 plane_normals[3] = {
				Vector3(1.0f, 0.0f, 0.0f),
				Vector3(0.0f, 1.0f, 0.0f),
				Vector3(0.0f, 0.0f, 1.0f),
			};

			Vector3 plane_normal = plane_normals[spatial_split_dimension];

			for (int dimension = 0; dimension < 3; dimension++) {
				for (int i = first_index; i < first_index + index_count; i++) {
					int index = indices[dimension][i];
					const Triangle & triangle = triangles[index];

					Vector3 i0, i1;
					Math::PlaneTriangleIntersection intersection = Math::plane_triangle_intersection(plane_normal, spatial_split_plane_distance, 
						triangle.position0,
						triangle.position1,
						triangle.position2,
						i0, i1
					);

					// @TODO: unsplitting

					if (intersection == Math::PlaneTriangleIntersection::LEFT || intersection == Math::PlaneTriangleIntersection::INTERSECTS) {
						children_left[dimension][children_left_count[dimension]++] = index;
					}
					if (intersection == Math::PlaneTriangleIntersection::RIGHT || intersection == Math::PlaneTriangleIntersection::INTERSECTS) {
						children_right[dimension][children_right_count[dimension]++] = index;
					}
				}
			}

			assert(children_left_count [0] == children_left_count [1] && children_left_count [1] == children_left_count [2]);
			assert(children_right_count[0] == children_right_count[1] && children_right_count[1] == children_right_count[2]);

			assert(children_left_count [0] < index_count);
			assert(children_right_count[0] < index_count);

			assert(children_left_count[0] + children_right_count[0] >= index_count);

			n_left  = children_left_count [0];
			n_right = children_right_count[0];

			memcpy(indices[0] + first_index,          children_left[0],  n_left  * sizeof(int));
			memcpy(indices[1] + first_index,          children_left[1],  n_left  * sizeof(int));
			memcpy(indices[2] + first_index,          children_left[2],  n_left  * sizeof(int));
			memcpy(indices[0] + first_index + n_left, children_right[0], n_right * sizeof(int));
			memcpy(indices[1] + first_index + n_left, children_right[1], n_right * sizeof(int));
			memcpy(indices[2] + first_index + n_left, children_right[2], n_right * sizeof(int));

			delete [] children_left[0];
			delete [] children_left[1];
			delete [] children_left[2];
			delete [] children_right[0];
			delete [] children_right[1];
			delete [] children_right[2];
		}

		return
			nodes[left  ].subdivide(triangles, indices, nodes, node_index, first_index,          n_left,  sah, temp) +
			nodes[left+1].subdivide(triangles, indices, nodes, node_index, first_index + n_left, n_right, sah, temp);
	}
	
	inline bool is_leaf() const {
		return (count & (~SBVH_AXIS_MASK)) > 0;
	}

	inline bool should_visit_left_first(const Ray & ray) const {
#if SBVH_TRAVERSAL_STRATEGY == SBVH_TRAVERSE_TREE_NAIVE
		return true;
#elif SBVH_TRAVERSAL_STRATEGY == SBVH_TRAVERSE_TREE_ORDERED
		switch (count & SBVH_AXIS_MASK) {
			case SBVH_AXIS_X_BITS: return ray.direction.x[0] > 0.0f;
			case SBVH_AXIS_Y_BITS: return ray.direction.y[0] > 0.0f;
			case SBVH_AXIS_Z_BITS: return ray.direction.z[0] > 0.0f;

			default: abort();
		}
#endif
	}

	inline void trace(const Triangle * primitives, const int * indices, const SBVHNode nodes[], const Ray & ray, RayHit & ray_hit, int step) const {
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

	inline SIMD_float intersect(const Triangle * primitives, const int * indices, const SBVHNode nodes[], const Ray & ray, SIMD_float max_distance) const {
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

struct SBVH {
	Triangle * primitives;
	int        primitive_count;

	int * indices_x;
	int * indices_y;
	int * indices_z;

	SBVHNode * nodes;

	inline void init(int count) {
		assert(count > 0);

		primitive_count = count; 
		primitives = new Triangle[primitive_count];

		// Construct index array
		int * all_indices  = new int[18 * primitive_count];
		indices_x = all_indices;
		indices_y = all_indices + primitive_count * 6;
		indices_z = all_indices + primitive_count * 12;

		for (int i = 0; i < primitive_count; i++) {
			indices_x[i] = i;
			indices_y[i] = i;
			indices_z[i] = i;
		}

		// Construct Node pool
		nodes = reinterpret_cast<SBVHNode *>(ALLIGNED_MALLOC(2 * primitive_count * sizeof(SBVHNode), 64));
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
#if SBVH_TRAVERSAL_STRATEGY == SBVH_TRAVERSE_BRUTE_FORCE
		for (int i = 0; i < primitive_count; i++) {
			primitives[i].trace(ray, ray_hit, 0);
		}
#elif SBVH_TRAVERSAL_STRATEGY == SBVH_TRAVERSE_TREE_NAIVE || SBVH_TRAVERSAL_STRATEGY == SBVH_TRAVERSE_TREE_ORDERED
		nodes[0].trace(primitives, indices_x, nodes, ray, ray_hit, 0);
#endif
	}

	inline SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const {
#if SBVH_TRAVERSAL_STRATEGY == SBVH_TRAVERSE_BRUTE_FORCE
		SIMD_float result(0.0f);

		for (int i = 0; i < primitive_count; i++) {
			result = result | primitives[i].intersect(ray, max_distance);

			if (SIMD_float::all_true(result)) break;
		}

		return result;
#elif SBVH_TRAVERSAL_STRATEGY == SBVH_TRAVERSE_TREE_NAIVE || SBVH_TRAVERSAL_STRATEGY == SBVH_TRAVERSE_TREE_ORDERED
		return nodes[0].intersect(primitives, indices_x, nodes, ray, max_distance);
#endif
	}
};
