#include "MeshData.h"

#include <vector>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Util.h"

static std::unordered_map<std::string, MeshData *> cache;

const MeshData * MeshData::load(const char * file_path) {
	MeshData *& mesh_data = cache[file_path];

	// If the cache already contains this Model Data simply return it
	if (mesh_data) return mesh_data;

	// Otherwise, load new MeshData
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warning;
	std::string error;

	const char * path = get_path(file_path);

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, file_path, path);

	if (shapes.size() == 0) abort(); // Either the model is empty, or something went wrong

	mesh_data = new MeshData();

	int vertex_count = shapes[0].mesh.indices.size();
	assert(vertex_count % 3 == 0);

	mesh_data->vertex_count = vertex_count;
	mesh_data->positions    = new Vector3[vertex_count];
	mesh_data->tex_coords   = new Vector2[vertex_count];
	mesh_data->normals      = new Vector3[vertex_count];
	mesh_data->material_ids = new int    [vertex_count / 3];

	if (materials.size() > 0) {
		mesh_data->materials = new Material[materials.size()];

		for (int i = 0; i < materials.size(); i++) {
			const tinyobj::material_t & material = materials[i];

			mesh_data->materials[i].diffuse = Vector3(material.diffuse[0], material.diffuse[1], material.diffuse[2]);

			if (material.diffuse_texname.length() > 0) {
				mesh_data->materials[i].texture = Texture::load((std::string(path) + material.diffuse_texname).c_str());
			}

			mesh_data->materials[i].reflection = Vector3(material.specular[0], material.specular[1], material.specular[2]);

			mesh_data->materials[i].transmittance       = Vector3(material.transmittance[0], material.transmittance[1], material.transmittance[2]);
			mesh_data->materials[i].index_of_refraction = material.ior;
		}
	} else {
		mesh_data->materials = new Material();
		mesh_data->materials->diffuse = Vector3(1.0f, 0.0f, 1.0f);
	}

	// Iterate over vertices and assign attributes
	for (int i = 0; i < vertex_count; i++) {
		int vertex_index    = shapes[0].mesh.indices[i].vertex_index;
		int tex_coord_index = shapes[0].mesh.indices[i].texcoord_index;
		int normal_index    = shapes[0].mesh.indices[i].normal_index;
			
		if (vertex_index != INVALID) {
			mesh_data->positions[i] = Vector3(
				attrib.vertices[3*vertex_index    ], 
				attrib.vertices[3*vertex_index + 1], 
				attrib.vertices[3*vertex_index + 2]
			);
		}
		if (tex_coord_index != INVALID) {
			mesh_data->tex_coords[i] = Vector2(
				attrib.texcoords[2*tex_coord_index    ], 
				attrib.texcoords[2*tex_coord_index + 1]
			);
		}
		if (normal_index != INVALID) {
			mesh_data->normals[i] = Vector3(
				attrib.normals[3*normal_index    ], 
				attrib.normals[3*normal_index + 1], 
				attrib.normals[3*normal_index + 2]
			);
		}
	}

	// Iterate over faces and assign material ids
	for (int i = 0; i < vertex_count / 3; i++) {
		int material_id = shapes[0].mesh.material_ids[i];
		mesh_data->material_ids[i] = material_id == -1 ? 0 : material_id;
	}

	printf("Loaded Mesh %s from disk, consisting of %u vertices.\n", file_path, mesh_data->vertex_count);

	delete[] path;

	return mesh_data;
}
