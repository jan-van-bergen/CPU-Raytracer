#include "MeshData.h"

#include <vector>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

static std::unordered_map<const char *, MeshData *> cache;

const MeshData * MeshData::load(const char * file_path) {
	MeshData *& mesh_data = cache[file_path];

	if (mesh_data) return mesh_data;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warning;
	std::string error;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, file_path);

	mesh_data = new MeshData();
	mesh_data->vertex_count = shapes[0].mesh.indices.size();
	mesh_data->positions  = new Vector3[mesh_data->vertex_count];
	mesh_data->tex_coords = new Vector2[mesh_data->vertex_count];
	mesh_data->normals    = new Vector3[mesh_data->vertex_count];

	for (int i = 0; i < mesh_data->vertex_count; i++) {
		int vertex_index    = shapes[0].mesh.indices[i].vertex_index;
		int tex_coord_index = shapes[0].mesh.indices[i].texcoord_index;
		int normal_index    = shapes[0].mesh.indices[i].normal_index;

		mesh_data->positions[i] = Vector3(
			attrib.vertices[3*vertex_index    ], 
			attrib.vertices[3*vertex_index + 1], 
			attrib.vertices[3*vertex_index + 2]
		);
		mesh_data->tex_coords[i] = Vector2(
			attrib.texcoords[2*tex_coord_index    ], 
			attrib.texcoords[2*tex_coord_index + 1]
		);
		mesh_data->normals[i] = Vector3(
			attrib.normals[3*normal_index    ], 
			attrib.normals[3*normal_index + 1], 
			attrib.normals[3*normal_index + 2]
		);
	}

	return mesh_data;
}
