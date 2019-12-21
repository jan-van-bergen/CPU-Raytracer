#pragma once
#include <cassert>
#include <algorithm>

#include "BVHBuilders.h"

#include "ScopedTimer.h"

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

		int overallocation = 2;

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
		ScopedTimer timer("BVH Construction");

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
		ScopedTimer timer("SBVH Construction");

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

		assert(node_index <= 8 * primitive_count);

		delete [] temp[0];
		delete [] temp[1];
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
