#include "ModelData.h"

#include <vector>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Util.h"

static std::unordered_map<std::string, ModelData *> cache;

const ModelData * ModelData::load(const char * file_path) {
	ModelData *& model_data = cache[file_path];

	// If the cache already contains this Model Data simply return it
	if (model_data) return model_data;

	// Otherwise, load new MeshData
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warning;
	std::string error;

	const char * path = get_path(file_path);

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, file_path, path);

	if (shapes.size() == 0) abort(); // Either the model is empty, or something went wrong

	model_data = new ModelData();
	model_data->mesh_data_count = int(shapes.size());
	model_data->mesh_data       = new MeshData[model_data->mesh_data_count];

	for (int m = 0; m < model_data->mesh_data_count; m++) {
		int vertex_count = shapes[m].mesh.indices.size();
		assert(vertex_count % 3 == 0);

		MeshData * mesh_data = const_cast<MeshData *>(model_data->mesh_data + m);
		mesh_data->vertex_count = vertex_count;
		mesh_data->positions  = new Vector3[vertex_count];
		mesh_data->tex_coords = new Vector2[vertex_count];
		mesh_data->normals    = new Vector3[vertex_count];

		// We assume Material ids are constant over the whole Mesh
		int material_id = shapes[m].mesh.material_ids[0];
		if (material_id != -1) {
			tinyobj::material_t material = materials[material_id];

			mesh_data->material.diffuse = Vector3(material.diffuse[0], material.diffuse[1], material.diffuse[2]);

			if (material.diffuse_texname.length() > 0) {
				mesh_data->material.texture = Texture::load((std::string(path) + material.diffuse_texname).c_str());
			}

			mesh_data->material.reflection = Vector3(material.specular[0], material.specular[1], material.specular[2]);

			mesh_data->material.transmittance       = Vector3(material.transmittance[0], material.transmittance[1], material.transmittance[2]);
			mesh_data->material.index_of_refraction = material.ior;

			/*switch (material.illum) {
				case 0: case 1:                 mesh_data->material.type = Material::Type::DIFFUSE;    break;
				case 3: case 8:                 mesh_data->material.type = Material::Type::MIRROR;     break;
				case 4: case 5: case 6: case 7: mesh_data->material.type = Material::Type::DIELECTRIC; break;
			}*/
		} else {
			mesh_data->material.diffuse = Vector3(1.0f, 0.0f, 1.0f);
		}

		for (int i = 0; i < vertex_count; i++) {
			int vertex_index    = shapes[m].mesh.indices[i].vertex_index;
			int tex_coord_index = shapes[m].mesh.indices[i].texcoord_index;
			int normal_index    = shapes[m].mesh.indices[i].normal_index;

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

		printf("Loaded Mesh %s from disk, consisting of %u vertices.\n", file_path, model_data->mesh_data[m].vertex_count);
	}
	
	delete[] path;

	return model_data;
}
