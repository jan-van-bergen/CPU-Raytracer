#include "TopLevelBVH.h"

#include <algorithm>

void TopLevelBVH::init(int count) {
	assert(count > 0);

	primitive_count = count; 
	primitives = new Mesh[primitive_count];

	indices = nullptr;

	// Construct Node pool
	nodes = reinterpret_cast<BVHNode *>(ALLIGNED_MALLOC(2 * primitive_count * sizeof(BVHNode), 64));
	assert((unsigned long long)nodes % 64 == 0);

	// Used for rebuilding, allocated once so we don't have to heap allocate/destroy every frame
	indices_x = new int[primitive_count];
	indices_y = new int[primitive_count];
	indices_z = new int[primitive_count];

	for (int i = 0; i < primitive_count; i++) {
		indices_x[i] = i;
		indices_y[i] = i;
		indices_z[i] = i;
	}

	sah  = new float[primitive_count];
	temp = new int[primitive_count];

	indices = indices_x;
}

void TopLevelBVH::build_bvh() {
	std::sort(indices_x, indices_x + primitive_count, [&](int a, int b) { return primitives[a].get_position().x < primitives[b].get_position().x; });
	std::sort(indices_y, indices_y + primitive_count, [&](int a, int b) { return primitives[a].get_position().y < primitives[b].get_position().y; });
	std::sort(indices_z, indices_z + primitive_count, [&](int a, int b) { return primitives[a].get_position().z < primitives[b].get_position().z; });
		
	int * indices_xyz[3] = { indices_x, indices_y, indices_z };

	node_count = 2;
	BVHBuilders::build_bvh(nodes[0], primitives, indices_xyz, nodes, node_count, 0, primitive_count, sah, temp);

	assert(node_count <= 2 * primitive_count);

	leaf_count = primitive_count;
}

void TopLevelBVH::update() const {
	for (int i = 0; i < primitive_count; i++) {
		primitives[i].update();
	}
}

void TopLevelBVH::trace(const Ray & ray, RayHit & ray_hit, const Matrix4 & world) const {
	int stack[128];
	int stack_size = 1;

	// Push root on stack
	stack[0] = 0;

	int step = 0;

	while (stack_size > 0) {
		// Pop Node of the stack
		const BVHNode & node = nodes[stack[--stack_size]];

		SIMD_float mask = node.aabb.intersect(ray, ray_hit.distance);
		if (SIMD_float::all_false(mask)) continue;

		if (node.is_leaf()) {
			for (int i = node.first; i < node.first + node.count; i++) {
				primitives[indices[i]].trace(ray, ray_hit, world, step);
			}
		} else {
			if (node.should_visit_left_first(ray)) {
				stack[stack_size++] = node.left + 1;
				stack[stack_size++] = node.left;
			} else {
				stack[stack_size++] = node.left;
				stack[stack_size++] = node.left + 1;
			}
		}

		step++;
	}
}

SIMD_float TopLevelBVH::intersect(const Ray & ray, SIMD_float max_distance) const {
	int stack[128];
	int stack_size = 1;

	// Push root on stack
	stack[0] = 0;

	int step = 0;

	SIMD_float hit(0.0f);

	while (stack_size > 0) {
		// Pop Node of the stack
		const BVHNode & node = nodes[stack[--stack_size]];

		SIMD_float mask = node.aabb.intersect(ray, max_distance);
		if (SIMD_float::all_false(mask)) continue;

		if (node.is_leaf()) {
			for (int i = node.first; i < node.first + node.count; i++) {
				hit = hit | primitives[indices[i]].intersect(ray, max_distance);

				if (SIMD_float::all_true(hit)) return hit;
			}
		} else {
			if (node.should_visit_left_first(ray)) {
				stack[stack_size++] = node.left + 1;
				stack[stack_size++] = node.left;
			} else {
				stack[stack_size++] = node.left;
				stack[stack_size++] = node.left + 1;
			}
		}

		step++;
	}

	return hit;
}
