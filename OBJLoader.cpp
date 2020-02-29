#include "OBJLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Material.h"

static int load_materials(BottomLevelBVH * bvh, std::vector<tinyobj::material_t> & materials, const char * path) {
	bvh->material_offset = MaterialBuffer::material_count;

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

			MaterialBuffer::add(new_material);
		}
	} else {
		material_count = 1;

		Material new_material;
		new_material.diffuse = Vector3(1.0f, 0.0f, 1.0f);

		MaterialBuffer::add(new_material);
	}

	return material_count;
}

void OBJLoader::load_mtl(BottomLevelBVH * bvh, const char * filename) {
	// Load only the mtl file
	std::map<std::string, int> material_map;
	std::vector<tinyobj::material_t> materials;

	std::string warning;
	std::string error;

	std::string str(filename);

	std::filebuf fb;
	if (fb.open(str.substr(0, str.length() - 4) + ".mtl", std::ios::in)) {
		std::istream is(&fb);

		tinyobj::LoadMtl(&material_map, &materials, &is, &warning, &error);
		
		const char * path = Util::get_path(filename);

		load_materials(bvh, materials, path);

		delete [] path;
	} else {
		printf("WARNING: mtl file for %s not found! Make sure the .mtl file has the same name as the .obj file.\n", filename);
	}
}

const Triangle * OBJLoader::load_obj(BottomLevelBVH * bvh, const char * filename) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warning;
	std::string error;
	
	const char * path = Util::get_path(filename);

	bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, filename, path);
	if (!success) {
		printf("Failed to load obj file %s!\n", filename);
		printf("Warning: %s\n", warning.c_str());
		printf("Error:   %s\n", error.c_str());

		abort();
	}

	int material_count = load_materials(bvh, materials, path);

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

	int        triangle_count = total_vertex_count / 3;
	Triangle * triangles = new Triangle[triangle_count];

	bvh->init(triangle_count);
		
	int triangle_offset = 0;

	for (int s = 0; s < shapes.size(); s++) {
		int vertex_count = shapes[s].mesh.indices.size();
		assert(vertex_count % 3 == 0);

		// Iterate over faces
		for (int f = 0; f < vertex_count / 3; f++) {
			int index_0 = 3*f;
			int index_1 = 3*f + 1;
			int index_2 = 3*f + 2;

			// Get indices for the positions, texcoords, and normals for the current Triangle face
			int index_position_0 = 3 * shapes[s].mesh.indices[index_0].vertex_index;
			int index_position_1 = 3 * shapes[s].mesh.indices[index_1].vertex_index;
			int index_position_2 = 3 * shapes[s].mesh.indices[index_2].vertex_index;

			int index_tex_coord_0 = 2 * shapes[s].mesh.indices[index_0].texcoord_index;
			int index_tex_coord_1 = 2 * shapes[s].mesh.indices[index_1].texcoord_index;
			int index_tex_coord_2 = 2 * shapes[s].mesh.indices[index_2].texcoord_index;

			int index_normal_0 = 3 * shapes[s].mesh.indices[index_0].normal_index;
			int index_normal_1 = 3 * shapes[s].mesh.indices[index_1].normal_index;
			int index_normal_2 = 3 * shapes[s].mesh.indices[index_2].normal_index;

			// Obtain positions, texcoords, and normals by indexing the buffers
			Vector3 position_0 = Vector3(attrib.vertices[index_position_0], attrib.vertices[index_position_0 + 1], attrib.vertices[index_position_0 + 2]);
			Vector3 position_1 = Vector3(attrib.vertices[index_position_1], attrib.vertices[index_position_1 + 1], attrib.vertices[index_position_1 + 2]);
			Vector3 position_2 = Vector3(attrib.vertices[index_position_2], attrib.vertices[index_position_2 + 1], attrib.vertices[index_position_2 + 2]);
				
			Vector2 tex_coord_0 = index_tex_coord_0 >= 0 ? Vector2(attrib.texcoords[index_tex_coord_0], 1.0f - attrib.texcoords[index_tex_coord_0 + 1]) : Vector2(0.0f, 0.0f);
			Vector2 tex_coord_1 = index_tex_coord_1 >= 0 ? Vector2(attrib.texcoords[index_tex_coord_1], 1.0f - attrib.texcoords[index_tex_coord_1 + 1]) : Vector2(0.0f, 0.0f);
			Vector2 tex_coord_2 = index_tex_coord_2 >= 0 ? Vector2(attrib.texcoords[index_tex_coord_2], 1.0f - attrib.texcoords[index_tex_coord_2 + 1]) : Vector2(0.0f, 0.0f);

			Vector3 normal_0 = Vector3(attrib.normals[index_normal_0], attrib.normals[index_normal_0 + 1], attrib.normals[index_normal_0 + 2]);
			Vector3 normal_1 = Vector3(attrib.normals[index_normal_1], attrib.normals[index_normal_1 + 1], attrib.normals[index_normal_1 + 2]);
			Vector3 normal_2 = Vector3(attrib.normals[index_normal_2], attrib.normals[index_normal_2 + 1], attrib.normals[index_normal_2 + 2]);
			
			int index_triangle = triangle_offset + f;

			// Store positions in the AoS buffer used for BVH construction
			triangles[index_triangle].position_0 = position_0;
			triangles[index_triangle].position_1 = position_1;
			triangles[index_triangle].position_2 = position_2;

			triangles[index_triangle].calc_aabb();

			// Store positions, texcoords, and normals in SoA layout in the BVH itself
			bvh->triangles_hot[index_triangle].position_0      = position_0;
			bvh->triangles_hot[index_triangle].position_edge_1 = position_1 - position_0;
			bvh->triangles_hot[index_triangle].position_edge_2 = position_2 - position_0;

			bvh->triangles_cold[index_triangle].tex_coord_0      = tex_coord_0;
			bvh->triangles_cold[index_triangle].tex_coord_edge_1 = tex_coord_1 - tex_coord_0;
			bvh->triangles_cold[index_triangle].tex_coord_edge_2 = tex_coord_2 - tex_coord_0;

			bvh->triangles_cold[index_triangle].normal_0      = normal_0;
			bvh->triangles_cold[index_triangle].normal_edge_1 = normal_1 - normal_0;
			bvh->triangles_cold[index_triangle].normal_edge_2 = normal_2 - normal_0;

			// Lookup and store material id
			int material_id = shapes[s].mesh.material_ids[f];
			if (material_id == INVALID) material_id = 0;
			
			assert(material_id < material_count);

			bvh->triangles_cold[index_triangle].material_id = material_id;
		}
		
		triangle_offset += vertex_count / 3;
	}

	assert(triangle_offset == triangle_count);

	printf("Loaded Mesh %s from disk, consisting of %u triangles.\n", filename, triangle_count);

	delete [] path;

	return triangles;
}