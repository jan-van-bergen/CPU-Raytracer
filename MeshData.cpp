#include "MeshData.h"

#include <vector>
#include <filesystem>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Util.h"
#include "ScopeTimer.h"

#define MESH_USE_BVH  0
#define MESH_USE_SBVH 1

#define MESH_ACCELERATOR MESH_USE_SBVH

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

	const char * path = Util::get_path(file_path);

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, file_path, path);

	if (shapes.size() == 0) abort(); // Either the model is empty, or something went wrong
	
	mesh_data = new MeshData();
	
	int material_base_index = Material::materials.size();

	// Load Materials
	int material_count = materials.size();
	if (material_count > 0) {
		for (int i = 0; i < material_count; i++) {
			const tinyobj::material_t & material = materials[i];

			Material new_material;
			new_material.diffuse = Vector3(material.diffuse[0], material.diffuse[1], material.diffuse[2]);

			if (material.diffuse_texname.length() > 0) {
				new_material.texture = Texture::load((std::string(path) + material.diffuse_texname).c_str());
			}

			new_material.reflection = Vector3(material.specular[0], material.specular[1], material.specular[2]);

			new_material.transmittance       = Vector3(material.transmittance[0], material.transmittance[1], material.transmittance[2]);
			new_material.index_of_refraction = material.ior;

			Material::materials.push_back(new_material);
		}
	} else {
		material_count = 1;

		Material new_material;
		new_material.diffuse = Vector3(1.0f, 0.0f, 1.0f);

		Material::materials.push_back(new_material);
	}
	
	delete [] path;

	// Load Meshes
	int total_vertex_count = 0;
	int max_vertex_count = -1;

	// Count total amount of vertices over all Shapes
	for (int s = 0; s < shapes.size(); s++) {
		int vertex_count = shapes[s].mesh.indices.size();
		total_vertex_count += vertex_count;

		if (vertex_count > max_vertex_count) {
			max_vertex_count = vertex_count;
		}
	}

	std::string bvh_filename = std::string(file_path) + ".bvh";

	if (std::filesystem::exists(bvh_filename)) {
		printf("Loading BVH %s from disk.\n", bvh_filename.c_str());

		mesh_data->triangle_bvh.load_from_disk(bvh_filename.c_str());
	} else {
		mesh_data->triangle_bvh.init(total_vertex_count / 3);

		Vector3 * positions  = new Vector3[max_vertex_count];
		Vector2 * tex_coords = new Vector2[max_vertex_count];
		Vector3 * normals    = new Vector3[max_vertex_count];

		int triangle_offset = 0;

		for (int s = 0; s < shapes.size(); s++) {
			int vertex_count = shapes[s].mesh.indices.size();
			assert(vertex_count % 3 == 0);

			// Iterate over vertices and assign attributes
			for (int v = 0; v < vertex_count; v++) {
				int vertex_index    = shapes[s].mesh.indices[v].vertex_index;
				int tex_coord_index = shapes[s].mesh.indices[v].texcoord_index;
				int normal_index    = shapes[s].mesh.indices[v].normal_index;
			
				if (vertex_index != INVALID) {
					positions[v] = Vector3(
						attrib.vertices[3*vertex_index    ], 
						attrib.vertices[3*vertex_index + 1], 
						attrib.vertices[3*vertex_index + 2]
					);
				}
				if (tex_coord_index != INVALID) {
					tex_coords[v] = Vector2(
							   attrib.texcoords[2*tex_coord_index    ], 
						1.0f - attrib.texcoords[2*tex_coord_index + 1] // Flip uv along y
					);
				}
				if (normal_index != INVALID) {
					normals[v] = Vector3(
						attrib.normals[3*normal_index    ], 
						attrib.normals[3*normal_index + 1], 
						attrib.normals[3*normal_index + 2]
					);
				}
			}

			// Iterate over faces
			for (int v = 0; v < vertex_count / 3; v++) {
				mesh_data->triangle_bvh.primitives[triangle_offset + v].position0 = positions[3*v    ];
				mesh_data->triangle_bvh.primitives[triangle_offset + v].position1 = positions[3*v + 1];
				mesh_data->triangle_bvh.primitives[triangle_offset + v].position2 = positions[3*v + 2];

				mesh_data->triangle_bvh.primitives[triangle_offset + v].tex_coord0 = tex_coords[3*v    ];
				mesh_data->triangle_bvh.primitives[triangle_offset + v].tex_coord1 = tex_coords[3*v + 1];
				mesh_data->triangle_bvh.primitives[triangle_offset + v].tex_coord2 = tex_coords[3*v + 2];

				mesh_data->triangle_bvh.primitives[triangle_offset + v].normal0 = normals[3*v    ];
				mesh_data->triangle_bvh.primitives[triangle_offset + v].normal1 = normals[3*v + 1];
				mesh_data->triangle_bvh.primitives[triangle_offset + v].normal2 = normals[3*v + 2];

				int material_id = shapes[s].mesh.material_ids[v];
				if (material_id == INVALID) material_id = 0;
			
				assert(material_id < material_count);

				mesh_data->triangle_bvh.primitives[triangle_offset + v].material_id = material_base_index + material_id;
			}
		
			triangle_offset += vertex_count / 3;
		}

		assert(triangle_offset == mesh_data->triangle_bvh.primitive_count);

		printf("Loaded Mesh %s from disk, consisting of %u triangles.\n", file_path, mesh_data->triangle_bvh.primitive_count);
	
		delete [] positions;
		delete [] tex_coords;
		delete [] normals;
		
		for (int i = 0; i < mesh_data->triangle_bvh.primitive_count; i++) {
			mesh_data->triangle_bvh.primitives[i].calc_aabb();
		}

#if MESH_ACCELERATOR == MESH_USE_BVH
		{
			ScopeTimer timer("Mesh BVH Construction");
			mesh_data->triangle_bvh.build_bvh();
		}
#elif MESH_ACCELERATOR == MESH_USE_SBVH
		{
			ScopeTimer timer("Mesh SBVH Construction");
			mesh_data->triangle_bvh.build_sbvh();
		}
#endif
		mesh_data->triangle_bvh.save_to_disk(bvh_filename.c_str());
	}

	return mesh_data;
}
