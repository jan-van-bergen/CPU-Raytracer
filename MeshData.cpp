#include "MeshData.h"

#include <vector>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Hash.h"
#include "Util.h"

static std::unordered_map<const char *, MeshData *, StringHash, StringCompare> cache;

const MeshData * MeshData::load(const char * file_path) {
	MeshData *& mesh_data = cache[file_path];

	// If the cache already contains this MeshData simply return it
	if (mesh_data) return mesh_data;

	// Otherwise, load new MeshData
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warning;
	std::string error;

	const char * path = get_path(file_path);

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, file_path, path);

	delete[] path;

	if (shapes.size() == 0) abort(); // Either the model is empty, or something went wrong

	int vertex_count         = shapes[0].mesh.indices.size();
	int vertex_count_rounded = ((vertex_count - 1) / 12 + 1) * 12;

	mesh_data = new MeshData();
	mesh_data->vertex_count = vertex_count_rounded;
	mesh_data->position_x = new float[vertex_count_rounded];
	mesh_data->position_y = new float[vertex_count_rounded];
	mesh_data->position_z = new float[vertex_count_rounded];
	mesh_data->tex_coords = new Vector2[vertex_count_rounded];
	mesh_data->normals    = new Vector3[vertex_count_rounded];

	// First copy the vertices into 3 temporary AoS buffers
	Vector3 * temp_pos = new Vector3[vertex_count_rounded];
	Vector2 * temp_tex = new Vector2[vertex_count_rounded];
	Vector3 * temp_nor = new Vector3[vertex_count_rounded];

	for (int i = 0; i < vertex_count; i++) {
		int vertex_index    = shapes[0].mesh.indices[i].vertex_index;
		int tex_coord_index = shapes[0].mesh.indices[i].texcoord_index;
		int normal_index    = shapes[0].mesh.indices[i].normal_index;

		if (vertex_index != INVALID) {
			temp_pos[i] = Vector3(
				attrib.vertices[3*vertex_index    ], 
				attrib.vertices[3*vertex_index + 1], 
				attrib.vertices[3*vertex_index + 2]
			);
		}
		if (tex_coord_index != INVALID) {
			temp_tex[i] = Vector2(
				attrib.texcoords[2*tex_coord_index    ], 
				attrib.texcoords[2*tex_coord_index + 1]
			);
		}
		if (normal_index != INVALID) {
			temp_nor[i] = Vector3(
				attrib.normals[3*normal_index    ], 
				attrib.normals[3*normal_index + 1], 
				attrib.normals[3*normal_index + 2]
			);
		}
	}

	// Pad the vertices with nonsense data until a multiple of 12 is reached
	for (int i = vertex_count; i < vertex_count_rounded; i++) {
		temp_pos[i] = Vector3(0.0f, 0.0f, 0.0f);
		temp_tex[i] = Vector2(0.0f, 0.0f);
		temp_nor[i] = Vector3(0.0f, 0.0f, 0.0f);
	}

	// Permute the data so we end up with SoA buffers where the 3 vertices 0,1,2 of every 4 triangles a,b,c,d are arranged as such:
	// a0,b0,c0,d0, a1,b1,c1,d1, a2,b2,c2,d2
	int permutations[12] = {
		0, 3, 6, 9,
		1, 4, 7, 10,
		2, 5, 8, 11
	};
	
	for (int i = 0; i < vertex_count_rounded; i += 12) {
		for (int j = 0; j < 12; j++) {
			int index       = i + j;
			int permutation = i + permutations[j];

			mesh_data->position_x[index] = temp_pos[permutation].x;
			mesh_data->position_y[index] = temp_pos[permutation].y;
			mesh_data->position_z[index] = temp_pos[permutation].z;

			mesh_data->normals[index] = temp_nor[permutation];
			
			mesh_data->tex_coords[index] = temp_tex[permutation];
		}
	}

	// Clean up temporary buffers
	delete[] temp_pos;
	delete[] temp_tex;
	delete[] temp_nor;

	printf("Loaded Mesh %s from disk, consisting of %u vertices.\n", file_path, mesh_data->vertex_count);

	return mesh_data;
}
