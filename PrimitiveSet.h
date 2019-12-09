#pragma once
#include "Mesh.h"

struct Meshes {
	Mesh * meshes;
	int    mesh_count;

	inline void init(int count) {
		meshes     = new Mesh[count];
		mesh_count = count;
	}

	inline void update() const {
		for (int i = 0; i < mesh_count; i++) {
			meshes[i].update();
		}
	}

	inline void trace(int index, const Ray & ray, RayHit & ray_hit) const {
		assert(index >= 0 && index < mesh_count);

		meshes[index].trace(ray, ray_hit);
	}

	inline SIMD_float intersect(int index, const Ray & ray, SIMD_float max_distance) const {
		assert(index >= 0 && index < mesh_count);

		return meshes[index].intersect(ray, max_distance);
	}

	inline void expand(int index, AABB & aabb) const {
		assert(index >= 0 && index < mesh_count);

		for (int t = 0; t < meshes[index].mesh_data->triangle_count; t++) {
			meshes[index].world_positions[t].expand(aabb);
		}
	}

	inline Vector3 get_position(int index) const {
		assert(index >= 0 && index < mesh_count);
		return meshes[index].transform.position;
	}
};

struct Triangles {
	int triangle_count;
	TrianglePositions * positions;
	TriangleNormals   * normals;

	MeshData * mesh_data;

	Matrix4 world_matrix;

	inline void init(int count) {
		
	}

	inline void update() const {
		for (int i = 0; i < triangle_count; i++) {
			positions[i].position0 = Matrix4::transform_position(world_matrix, mesh_data->positions[i].position0);
			positions[i].position1 = Matrix4::transform_position(world_matrix, mesh_data->positions[i].position1);
			positions[i].position2 = Matrix4::transform_position(world_matrix, mesh_data->positions[i].position2);

			normals[i].normal0 = Matrix4::transform_direction(world_matrix, mesh_data->normals[i].normal0);
			normals[i].normal1 = Matrix4::transform_direction(world_matrix, mesh_data->normals[i].normal1);
			normals[i].normal2 = Matrix4::transform_direction(world_matrix, mesh_data->normals[i].normal2);
		}
	}

	inline void trace(int index, const Ray & ray, RayHit & ray_hit) const;

	inline SIMD_float intersect(int index, const Ray & ray, SIMD_float max_distance) const;

	inline void expand(int index, AABB & aabb) const {
		assert(index >= 0 && index < triangle_count);

		positions[index].expand(aabb);
	}

	inline Vector3 get_position(int index) const {
		assert(index >= 0 && index < triangle_count);
		return (positions[index].position0 + positions[index].position1 + positions[index].position2) / 3.0f;
	}
};
