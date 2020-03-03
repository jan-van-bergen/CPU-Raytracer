#include "BottomLevelBVH.h"

#include <algorithm>
#include <filesystem>
#include <unordered_map>

#include "OBJLoader.h"

#include "Material.h"

#include "SIMD_Vector2.h"
#include "SIMD_Vector3.h"

#include "ScopeTimer.h"

static std::unordered_map<std::string, BottomLevelBVH *> bvh_cache;

const BottomLevelBVH * BottomLevelBVH::load(const char * filename) {
	BottomLevelBVH *& bvh = bvh_cache[filename];

	// If the cache already contains the requested BVH simply return it
	if (bvh) return bvh;

	bvh = new BottomLevelBVH();

	std::string bvh_filename = std::string(filename) + ".bvh";
	
	if (std::filesystem::exists(bvh_filename)) {
		printf("Loading BVH %s from disk.\n", bvh_filename.c_str());

		bvh->load_from_disk(bvh_filename.c_str());

		OBJLoader::load_mtl(bvh, filename);
	} else {
		const Triangle * triangles = OBJLoader::load_obj(bvh, filename);

#if MESH_ACCELERATOR == MESH_USE_BVH
		{
			ScopeTimer timer("Mesh BVH Construction");
			bvh->build_bvh(triangles);
		}
#elif MESH_ACCELERATOR == MESH_USE_SBVH
		{
			ScopeTimer timer("Mesh SBVH Construction");
			bvh->build_sbvh(triangles);
		}
#endif

		delete [] triangles;

		bvh->save_to_disk(bvh_filename.c_str());
	}
	
	bvh->flatten();
	return bvh;
}

void BottomLevelBVH::init(int count) {
	assert(count > 0);

	triangle_count = count; 
	triangles_hot  = reinterpret_cast<TriangleHot  *>(ALIGNED_MALLOC(triangle_count * sizeof(TriangleHot),  CACHE_LINE_WIDTH));
	triangles_cold = reinterpret_cast<TriangleCold *>(ALIGNED_MALLOC(triangle_count * sizeof(TriangleCold), CACHE_LINE_WIDTH));

	indices = nullptr;

	nodes = reinterpret_cast<BVHNode *>(ALIGNED_MALLOC(2 * triangle_count * sizeof(BVHNode), CACHE_LINE_WIDTH));
}

void BottomLevelBVH::build_bvh(const Triangle * triangles) {
	int * indices_x = new int[triangle_count];
	int * indices_y = new int[triangle_count];
	int * indices_z = new int[triangle_count];

	for (int i = 0; i < triangle_count; i++) {
		indices_x[i] = i;
		indices_y[i] = i;
		indices_z[i] = i;
	}

	std::sort(indices_x, indices_x + triangle_count, [&](int a, int b) { return triangles[a].get_position().x < triangles[b].get_position().x; });
	std::sort(indices_y, indices_y + triangle_count, [&](int a, int b) { return triangles[a].get_position().y < triangles[b].get_position().y; });
	std::sort(indices_z, indices_z + triangle_count, [&](int a, int b) { return triangles[a].get_position().z < triangles[b].get_position().z; });
		
	int * indices_xyz[3] = { indices_x, indices_y, indices_z };

	float * sah  = new float[triangle_count];
	int   * temp = new int[triangle_count];

	node_count = 2;
	BVHBuilders::build_bvh(nodes[0], triangles, indices_xyz, nodes, node_count, 0, triangle_count, sah, temp);

	assert(node_count <= 2 * triangle_count);

	index_count = triangle_count;

	// Use indices_x to index the Primitives array, and delete the other two
	indices = indices_x;
	delete [] indices_y;
	delete [] indices_z;

	delete [] temp;
	delete [] sah;
}

void BottomLevelBVH::build_sbvh(const Triangle * triangles) {
	const int overallocation = 2; // SBVH requires more space

	int * indices_x = new int[overallocation * triangle_count];
	int * indices_y = new int[overallocation * triangle_count];
	int * indices_z = new int[overallocation * triangle_count];

	for (int i = 0; i < triangle_count; i++) {
		indices_x[i] = i;
		indices_y[i] = i;
		indices_z[i] = i;
	}

	std::sort(indices_x, indices_x + triangle_count, [&](int a, int b) { return triangles[a].get_position().x < triangles[b].get_position().x; });
	std::sort(indices_y, indices_y + triangle_count, [&](int a, int b) { return triangles[a].get_position().y < triangles[b].get_position().y; });
	std::sort(indices_z, indices_z + triangle_count, [&](int a, int b) { return triangles[a].get_position().z < triangles[b].get_position().z; });

	int * indices_xyz[3] = { indices_x, indices_y, indices_z };

	float * sah     = new float[triangle_count];
	int   * temp[2] = { new int[triangle_count], new int[triangle_count] };

	AABB root_aabb = BVHPartitions::calculate_bounds(triangles, indices_xyz[0], 0, triangle_count);

	node_count = 2;
	index_count = BVHBuilders::build_sbvh(nodes[0], triangles, indices_xyz, nodes, node_count, 0, triangle_count, sah, temp, 1.0f / root_aabb.surface_area(), root_aabb);

	printf("SBVH Leaf count: %i\n", index_count);

	assert(node_count <= 2 * triangle_count);

	// Use indices_x to index the Primitives array, and delete the other two
	indices = indices_x;
	delete [] indices_y;
	delete [] indices_z;

	delete [] temp[0];
	delete [] temp[1];
	delete [] sah;
}

void BottomLevelBVH::save_to_disk(const char * bvh_filename) const {
	FILE * file;
	fopen_s(&file, bvh_filename, "wb");

	if (file == nullptr) abort();

	fwrite(&triangle_count, sizeof(int), 1, file);

	fwrite(triangles_hot,  sizeof(TriangleHot),  triangle_count, file);
	fwrite(triangles_cold, sizeof(TriangleCold), triangle_count, file);

	fwrite(&node_count, sizeof(int), 1, file);
	fwrite(nodes, sizeof(BVHNode), node_count, file);

	fwrite(&index_count, sizeof(int), 1, file);
		
	fwrite(indices, sizeof(int), index_count, file);

	fclose(file);
}

void BottomLevelBVH::load_from_disk(const char * bvh_filename) {
	FILE * file;
	fopen_s(&file, bvh_filename, "rb"); 
	
	if (file == nullptr) abort();

	fread(&triangle_count, sizeof(int), 1, file);
	init(triangle_count);

	fread(triangles_hot,  sizeof(TriangleHot),  triangle_count, file);
	fread(triangles_cold, sizeof(TriangleCold), triangle_count, file);

	fread(&node_count, sizeof(int), 1, file);

	fread(nodes, sizeof(BVHNode), node_count, file);

	fread(&index_count, sizeof(int), 1, file);
	
	indices = new int[index_count];
	fread(indices, sizeof(int), index_count, file);

	fclose(file);
}

// Flattens the Triangle arrays out, so that the indices array is no longer required to index the Triangle array
// This means more memory consumption but is better for the cache and improves frame times slightly
void BottomLevelBVH::flatten() {
	TriangleHot  * flat_triangles_hot  = new TriangleHot [index_count];
	TriangleCold * flat_triangles_cold = new TriangleCold[index_count];

	for (int i = 0; i < index_count; i++) {
		flat_triangles_hot [i] = triangles_hot [indices[i]];
		flat_triangles_cold[i] = triangles_cold[indices[i]];
	}

	delete [] indices;

	ALIGNED_FREE(triangles_hot);
	ALIGNED_FREE(triangles_cold);

	triangles_hot  = flat_triangles_hot;
	triangles_cold = flat_triangles_cold;
}

void BottomLevelBVH::triangle_trace(int index, const Ray & ray, RayHit & ray_hit, const Matrix4 & world) const {
	const SIMD_float zero(0.0f);
	const SIMD_float one (1.0f);
	
	SIMD_Vector3 edge_1(triangles_hot[index].position_edge_1);
	SIMD_Vector3 edge_2(triangles_hot[index].position_edge_2);

	SIMD_Vector3 h = SIMD_Vector3::cross(ray.direction, edge_2);
	SIMD_float   a = SIMD_Vector3::dot(edge_1, h);

	SIMD_float   f = SIMD_float::rcp(a);
	SIMD_Vector3 s = ray.origin - SIMD_Vector3(triangles_hot[index].position_0);
	SIMD_float   u = f * SIMD_Vector3::dot(s, h);

	// If the barycentric coordinate on the edge between vertices i and i+1 
	// is outside the interval [0, 1] we know no intersection is possible
	SIMD_float mask = (u > zero) & (u < one);
	if (SIMD_float::all_false(mask)) return;

	SIMD_Vector3 q = SIMD_Vector3::cross(s, edge_1);
	SIMD_float   v = f * SIMD_Vector3::dot(ray.direction, q);

	// If the barycentric coordinate on the edge between vertices i and i+2 
	// is outside the interval [0, 1] we know no intersection is possible
	mask = mask & (v       > zero);
	mask = mask & ((u + v) < one);
	if (SIMD_float::all_false(mask)) return;

	SIMD_float t = f * SIMD_Vector3::dot(edge_2, q);

	// Check if we are in the right distance range
	mask = mask & (t > Ray::EPSILON);
	mask = mask & (t < ray_hit.distance);

	int int_mask = SIMD_float::mask(mask);
	if (int_mask == 0x0) return;
		
	ray_hit.hit      = ray_hit.hit | mask;
	ray_hit.distance = SIMD_float::blend(ray_hit.distance, t, mask);

	SIMD_Vector3 n_edge_1 = SIMD_Vector3(triangles_cold[index].normal_edge_1);
	SIMD_Vector3 n_edge_2 = SIMD_Vector3(triangles_cold[index].normal_edge_2);

	SIMD_Vector3 n = Math::barycentric(SIMD_Vector3(triangles_cold[index].normal_0), n_edge_1, n_edge_2, u, v);

	SIMD_Vector3 point  = Matrix4::transform_position(world, ray.origin + ray.direction * t);
	SIMD_Vector3 normal = Matrix4::transform_direction(world, SIMD_Vector3::normalize(n));

	ray_hit.point  = SIMD_Vector3::blend(ray_hit.point,  point,  mask);
	ray_hit.normal = SIMD_Vector3::blend(ray_hit.normal, normal, mask);
	
	ray_hit.material_id = SIMD_int::blend(ray_hit.material_id, SIMD_int(material_offset + triangles_cold[index].material_id), SIMD_float_as_int(mask));

	// Obtain u,v by barycentric interpolation of the texture coordinates of the three current vertices
	SIMD_Vector2 t_edge_1(triangles_cold[index].tex_coord_edge_1);
	SIMD_Vector2 t_edge_2(triangles_cold[index].tex_coord_edge_2);

	SIMD_Vector2 tex_coords = Math::barycentric(SIMD_Vector2(triangles_cold[index].tex_coord_0), t_edge_1, t_edge_2, u, v);
	ray_hit.u = SIMD_float::blend(ray_hit.u, tex_coords.x, mask);
	ray_hit.v = SIMD_float::blend(ray_hit.v, tex_coords.y, mask);
	
#if RAY_DIFFERENTIALS_ENABLED
	// Formulae from Chapter 20 of Ray Tracing Gems "Texture Level of Detail Strategies for Real-Time Ray Tracing"
	SIMD_float one_over_k = SIMD_float(1.0f) / SIMD_Vector3::dot(SIMD_Vector3::cross(edge_1, edge_2), ray.direction); 

	SIMD_Vector3 _q = SIMD_Vector3::madd(ray.dD_dx, t, ray.dO_dx);
	SIMD_Vector3 _r = SIMD_Vector3::madd(ray.dD_dy, t, ray.dO_dy);

	SIMD_Vector3 c_u = SIMD_Vector3::cross(edge_2, ray.direction);
	SIMD_Vector3 c_v = SIMD_Vector3::cross(ray.direction, edge_1);

	SIMD_float du_dx = one_over_k * SIMD_Vector3::dot(c_u, _q);
	SIMD_float du_dy = one_over_k * SIMD_Vector3::dot(c_u, _r);
	SIMD_float dv_dx = one_over_k * SIMD_Vector3::dot(c_v, _q);
	SIMD_float dv_dy = one_over_k * SIMD_Vector3::dot(c_v, _r);
	
	ray_hit.dO_dx = SIMD_Vector3::blend(ray_hit.dO_dx, du_dx * edge_1 + dv_dx * edge_2, mask);
	ray_hit.dO_dy = SIMD_Vector3::blend(ray_hit.dO_dy, du_dy * edge_1 + dv_dy * edge_2, mask);

	// Calculate derivative of the non-normalized vector n
	SIMD_Vector3 dn_dx = du_dx * n_edge_1 + dv_dx * n_edge_2;
	SIMD_Vector3 dn_dy = du_dy * n_edge_1 + dv_dy * n_edge_2;

	// Calculate derivative of the normalized vector N
	SIMD_float n_dot_n = SIMD_Vector3::dot(n, n);
	SIMD_float N_denom = SIMD_float::inv_sqrt(n_dot_n) / n_dot_n;

	ray_hit.dN_dx = SIMD_Vector3::blend(ray_hit.dN_dx, (n_dot_n * dn_dx - SIMD_Vector3::dot(n, dn_dx) * n) * N_denom, mask);
	ray_hit.dN_dy = SIMD_Vector3::blend(ray_hit.dN_dy, (n_dot_n * dn_dy - SIMD_Vector3::dot(n, dn_dy) * n) * N_denom, mask);

	ray_hit.ds_dx = SIMD_float::blend(ray_hit.ds_dx, du_dx * t_edge_1.x + dv_dx * t_edge_2.x, mask);
	ray_hit.ds_dy = SIMD_float::blend(ray_hit.ds_dy, du_dy * t_edge_1.x + dv_dy * t_edge_2.x, mask);
	ray_hit.dt_dx = SIMD_float::blend(ray_hit.dt_dx, du_dx * t_edge_1.y + dv_dx * t_edge_2.y, mask);
	ray_hit.dt_dy = SIMD_float::blend(ray_hit.dt_dy, du_dy * t_edge_1.y + dv_dy * t_edge_2.y, mask);
#endif
}

SIMD_float BottomLevelBVH::triangle_intersect(int index, const Ray & ray, SIMD_float max_distance) const {
	const SIMD_float zero(0.0f);
	const SIMD_float one (1.0f);

	SIMD_Vector3 edge_0(triangles_hot[index].position_edge_1);
	SIMD_Vector3 edge_1(triangles_hot[index].position_edge_2);

	SIMD_Vector3 h = SIMD_Vector3::cross(ray.direction, edge_1);
	SIMD_float   a = SIMD_Vector3::dot(edge_0, h);

	SIMD_float   f = SIMD_float::rcp(a);
	SIMD_Vector3 s = ray.origin - SIMD_Vector3(triangles_hot[index].position_0);
	SIMD_float   u = f * SIMD_Vector3::dot(s, h);

	// If the barycentric coordinate on the edge between vertices i and i+1 
	// is outside the interval [0, 1] we know no intersection is possible
	SIMD_float mask = (u > zero) & (u < one);
	if (SIMD_float::all_false(mask)) return mask;

	SIMD_Vector3 q = SIMD_Vector3::cross(s, edge_0);
	SIMD_float   v = f * SIMD_Vector3::dot(ray.direction, q);

	// If the barycentric coordinate on the edge between vertices i and i+2 
	// is outside the interval [0, 1] we know no intersection is possible
	mask = mask & (v       > zero);
	mask = mask & ((u + v) < one);
	if (SIMD_float::all_false(mask)) return mask;

	SIMD_float t = f * SIMD_Vector3::dot(edge_1, q);

	// Check if we are in the right distance range
	mask = mask & (t > Ray::EPSILON);
	mask = mask & (t < max_distance);

	return mask;
}

// Possible hints:
// _MM_HINT_NTA - non-temporal
// _MM_HINT_T0  - L1, L2, and L3 cache
// _MM_HINT_T1  -     L2, and L3 cache
// _MM_HINT_T2  -             L3 cache
#define PREFETCH_HINT _MM_HINT_T0

void BottomLevelBVH::trace(const Ray & ray, RayHit & ray_hit, const Matrix4 & world) const {
	int stack[BVH_TRAVERSAL_STACK_SIZE];
	int stack_size = 1;

	// Push root on stack
	stack[0] = 0;

	int steps = 0;

	SIMD_Vector3 inv_direction = SIMD_Vector3::rcp(ray.direction);

	while (stack_size > 0) {
		// Pop Node of the stack
		const BVHNode & node = nodes[stack[--stack_size]];

		SIMD_float mask = node.aabb.intersect(ray, inv_direction, ray_hit.distance);
		if (SIMD_float::all_false(mask)) continue;

		if (node.is_leaf()) {
			for (int i = node.first; i < node.first + node.count; i++) {
				triangle_trace(i, ray, ray_hit, world);
			}
		} else {
			// Prefetch the cacheline containing the children of the current Node
			_mm_prefetch(reinterpret_cast<const char *>(nodes + node.left), PREFETCH_HINT);

			if (node.should_visit_left_first(ray)) {
				stack[stack_size++] = node.left + 1;
				stack[stack_size++] = node.left;
			} else {
				stack[stack_size++] = node.left;
				stack[stack_size++] = node.left + 1;
			}
		}

		steps++;
	}
	
#if BVH_VISUALIZE_HEATMAP
	ray_hit.bvh_steps += steps;
#endif
}

SIMD_float BottomLevelBVH::intersect(const Ray & ray, SIMD_float max_distance) const {
	int stack[BVH_TRAVERSAL_STACK_SIZE];
	int stack_size = 1;

	// Push root on stack
	stack[0] = 0;

	SIMD_float hit(0.0f);
	
	SIMD_Vector3 inv_direction = SIMD_Vector3::rcp(ray.direction);

	while (stack_size > 0) {
		// Pop Node of the stack
		const BVHNode & node = nodes[stack[--stack_size]];

		SIMD_float mask = node.aabb.intersect(ray, inv_direction, max_distance);
		if (SIMD_float::all_false(mask)) continue;

		if (node.is_leaf()) {
			for (int i = node.first; i < node.first + node.count; i++) {
				hit = hit | triangle_intersect(i, ray, max_distance);

				if (SIMD_float::all_true(hit)) return hit;
			}
		} else {
			// Prefetch the cacheline containing the children of the current Node
			_mm_prefetch(reinterpret_cast<const char *>(nodes + node.left), PREFETCH_HINT);

			if (node.should_visit_left_first(ray)) {
				stack[stack_size++] = node.left + 1;
				stack[stack_size++] = node.left;
			} else {
				stack[stack_size++] = node.left;
				stack[stack_size++] = node.left + 1;
			}
		}
	}

	return hit;
}
