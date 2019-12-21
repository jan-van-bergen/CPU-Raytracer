#pragma once
#include <cassert>

#include "MeshData.h"
#include "Transform.h"

#include "Ray.h"
#include "RayHit.h"

#include "BVH.h"

#define MESH_USE_BVH  0
#define MESH_USE_SBVH 1

#define MESH_ACCELERATOR MESH_USE_BVH

struct Mesh {
	Transform transform;

	const MeshData * mesh_data = nullptr;

	BVH<Triangle> triangle_bvh;

	inline void init(const char * file_path)  {
		mesh_data = MeshData::load(file_path);
		triangle_bvh.init(mesh_data->triangle_count);
		
		update();

		aabb = AABB::create_empty();

		// Copy Texture Coordinates and Material
		for (int i = 0; i < mesh_data->triangle_count; i++) {
			triangle_bvh.primitives[i].tex_coord0 = mesh_data->triangles[i].tex_coord0;
			triangle_bvh.primitives[i].tex_coord1 = mesh_data->triangles[i].tex_coord1;
			triangle_bvh.primitives[i].tex_coord2 = mesh_data->triangles[i].tex_coord2;

			triangle_bvh.primitives[i].material = mesh_data->triangles[i].material;

			Vector3 vertices[3] = { 
				triangle_bvh.primitives[i].position0, 
				triangle_bvh.primitives[i].position1, 
				triangle_bvh.primitives[i].position2 
			};
			triangle_bvh.primitives[i].aabb = AABB::from_points(vertices, 3);

			aabb.expand(triangle_bvh.primitives[i].aabb);
		}

		aabb.fix_if_needed();

#if MESH_ACCELERATOR == MESH_USE_BVH
		triangle_bvh.build_bvh();
#elif MESH_ACCELERATOR == MESH_USE_SBVH
		triangle_bvh.build_sbvh();
#endif
		//triangle_bvh.debug();
	}

	void update();

	void trace(const Ray & ray, RayHit & ray_hit, int bvh_step) const {
		triangle_bvh.trace(ray, ray_hit);
	}

	SIMD_float intersect(const Ray & ray, SIMD_float max_distance) const {
		return triangle_bvh.intersect(ray, max_distance);
	}
	
	// BVH Related methods
	AABB aabb;

	inline Vector3 get_position() const {
		return transform.position;
	}
};
