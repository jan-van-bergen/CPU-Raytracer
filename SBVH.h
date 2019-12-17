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

	inline int subdivide(const Triangle * triangles, int * indices[3], SBVHNode nodes[], int & node_index, int first_index, int index_count, float * sah, int * temp, float inv_root_surface_area) {
		aabb = BVHConstructors::calculate_bounds(triangles, indices[0], first_index, first_index + index_count);
		
		if (index_count < 3) {
			// Leaf Node, terminate recursion
			first = first_index;
			count = index_count;
			
			return count;
		}
		
		left = node_index;
		node_index += 2;

		float full_sah_split_cost = NAN;
		int   full_sah_split_dimension = -1;
		int   full_sah_split_index = BVHConstructors::partition_full_sah(triangles, indices, first_index, index_count, sah, temp, full_sah_split_dimension, full_sah_split_cost);

		AABB aabb_left  = BVHConstructors::calculate_bounds(triangles, indices[full_sah_split_dimension], first_index,          full_sah_split_index);
		AABB aabb_right = BVHConstructors::calculate_bounds(triangles, indices[full_sah_split_dimension], full_sah_split_index, first_index + index_count);

		float spatial_split_cost = INFINITY;
		int   spatial_split_dimension = -1;
		float spatial_split_plane_distance = NAN;
		
		float lamba = AABB::overlap(aabb_left, aabb_right).surface_area();

		const float alpha = 0.00001f; // Alpha == 1 means regular BVH, Alpha == 0 means full SBVH
		if (lamba * inv_root_surface_area > alpha) { 
			BVHConstructors::partition_spatial(triangles, indices, first_index, index_count, sah, temp, spatial_split_dimension, spatial_split_cost, spatial_split_plane_distance);
		}

		// @TODO: left is not needed and right can use the 'temp' array
		int * children_left [3] { new int[index_count], new int[index_count], new int[index_count] };
		int * children_right[3] { new int[index_count], new int[index_count], new int[index_count] };

		int children_left_count [3] = { 0, 0, 0 };
		int children_right_count[3] = { 0, 0, 0 };

		if (full_sah_split_cost <= spatial_split_cost) {
			// Check SAH termination condition
			float parent_cost = aabb.surface_area() * float(index_count); 
			if (full_sah_split_cost >= parent_cost) {
				first = first_index;
				count = index_count;
				
				return count;
			}
			
			count = (full_sah_split_dimension + 1) << 30;

			float split = triangles[indices[full_sah_split_dimension][full_sah_split_index]].get_position()[full_sah_split_dimension];

			for (int dimension = 0; dimension < 3; dimension++) {
				//if (dimension != full_sah_split_dimension) {
					for (int i = first_index; i < first_index + index_count; i++) {
						int index = indices[dimension][i];

						bool goes_left = triangles[indices[dimension][i]].get_position()[full_sah_split_dimension] < split;

						if (triangles[indices[dimension][i]].get_position()[full_sah_split_dimension] == split) {
							// In case the current primitive has the same coordianate as the one we split on along the split dimension,
							// We don't know whether the primitive should go left or right.
							// In this case check all primitive indices on the left side of the split that 
							// have the same split coordinate for equality with the current primitive index i

							int j = full_sah_split_index - 1;
							// While we can go left and the left primitive has the same coordinate along the split dimension as the split itself
							while (j >= first_index && triangles[indices[full_sah_split_dimension][j]].get_position()[full_sah_split_dimension] == split) {
								if (indices[full_sah_split_dimension][j] == indices[dimension][i]) {
									goes_left = true;

									break;
								}

								j--;
							}
						}

						if (goes_left) {
							children_left[dimension][children_left_count[dimension]++] = index;
						} else {
							children_right[dimension][children_right_count[dimension]++] = index;
						}
					}
				//}
			}
			
			assert(children_left_count [0] == children_left_count [1] && children_left_count [1] == children_left_count [2]);
			assert(children_right_count[0] == children_right_count[1] && children_right_count[1] == children_right_count[2]);
			
			int n_left  = children_left_count [0];
			int n_right = children_right_count[0];

			assert(first_index + n_left == full_sah_split_index);
			assert(n_left + n_right == index_count);

			memcpy(indices[0] + first_index, children_left[0], n_left * sizeof(int));
			memcpy(indices[1] + first_index, children_left[1], n_left * sizeof(int));
			memcpy(indices[2] + first_index, children_left[2], n_left * sizeof(int));

			int offset_left  = nodes[left].subdivide(triangles, indices, nodes, node_index, first_index, n_left,  sah, temp, inv_root_surface_area);

			memcpy(indices[0] + first_index + offset_left, children_right[0], n_right * sizeof(int));
			memcpy(indices[1] + first_index + offset_left, children_right[1], n_right * sizeof(int));
			memcpy(indices[2] + first_index + offset_left, children_right[2], n_right * sizeof(int));

			int offset_right = nodes[left + 1].subdivide(triangles, indices, nodes, node_index, first_index + offset_left, n_right, sah, temp, inv_root_surface_area);
			
			delete [] children_left[0];
			delete [] children_left[1];
			delete [] children_left[2];
			delete [] children_right[0];
			delete [] children_right[1];
			delete [] children_right[2];
			
			return offset_left + offset_right;
		} else {
			// Check SAH termination condition
			float parent_cost = aabb.surface_area() * float(index_count); 
			if (spatial_split_cost >= parent_cost) {
				first = first_index;
				count = index_count;

				return count;
			}
			
			count = (spatial_split_dimension + 1) << 30;

			for (int dimension = 0; dimension < 3; dimension++) {
				for (int i = first_index; i < first_index + index_count; i++) {
					int index = indices[dimension][i];
					const Triangle & triangle = triangles[index];

					float dist_p0 = triangle.position0[spatial_split_dimension] + spatial_split_plane_distance;
					float dist_p1 = triangle.position1[spatial_split_dimension] + spatial_split_plane_distance;
					float dist_p2 = triangle.position2[spatial_split_dimension] + spatial_split_plane_distance;

					const float epsilon = 0.0001f;
					bool goes_left  = dist_p0 < -epsilon || dist_p1 < -epsilon || dist_p2 < -epsilon;
					bool goes_right = dist_p0 >  epsilon || dist_p1 >  epsilon || dist_p2 >  epsilon;

					if (goes_left && goes_right) {
						// B1 en B2 weet je op dit moment nog niet, die moet je dus WEL berekenen in parition_spatial
						// Zelfde geldt voor N1 en N2

						// @TODO: unsplitting
					} 
					
					if (goes_left) {
						children_left[dimension][children_left_count[dimension]++] = index;
					} 
					
					if (goes_right) {
						children_right[dimension][children_right_count[dimension]++] = index;
					}
				}
			}

			assert(children_left_count [0] == children_left_count [1] && children_left_count [1] == children_left_count [2]);
			assert(children_right_count[0] == children_right_count[1] && children_right_count[1] == children_right_count[2]);
			
			int n_left  = children_left_count [0];
			int n_right = children_right_count[0];

			assert(n_left  > 0 && n_left  < index_count);
			assert(n_right > 0 && n_right < index_count);
			
			assert(n_left + n_right >= index_count);

			memcpy(indices[0] + first_index, children_left[0], n_left * sizeof(int));
			memcpy(indices[1] + first_index, children_left[1], n_left * sizeof(int));
			memcpy(indices[2] + first_index, children_left[2], n_left * sizeof(int));

			int offset_left = nodes[left].subdivide(triangles, indices, nodes, node_index, first_index, n_left, sah, temp, inv_root_surface_area);

			memcpy(indices[0] + first_index + offset_left, children_right[0], n_right * sizeof(int));
			memcpy(indices[1] + first_index + offset_left, children_right[1], n_right * sizeof(int));
			memcpy(indices[2] + first_index + offset_left, children_right[2], n_right * sizeof(int));

			int offset_right = nodes[left + 1].subdivide(triangles, indices, nodes, node_index, first_index + offset_left, n_right, sah, temp, inv_root_surface_area);

			delete [] children_left[0];
			delete [] children_left[1];
			delete [] children_left[2];
			delete [] children_right[0];
			delete [] children_right[1];
			delete [] children_right[2];
			
			return offset_left + offset_right;
		}
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
	
	inline void debug(FILE * file, const SBVHNode nodes[], int & index) const {
		aabb.debug(file, index++);

		if (!is_leaf()) {
			nodes[left  ].debug(file, nodes, index);
			nodes[left+1].debug(file, nodes, index);
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

		float root_surface_area = BVHConstructors::calculate_bounds(primitives, indices[0], 0, primitive_count).surface_area();

		int node_index = 2;
		int leaf_count = nodes[0].subdivide(primitives, indices, nodes, node_index, 0, primitive_count, sah, temp, 1.0f / root_surface_area);

		printf("Leaf count: %i\n", leaf_count);

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
	
	inline void debug() const {
		FILE * file = nullptr;
		fopen_s(&file, DATA_PATH("debug.obj"), "w");

		if (file == nullptr) abort(); // Error opening file!

		int index = 0;
		nodes[0].debug(file, nodes, index);

		fclose(file);

		printf("Written debug info to debug.obj\n");
	}
};
