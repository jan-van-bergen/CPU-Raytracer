#pragma once

#include "BVHPartitions.h"

#include "BVHNode.h"

namespace BVHBuilders {
	template<typename PrimitiveType>
	inline void build_bvh(BVHNode & node, const PrimitiveType * primitives, int * indices[3], BVHNode nodes[], int & node_index, int first_index, int index_count, float * sah, int * temp) {
		node.aabb = BVHPartitions::calculate_bounds(primitives, indices[0], first_index, first_index + index_count);
		
		if (index_count < 3) {
			// Leaf Node, terminate recursion
			node.first = first_index;
			node.count = index_count;

			return;
		}
		
		node.left = node_index;
		node_index += 2;
		
		int split_dimension;
		float split_cost;
		int split_index = BVHPartitions::partition_sah(primitives, indices, first_index, index_count, sah, split_dimension, split_cost);

		// Check SAH termination condition
		float parent_cost = node.aabb.surface_area() * float(index_count); 
		if (split_cost >= parent_cost) {
			node.first = first_index;
			node.count = index_count;

			return;
		}

		float split = primitives[indices[split_dimension][split_index]].get_position()[split_dimension];
		BVHPartitions::split_indices(primitives, indices, first_index, index_count, temp, split_dimension, split_index, split);

		node.count = (split_dimension + 1) << 30;

		int n_left  = split_index - first_index;
		int n_right = first_index + index_count - split_index;

		build_bvh(nodes[node.left    ], primitives, indices, nodes, node_index, first_index,          n_left,  sah, temp);
		build_bvh(nodes[node.left + 1], primitives, indices, nodes, node_index, first_index + n_left, n_right, sah, temp);
	}

	inline int build_sbvh(BVHNode & node, const Triangle * triangles, int * indices[3], BVHNode nodes[], int & node_index, int first_index, int index_count, float * sah, int * temp[2], float inv_root_surface_area, AABB node_aabb) {
		node.aabb = node_aabb;

		if (index_count < 3) {
			// Leaf Node, terminate recursion
			node.first = first_index;
			node.count = index_count;
			
			return node.count;
		}
		
		node.left = node_index;
		node_index += 2;

		// Object Split information
		float object_split_cost;
		int   object_split_dimension;
		AABB  object_split_aabb_left;
		AABB  object_split_aabb_right;
		int   object_split_index = BVHPartitions::partition_object(triangles, indices, first_index, index_count, sah, object_split_dimension, object_split_cost, node_aabb, object_split_aabb_left, object_split_aabb_right);

		assert(object_split_index != -1);

		// Spatial Split information
		float spatial_split_cost = INFINITY;
		int   spatial_split_dimension;
		float spatial_split_plane_distance;
		AABB  spatial_split_aabb_left;
		AABB  spatial_split_aabb_right;
		int   spatial_split_count_left;
		int   spatial_split_count_right;

		// Calculate the overlap between the child bounding boxes resulting from the Object Split
		float lamba = 0.0f;
		AABB overlap = AABB::overlap(object_split_aabb_left, object_split_aabb_right);
		if (overlap.is_valid()) {
			lamba = overlap.surface_area();
		}

		// Alpha == 1 means regular BVH, Alpha == 0 means full SBVH
		const float alpha = 10e-5; 

		// Divide by the surface area of the bounding box of the root Node
		float ratio = lamba * inv_root_surface_area;
		
		assert(ratio >= 0.0f && ratio <= 1.0f);

		// If ratio between overlap area and root area is large enough, consider a Spatial Split
		if (ratio > alpha) { 
			BVHPartitions::partition_spatial(triangles, indices, first_index, index_count, spatial_split_dimension, spatial_split_cost, spatial_split_plane_distance, spatial_split_aabb_left, spatial_split_aabb_right, spatial_split_count_left, spatial_split_count_right, node_aabb);
		}

		// Check SAH termination condition
		float parent_cost = node.aabb.surface_area() * float(index_count); 
		if (parent_cost <= object_split_cost && parent_cost <= spatial_split_cost) {
			node.first = first_index;
			node.count = index_count;
			
			return node.count;
		} 

		// From this point on it is decided that this Node will NOT be a leaf Node
		node.count = (object_split_dimension + 1) << 30;
		
		int * children_left [3] { indices[0] + first_index, indices[1] + first_index, indices[2] + first_index };
		int * children_right[3] { new int[index_count],     new int[index_count],     new int[index_count]     };

		int children_left_count [3] = { 0, 0, 0 };
		int children_right_count[3] = { 0, 0, 0 };

		int n_left, n_right;

		AABB child_aabb_left;
		AABB child_aabb_right;

		if (object_split_cost <= spatial_split_cost) {
			// Perform Object Split

			// Obtain split plane
			float split = triangles[indices[object_split_dimension][object_split_index]].get_position()[object_split_dimension];

			for (int dimension = 0; dimension < 3; dimension++) {
				for (int i = first_index; i < first_index + index_count; i++) {
					int index = indices[dimension][i];

					bool goes_left = triangles[index].get_position()[object_split_dimension] < split;

					if (triangles[index].get_position()[object_split_dimension] == split) {
						// In case the current primitive has the same coordianate as the one we split on along the split dimension,
						// We don't know whether the primitive should go left or right.
						// In this case check all primitive indices on the left side of the split that 
						// have the same split coordinate for equality with the current primitive index i

						int j = object_split_index - 1;
						// While we can go left and the left primitive has the same coordinate along the split dimension as the split itself
						while (j >= first_index && triangles[indices[object_split_dimension][j]].get_position()[object_split_dimension] == split) {
							if (indices[object_split_dimension][j] == index) {
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
			}
			
			// We should have made the same decision (going left/right) in every dimension
			assert(children_left_count [0] == children_left_count [1] && children_left_count [1] == children_left_count [2]);
			assert(children_right_count[0] == children_right_count[1] && children_right_count[1] == children_right_count[2]);
			
			n_left  = children_left_count [0];
			n_right = children_right_count[0];

			// Using object split, no duplicates can occur. 
			// Thus, left + right should equal the total number of triangles
			assert(first_index + n_left == object_split_index);
			assert(n_left + n_right == index_count);
			
			child_aabb_left  = object_split_aabb_left;
			child_aabb_right = object_split_aabb_right;
		} else {
			// Perform Spatial Split

			// The two temp arrays will be used as lookup tables
			int * indices_going_left  = temp[0];
			int * indices_going_right = temp[1];

			int temp_left = 0, temp_right = 0;
			
			// Keep track of amount of rejected references on both sides for debugging purposes
			int rejected_left  = 0;
			int rejected_right = 0;

			float n_1 = float(spatial_split_count_left);
			float n_2 = float(spatial_split_count_right);

			for (int i = first_index; i < first_index + index_count; i++) {
				int index = indices[spatial_split_dimension][i];
				const Triangle & triangle = triangles[index];
				
				bool goes_left = 
					triangle.position_0[spatial_split_dimension] < spatial_split_plane_distance || 
					triangle.position_1[spatial_split_dimension] < spatial_split_plane_distance || 
					triangle.position_2[spatial_split_dimension] < spatial_split_plane_distance;
				bool goes_right = 
					triangle.position_0[spatial_split_dimension] >= spatial_split_plane_distance || 
					triangle.position_1[spatial_split_dimension] >= spatial_split_plane_distance || 
					triangle.position_2[spatial_split_dimension] >= spatial_split_plane_distance;

				assert(goes_left || goes_right);

				if (goes_left && goes_right) { // Straddler					
					// A split can result in triangles that lie on one side of the plane but that don't overlap the AABB
					bool valid_left  = AABB::overlap(triangle.aabb, spatial_split_aabb_left ).is_valid();
					bool valid_right = AABB::overlap(triangle.aabb, spatial_split_aabb_right).is_valid();

					if (valid_left && valid_right) {
						// Consider unsplitting
						AABB delta_left  = spatial_split_aabb_left;
						AABB delta_right = spatial_split_aabb_right;

						delta_left.expand (triangle.aabb);
						delta_right.expand(triangle.aabb);

						float spatial_split_aabb_left_surface_area  = spatial_split_aabb_left.surface_area();
						float spatial_split_aabb_right_surface_area = spatial_split_aabb_right.surface_area();

						// Calculate SAH cost for the 3 different cases
						float c_split = spatial_split_aabb_left_surface_area   *  n_1       + spatial_split_aabb_right_surface_area   *  n_2;
						float c_1     =              delta_left.surface_area() *  n_1       + spatial_split_aabb_right_surface_area   * (n_2-1.0f);
						float c_2     = spatial_split_aabb_left_surface_area   * (n_1-1.0f) +              delta_right.surface_area() *  n_2;

						// If C_1 resp. C_2 is cheapest, let the triangle go left resp. right
						// Otherwise, do nothing and let the triangle go both left and right
						if (c_1 < c_split) {
							if (c_2 < c_1) { // C_2 is cheapest, remove from left
								goes_left = false;
								rejected_left++;

								n_1 -= 1.0f;

								spatial_split_aabb_right.expand(triangle.aabb);
							} else { // C_1 is cheapest, remove from right
								goes_right = false;
								rejected_right++;
								
								n_2 -= 1.0f;

								spatial_split_aabb_left.expand(triangle.aabb);
							}
						} else if (c_2 < c_split) { // C_2 is cheapest, remove from left
							goes_left = false;
							rejected_left++;
							
							n_1 -= 1.0f;

							spatial_split_aabb_right.expand(triangle.aabb);
						}
					} else {
						if (!valid_right) {
							goes_right = false;

							rejected_right++;
						}

						if (!valid_left) {
							goes_left = false;

							rejected_left++;
						}
					}
				}

				// Triangle must go left, right, or both
				assert(goes_left || goes_right);

				indices_going_left [index] = goes_left;
				indices_going_right[index] = goes_right;
			}

			// In all three dimensions, use the lookup table to decide which way each Triangle should go
			for (int dimension = 0; dimension < 3; dimension++) {	
				for (int i = first_index; i < first_index + index_count; i++) {
					int index = indices[dimension][i];

					bool goes_left  = indices_going_left [index];
					bool goes_right = indices_going_right[index];

					assert(goes_left || goes_right);

					if (goes_left)  children_left [dimension][children_left_count [dimension]++] = index;
					if (goes_right) children_right[dimension][children_right_count[dimension]++] = index;
				}
			}

			// We should have made the same decision (going left/right) in every dimension
			assert(children_left_count [0] == children_left_count [1] && children_left_count [1] == children_left_count [2]);
			assert(children_right_count[0] == children_right_count[1] && children_right_count[1] == children_right_count[2]);
			
			n_left  = children_left_count [0];
			n_right = children_right_count[0];
			
			// The actual number of references going left/right should match the numbers calculated during spatial splitting
			assert(n_left  == spatial_split_count_left  - rejected_left);
			assert(n_right == spatial_split_count_right - rejected_right);

			// A valid partition contains at least one and strictly less than all
			assert(n_left  > 0 && n_left  < index_count);
			assert(n_right > 0 && n_right < index_count);
			
			// Make sure no triangles dissapeared
			assert(n_left + n_right >= index_count);
			
			child_aabb_left  = spatial_split_aabb_left;
			child_aabb_right = spatial_split_aabb_right;
		}
		
		// Do a depth first traversal, so that we know the amount of indices that were recursively created by the left child
		int number_of_leaves_left = build_sbvh(nodes[node.left], triangles, indices, nodes, node_index, first_index, n_left, sah, temp, inv_root_surface_area, child_aabb_left);

		// Using the depth first offset, we can now copy over the right references
		memcpy(indices[0] + first_index + number_of_leaves_left, children_right[0], n_right * sizeof(int));
		memcpy(indices[1] + first_index + number_of_leaves_left, children_right[1], n_right * sizeof(int));
		memcpy(indices[2] + first_index + number_of_leaves_left, children_right[2], n_right * sizeof(int));
			
		// Now recurse on the right side
		int number_of_leaves_right = build_sbvh(nodes[node.left + 1], triangles, indices, nodes, node_index, first_index + number_of_leaves_left, n_right, sah, temp, inv_root_surface_area, child_aabb_right);
		
		delete [] children_right[0];
		delete [] children_right[1];
		delete [] children_right[2];
		
		return number_of_leaves_left + number_of_leaves_right;
	}
}
