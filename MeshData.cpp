#include "MeshData.h"

#include <vector>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#include "Hash.h"

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

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, file_path);

	int vertex_count         = shapes[0].mesh.indices.size();
	int vertex_count_rounded = ((vertex_count - 1) / 12 + 1) * 12;

	mesh_data = new MeshData();
	mesh_data->vertex_count = vertex_count_rounded;
	mesh_data->position_x = new float[vertex_count_rounded];
	mesh_data->position_y = new float[vertex_count_rounded];
	mesh_data->position_z = new float[vertex_count_rounded];
	mesh_data->tex_coords = new Vector2[vertex_count_rounded];
	mesh_data->normals    = new Vector3[vertex_count_rounded];

	Vector3 * temp_pos = new Vector3[vertex_count_rounded];
	Vector2 * temp_tex = new Vector2[vertex_count_rounded];
	Vector3 * temp_nor = new Vector3[vertex_count_rounded];

	for (int i = 0; i < vertex_count; i++) {
		int vertex_index    = shapes[0].mesh.indices[i].vertex_index;
		int tex_coord_index = shapes[0].mesh.indices[i].texcoord_index;
		int normal_index    = shapes[0].mesh.indices[i].normal_index;

		temp_pos[i] = Vector3(
			attrib.vertices[3*vertex_index    ], 
			attrib.vertices[3*vertex_index + 1], 
			attrib.vertices[3*vertex_index + 2]
		);
		temp_tex[i] = Vector2(
			attrib.texcoords[2*tex_coord_index    ], 
			attrib.texcoords[2*tex_coord_index + 1]
		);
		temp_nor[i] = Vector3(
			attrib.normals[3*normal_index    ], 
			attrib.normals[3*normal_index + 1], 
			attrib.normals[3*normal_index + 2]
		);
	}

	for (int i = vertex_count; i < vertex_count_rounded; i++) {
		temp_pos[i] = Vector3(NAN, NAN, NAN);
		temp_tex[i] = Vector2(NAN, NAN);
		temp_nor[i] = Vector3(NAN, NAN, NAN);
	}
	
	for (int i = 0; i < vertex_count_rounded; i += 12) {
		// First Triangle (index 0 stays in place)
		mesh_data->position_x[i    ] = temp_pos[i    ].x;
		mesh_data->position_y[i    ] = temp_pos[i    ].y;
		mesh_data->position_z[i    ] = temp_pos[i    ].z;
		mesh_data->position_x[i + 1] = temp_pos[i + 3].x;
		mesh_data->position_y[i + 1] = temp_pos[i + 3].y;
		mesh_data->position_z[i + 1] = temp_pos[i + 3].z;
		mesh_data->position_x[i + 2] = temp_pos[i + 6].x;
		mesh_data->position_y[i + 2] = temp_pos[i + 6].y;
		mesh_data->position_z[i + 2] = temp_pos[i + 6].z;

		mesh_data->normals[i    ] = temp_nor[i    ];
		mesh_data->normals[i + 1] = temp_nor[i + 3];
		mesh_data->normals[i + 2] = temp_nor[i + 6];

		mesh_data->tex_coords[i    ] = temp_tex[i    ];
		mesh_data->tex_coords[i + 1] = temp_tex[i + 3];
		mesh_data->tex_coords[i + 2] = temp_tex[i + 6];

		// Second Triangle
		mesh_data->position_x[i + 3] = temp_pos[i + 9].x;
		mesh_data->position_y[i + 3] = temp_pos[i + 9].y;
		mesh_data->position_z[i + 3] = temp_pos[i + 9].z;
		mesh_data->position_x[i + 4] = temp_pos[i + 1].x;
		mesh_data->position_y[i + 4] = temp_pos[i + 1].y;
		mesh_data->position_z[i + 4] = temp_pos[i + 1].z;
		mesh_data->position_x[i + 5] = temp_pos[i + 4].x;
		mesh_data->position_y[i + 5] = temp_pos[i + 4].y;
		mesh_data->position_z[i + 5] = temp_pos[i + 4].z;

		mesh_data->normals[i + 3] = temp_nor[i + 9];
		mesh_data->normals[i + 4] = temp_nor[i + 1];
		mesh_data->normals[i + 5] = temp_nor[i + 4];

		mesh_data->tex_coords[i + 3] = temp_tex[i + 9];
		mesh_data->tex_coords[i + 4] = temp_tex[i + 1];
		mesh_data->tex_coords[i + 5] = temp_tex[i + 4];

		// Third Triangle
		mesh_data->position_x[i + 6] = temp_pos[i + 7].x;
		mesh_data->position_y[i + 6] = temp_pos[i + 7].y;
		mesh_data->position_z[i + 6] = temp_pos[i + 7].z;
		mesh_data->position_x[i + 7] = temp_pos[i + 10].x;
		mesh_data->position_y[i + 7] = temp_pos[i + 10].y;
		mesh_data->position_z[i + 7] = temp_pos[i + 10].z;
		mesh_data->position_x[i + 8] = temp_pos[i + 2].x;
		mesh_data->position_y[i + 8] = temp_pos[i + 2].y;
		mesh_data->position_z[i + 8] = temp_pos[i + 2].z;

		mesh_data->normals[i + 6] = temp_nor[i + 7];
		mesh_data->normals[i + 7] = temp_nor[i + 10];
		mesh_data->normals[i + 8] = temp_nor[i + 2];

		mesh_data->tex_coords[i + 6] = temp_tex[i + 7];
		mesh_data->tex_coords[i + 7] = temp_tex[i + 10];
		mesh_data->tex_coords[i + 8] = temp_tex[i + 2];

		// Fourth Triangle (index 11 stays in place)
		mesh_data->position_x[i + 9]  = temp_pos[i + 5].x;
		mesh_data->position_y[i + 9]  = temp_pos[i + 5].y;
		mesh_data->position_z[i + 9]  = temp_pos[i + 5].z;
		mesh_data->position_x[i + 10] = temp_pos[i + 8].x;
		mesh_data->position_y[i + 10] = temp_pos[i + 8].y;
		mesh_data->position_z[i + 10] = temp_pos[i + 8].z;
		mesh_data->position_x[i + 11] = temp_pos[i + 11].x;
		mesh_data->position_y[i + 11] = temp_pos[i + 11].y;
		mesh_data->position_z[i + 11] = temp_pos[i + 11].z;

		mesh_data->normals[i + 9]  = temp_nor[i + 5];
		mesh_data->normals[i + 10] = temp_nor[i + 8];
		mesh_data->normals[i + 11] = temp_nor[i + 11];
		
		mesh_data->tex_coords[i + 9]  = temp_tex[i + 5];
		mesh_data->tex_coords[i + 10] = temp_tex[i + 8];
		mesh_data->tex_coords[i + 11] = temp_tex[i + 11];
	}

	delete[] temp_pos;
	delete[] temp_tex;
	delete[] temp_nor;

	return mesh_data;
}
