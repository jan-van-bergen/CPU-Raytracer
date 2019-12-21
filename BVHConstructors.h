#pragma once
#include "AABB.h"
#include "Math.h"

#include "Debug.h"

namespace BVHConstructors {
	inline const int BIN_COUNT = 256;

	// Calculates the smallest enclosing AABB over the union of all AABB's of the primitives in the range defined by [first, last>
	template<typename PrimitiveType>
	inline AABB calculate_bounds(const PrimitiveType * primitives, const int * indices, int first, int last) {
		AABB aabb = AABB::create_empty();

		// Iterate over relevant Primitives
		for (int i = first; i < last; i++) {
			aabb.expand(primitives[indices[i]].aabb);
		}

		aabb.fix_if_needed();

		assert(aabb.is_valid());

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

	// Used for debugging
	template<typename PrimitiveType>
	inline bool is_unique(const PrimitiveType * primitives, int * indices[3], int first_index, int index_count) {
		for (int dimension = 0; dimension < 3; dimension++) {
			for (int i = first_index; i < first_index + index_count; i++) {
				for (int j = first_index; j < i; j++) {
					if (indices[dimension][j] == indices[dimension][i]) abort();
				}
			}
		}

		return true;
	}

	// Reorders indices arrays such that indices on the left side of the splitting dimension end up on the left partition in the other dimensions as well
	template<typename PrimitiveType>
	inline void split_indices(const PrimitiveType * primitives, int * indices[3], int first_index, int index_count, int * temp, int split_dimension, int split_index, float split) {
		for (int dimension = 0; dimension < 3; dimension++) {
			if (dimension != split_dimension) {
				int left  = 0;
				int right = split_index - first_index;

				for (int i = first_index; i < first_index + index_count; i++) {
					bool goes_left = primitives[indices[dimension][i]].get_position()[split_dimension] < split;

					if (primitives[indices[dimension][i]].get_position()[split_dimension] == split) {
						// In case the current primitive has the same coordianate as the one we split on along the split dimension,
						// We don't know whether the primitive should go left or right.
						// In this case check all primitive indices on the left side of the split that 
						// have the same split coordinate for equality with the current primitive index i

						int j = split_index - 1;
						// While we can go left and the left primitive has the same coordinate along the split dimension as the split itself
						while (j >= first_index && primitives[indices[split_dimension][j]].get_position()[split_dimension] == split) {
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
				assert(left  == split_index - first_index);
				assert(right == index_count);

				memcpy(indices[dimension] + first_index, temp, index_count * sizeof(int));

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

		split_dimension = max_axis_dimension;

		int median_index = first_index + (index_count >> 1);
		return median_index;
	}

	// Evaluates SAH for every object for every dimension to determine splitting candidate
	template<typename PrimitiveType>
	inline int partition_object(const PrimitiveType * primitives, int * indices[3], int first_index, int index_count, float * sah, int * temp, int & split_dimension, float & split_cost) {
		float min_split_cost = INFINITY;
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
				if (cost < min_split_cost) {
					min_split_cost = cost;
					min_split_index = first_index + i + 1;
					min_split_dimension = dimension;
				}
			}
		}
		
		split_cost      = min_split_cost;
		split_dimension = min_split_dimension;

		return min_split_index;
	}

	// Evaluates SAH for every object for every dimension to determine splitting candidate
	template<typename PrimitiveType>
	inline int partition_full_sah(const PrimitiveType * primitives, int * indices[3], int first_index, int index_count, float * sah, int * temp, int & split_dimension, float & split_cost, const AABB & node_aabb, AABB & aabb_left, AABB & aabb_right) {
		float min_split_cost = INFINITY;
		int   min_split_index     = -1;
		int   min_split_dimension = -1;
		
		AABB * bounds_left  = new AABB[index_count];
		AABB * bounds_right = new AABB[index_count + 1];
		
		// Check splits along all 3 dimensions
		for (int dimension = 0; dimension < 3; dimension++) {
			assert(is_sorted(primitives, indices, first_index, index_count));

			bounds_left [0]           = AABB::create_empty();
			bounds_right[index_count] = AABB::create_empty();

			// First traverse left to right along the current dimension to evaluate first half of the SAH
			for (int i = 1; i < index_count; i++) {
				bounds_left[i] = bounds_left[i-1];
				bounds_left[i].expand(primitives[indices[dimension][first_index + i - 1]].aabb);
				bounds_left[i] = AABB::overlap(bounds_left[i], node_aabb);

				sah[i] = bounds_left[i].surface_area() * float(i);
			}

			// Then traverse right to left along the current dimension to evaluate second half of the SAH
			for (int i = index_count - 1; i > 0; i--) {
				bounds_right[i] = bounds_right[i+1];
				bounds_right[i].expand(primitives[indices[dimension][first_index + i]].aabb);
				bounds_right[i] = AABB::overlap(bounds_right[i], node_aabb);
				
				sah[i] += bounds_right[i].surface_area() * float(index_count - i);
			}
			
			// Find the minimum of the SAH
			for (int i = 1; i < index_count; i++) {
				float cost = sah[i];
				if (cost < min_split_cost) {
					min_split_cost = cost;
					min_split_index = first_index + i;
					min_split_dimension = dimension;
					
					assert(!bounds_left [i].is_empty());
					assert(!bounds_right[i].is_empty());

					aabb_left  = bounds_left[i];
					aabb_right = bounds_right[i];
				}
			}
		}

		delete [] bounds_left;
		delete [] bounds_right;
		
		split_dimension = min_split_dimension;
		split_cost      = min_split_cost;

		return min_split_index;
	}

	inline int partition_spatial(const Triangle * triangles, int * indices[3], int first_index, int index_count, float * sah, int * temp, int & split_dimension, float & split_cost, float & plane_distance, AABB & aabb_left, AABB & aabb_right, int & n_left, int & n_right, AABB bounds) {
		float min_bin_cost = INFINITY;
		int   min_bin_index     = -1;
		int   min_bin_dimension = -1;
		float min_bin_plane_distance = NAN;

		for (int dimension = 0; dimension < 3; dimension++) {
			float bounds_min  = bounds.min[dimension] - 0.001f;
			float bounds_max  = bounds.max[dimension] + 0.001f;
			float bounds_step = (bounds_max - bounds_min) / BIN_COUNT;
			
			float inv_bounds_delta = 1.0f / (bounds_max - bounds_min);

			struct Bin {
				AABB aabb = AABB::create_empty();
				int entries = 0;
				int exits   = 0;
			} bins[BIN_COUNT];

			// For each Triangle
			for (int i = first_index; i < first_index + index_count; i++) {
				const Triangle & triangle = triangles[indices[dimension][i]];
				
				float plane_left_distance = bounds_min;
				float plane_right_distance;

				int bin_min = int(BIN_COUNT * ((triangle.aabb.min[dimension] - bounds_min) * inv_bounds_delta));
				int bin_max = int(BIN_COUNT * ((triangle.aabb.max[dimension] - bounds_min) * inv_bounds_delta));

				bin_min = Math::clamp(bin_min, 0, BIN_COUNT - 1);
				bin_max = Math::clamp(bin_max, 0, BIN_COUNT - 1);

				bins[bin_min].entries++;
				bins[bin_max].exits++;

				// Iterate over bins that intersect the AABB along the current dimension
				for (int b = bin_min; b <= bin_max; b++) {
					Bin & bin = bins[b];

					plane_right_distance = bounds_min + float(b+1) * bounds_step;

					assert(bin.aabb.is_valid() || bin.aabb.is_empty());

					// Calculate relevant portion of the AABB with regard to the two planes that define the current Bin
					AABB box = Math::triangle_bin_bounds(dimension, plane_left_distance, plane_right_distance, triangle);

					// Clip the AABB against the parent bounds
					bin.aabb.expand(box);
					bin.aabb = AABB::overlap(bin.aabb, bounds);

					// AABB must be valid
					assert(bin.aabb.is_valid() || bin.aabb.is_empty());
					
					// The AABB of the current Bin cannot exceed the planes of the current Bin
					const float epsilon = 0.0001f;
					assert(bin.aabb.min[dimension] > plane_left_distance  - epsilon);
					assert(bin.aabb.max[dimension] < plane_right_distance + epsilon);

					// The AABB of the current Bin cannot exceed the bounds of the Node's AABB
					assert(bin.aabb.min[0] > bounds.min[0] - epsilon && bin.aabb.max[0] < bounds.max[0] + epsilon);
					assert(bin.aabb.min[1] > bounds.min[1] - epsilon && bin.aabb.max[1] < bounds.max[1] + epsilon);
					assert(bin.aabb.min[2] > bounds.min[2] - epsilon && bin.aabb.max[2] < bounds.max[2] + epsilon);
					
					// Advance to next Bin
					plane_left_distance = plane_right_distance;
				}
			}

			float bin_sah[BIN_COUNT];

			AABB bounds_left [BIN_COUNT];
			AABB bounds_right[BIN_COUNT + 1];
			
			bounds_left [0]         = AABB::create_empty();
			bounds_right[BIN_COUNT] = AABB::create_empty();

			int count_left [BIN_COUNT];
			int count_right[BIN_COUNT + 1];

			count_left [0]         = 0;
			count_right[BIN_COUNT] = 0;
			
			// First traverse left to right along the current dimension to evaluate first half of the SAH
			for (int b = 1; b < BIN_COUNT; b++) {
				bounds_left[b] = bounds_left[b-1];
				bounds_left[b].expand(bins[b-1].aabb);

				assert(bounds_left[b].is_valid() || bounds_left[b].is_empty());

				count_left[b] = count_left[b-1] + bins[b-1].entries;

				if (count_left[b] < index_count) {
					bin_sah[b] = bounds_left[b].surface_area() * float(count_left[b]);
				} else {
					bin_sah[b] = INFINITY;
				}
			}

			// Then traverse right to left along the current dimension to evaluate second half of the SAH
			for (int b = BIN_COUNT - 1; b > 0; b--) {
				bounds_right[b] = bounds_right[b+1];
				bounds_right[b].expand(bins[b].aabb);
				
				assert(bounds_right[b].is_valid() || bounds_right[b].is_empty());

				count_right[b] = count_right[b+1] + bins[b].exits;

				if (count_right[b] < index_count) {
					bin_sah[b] += bounds_right[b].surface_area() * float(count_right[b]);
				} else {
					bin_sah[b] = INFINITY;
				}
			}

			//assert(count_left[2] > 0);
			//assert(count_right[BIN_COUNT - 2] > 0);

			assert(count_left [BIN_COUNT - 1] + bins[BIN_COUNT - 1].entries == index_count);
			assert(count_right[1]             + bins[0].exits               == index_count);

			// Find the splitting plane that yields the lowest SAH cost along the current dimension
			for (int b = 1; b < BIN_COUNT; b++) {
				float cost = bin_sah[b];
				if (cost < min_bin_cost) {
					min_bin_cost = cost;
					min_bin_index = b;
					min_bin_dimension = dimension;

					aabb_left  = bounds_left [b];
					aabb_right = bounds_right[b];

					n_left  = count_left [b];
					n_right = count_right[b];

					min_bin_plane_distance = bounds_min + bounds_step * float(b);
				}
			}
		}

		split_dimension = min_bin_dimension;
		split_cost      = min_bin_cost;
		plane_distance  = min_bin_plane_distance;

		return min_bin_index;
	}
}
