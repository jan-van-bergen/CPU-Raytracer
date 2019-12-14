#pragma once
#include "AABB.h"

#include "Test.h"

namespace BVHConstructors {
	// Calculates the smallest enclosing AABB over the union of all AABB's of the primitives in the range defined by [first, last>
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

	// Used for debugging
	template<typename PrimitiveType>
	inline bool is_sorted(const PrimitiveType * primitives, int * indices[3], int first, int last) {
		for (int dimension = 0; dimension < 3; dimension++) {
			for (int i = first + 1; i < last; i++) {
				float prev = primitives[indices[dimension][i-1]].get_position()[dimension];
				float curr = primitives[indices[dimension][i  ]].get_position()[dimension];

				if (prev > curr) return false;
			}
		}

		return true;
	}

	// Reorders indices arrays such that indices on the left side of the splitting dimension end up on the left partition in the other dimensions as well
	template<typename PrimitiveType>
	inline void split_indices(const PrimitiveType * primitives, int * indices[3], int first_index, int index_count, int * temp, int split_dimension, int split_index, float split) {
		for (int dimension = 0; dimension < 3; dimension++) {
			if (dimension != split_dimension) {
				int left  = first_index;
				int right = split_index;

				for (int i = first_index; i < first_index + index_count; i++) {
					bool goes_left = primitives[indices[dimension][i]].get_position()[split_dimension] < split;

					if (primitives[indices[dimension][i]].get_position()[split_dimension] == split) {
						// In case the current primitive has the same coordianate as the one we split on along the split dimension,
						// We don't know whether the primitive should go left or right.
						// In this case check all primitive indices on the left side of the split that 
						// have the same split coordinate for equality with the current primitive index i

						int j = split_index - 1;
						// While we can go left and the left primitive has the same coordinate along the split dimension as the split itself
						while (j >= 0 && primitives[indices[split_dimension][j]].get_position()[split_dimension] == split) {
							if (indices[split_dimension][j] == indices[dimension][i]) {
								goes_left = true;

								break;
							}

							j--;
						}
					}

					if (goes_left) {					
						temp[left++] = indices[dimension][i];
					} else {
						temp[right++] = indices[dimension][i];
					}
				}

				// If these conditions are not met the memcpy below is invalid
				assert(left  == split_index);
				assert(right == first_index + index_count);

				memcpy(indices[dimension] + first_index, temp + first_index, index_count * sizeof(int));

				assert(is_sorted(primitives, indices, first_index,        left ));
				assert(is_sorted(primitives, indices, first_index + left, right));
			}
		}
	}

	// Partitions object using the median Primitive along the longest axis
	template<typename PrimitiveType>
	inline int partition_median(const PrimitiveType * primitives, int * indices[3], int first_index, int index_count, int * temp, int & split_dimension) {
		float max_axis_length    = -INFINITY;
		int   max_axis_dimension = -1;

		// Find longest dimension
		for (int dimension = 0; dimension < 3; dimension++) {
			assert(is_sorted(primitives, indices, first_index, first_index + index_count));

			float min = primitives[indices[dimension][first_index                  ]].get_position()[dimension];
			float max = primitives[indices[dimension][first_index + index_count - 1]].get_position()[dimension];

			float axis_length = max - min;
			if (axis_length > max_axis_length) {
				max_axis_length = axis_length;
				max_axis_dimension = dimension;
			}
		}

		int median_index = first_index + (index_count >> 1);
		float split = primitives[indices[max_axis_dimension][median_index]].get_position()[max_axis_dimension];
		
		split_indices(primitives, indices, first_index, index_count, temp, max_axis_dimension, median_index, split);

		split_dimension = (max_axis_dimension + 1) << 30;

		return median_index;
	}

	// Evaluates SAH for every object for every dimension to determine splitting candidate
	template<typename PrimitiveType>
	inline int partition_object(const PrimitiveType * primitives, int * indices[3], int first_index, int index_count, float parent_cost, float * sah, int * temp, int & split_dimension) {
		float min_cost = INFINITY;
		int   min_split_index     = -1;
		int   min_split_dimension = -1;

		// Check splits along all 3 dimensions
		for (int dimension = 0; dimension < 3; dimension++) {
			assert(is_sorted(primitives, indices, first_index, index_count));

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

		// Check SAH termination condition
		if (min_cost >= parent_cost) return -1;

		float split = primitives[indices[min_split_dimension][min_split_index]].get_position()[min_split_dimension];

		split_indices(primitives, indices, first_index, index_count, temp, min_split_dimension, min_split_index, split);

		split_dimension = (min_split_dimension + 1) << 30;

		return min_split_index;
	}
}
