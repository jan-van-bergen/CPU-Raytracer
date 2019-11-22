#include "Mesh.h"

#include "Math3d.h"
#include "SSEUtil.h"

void Mesh::update() {
	transform.calc_world_matrix();

	for (int i = 0; i < mesh_data->vertex_count; i++) {
		//Vector3 world_position = Matrix4::transform_position(transform.world_matrix, mesh_data->positions[i]);
		world_positions_x[i] = mesh_data->position_x[i];
		world_positions_y[i] = mesh_data->position_y[i];
		world_positions_z[i] = mesh_data->position_z[i];

		world_normals[i] = mesh_data->normals[i]; // Matrix4::transform_direction(transform.world_matrix, mesh_data->normals[i]);
	}
}

void Mesh::trace(const Ray & ray, RayHit & ray_hit) const {
	__m128 ray_origin_x = _mm_set1_ps(ray.origin.x);
	__m128 ray_origin_y = _mm_set1_ps(ray.origin.y);
	__m128 ray_origin_z = _mm_set1_ps(ray.origin.z);

	__m128 ray_direction_x = _mm_set1_ps(ray.direction.x);
	__m128 ray_direction_y = _mm_set1_ps(ray.direction.y);
	__m128 ray_direction_z = _mm_set1_ps(ray.direction.z);

	__m128     epsilon = _mm_set1_ps( EPSILON);
	__m128 neg_epsilon = _mm_set1_ps(-EPSILON);

	__m128 zero = _mm_set1_ps(0.0f);
	__m128 one  = _mm_set1_ps(1.0f);

	// Iterate over all Triangles in the Mesh
	for (int i = 0; i < mesh_data->vertex_count; i += 12) {
		__m128 world_position_i_x = _mm_load_ps(world_positions_x + i);
		__m128 world_position_i_y = _mm_load_ps(world_positions_y + i);
		__m128 world_position_i_z = _mm_load_ps(world_positions_z + i);
		
		// Vector3 edge0 = world_positions[i+1] - world_positions[i];
		__m128 edge0_x = _mm_sub_ps(_mm_load_ps(world_positions_x + i + 4), world_position_i_x);
		__m128 edge0_y = _mm_sub_ps(_mm_load_ps(world_positions_y + i + 4), world_position_i_y);
		__m128 edge0_z = _mm_sub_ps(_mm_load_ps(world_positions_z + i + 4), world_position_i_z);

		// Vector3 edge1 = world_positions[i+2] - world_positions[i];
		__m128 edge1_x = _mm_sub_ps(_mm_load_ps(world_positions_x + i + 8), world_position_i_x);
		__m128 edge1_y = _mm_sub_ps(_mm_load_ps(world_positions_y + i + 8), world_position_i_y);
		__m128 edge1_z = _mm_sub_ps(_mm_load_ps(world_positions_z + i + 8), world_position_i_z);

		//Vector3 h = Vector3::cross(ray.direction, edge1);
		__m128 h_x = _mm_sub_ps(_mm_mul_ps(ray_direction_y, edge1_z), _mm_mul_ps(ray_direction_z, edge1_y));
		__m128 h_y = _mm_sub_ps(_mm_mul_ps(ray_direction_z, edge1_x), _mm_mul_ps(ray_direction_x, edge1_z));
		__m128 h_z = _mm_sub_ps(_mm_mul_ps(ray_direction_x, edge1_y), _mm_mul_ps(ray_direction_y, edge1_x));

        //float a = Vector3::dot(edge0, h);
		__m128 a = _mm_add_ps(_mm_add_ps(_mm_mul_ps(edge0_x, h_x), _mm_mul_ps(edge0_y, h_y)), _mm_mul_ps(edge0_z, h_z));

		// If the ray is parallel to the plane formed by 
		// the triangle no intersection is possible
        //if (a > -EPSILON && a < EPSILON) continue;
		int mask = _mm_movemask_ps(_mm_cmpgt_ps(a, neg_epsilon)) & _mm_movemask_ps(_mm_cmplt_ps(a, epsilon));
		if (mask == ALL_TRUE_128) continue;

        //float f = 1.0f / a;
		__m128 f = _mm_rcp_ps(a);
        //Vector3 s = ray.origin - world_positions[i];
		__m128 s_x = _mm_sub_ps(ray_origin_x, world_position_i_x);
		__m128 s_y = _mm_sub_ps(ray_origin_y, world_position_i_y);
		__m128 s_z = _mm_sub_ps(ray_origin_z, world_position_i_z);
        //float u = f * Vector3::dot(s, h);
		__m128 u = _mm_mul_ps(f, _mm_add_ps(_mm_add_ps(_mm_mul_ps(s_x, h_x), _mm_mul_ps(s_y, h_y)), _mm_mul_ps(s_z, h_z)));

		// If the barycentric coordinate on the edge between vertices i and i+1 
		// is outside the interval [0, 1] we know no intersection is possible
        //if (u < 0.0f || u > 1.0f) continue;
		mask |= _mm_movemask_ps(_mm_cmplt_ps(u, zero));
		mask |= _mm_movemask_ps(_mm_cmpgt_ps(u, one));
		if (mask == ALL_TRUE_128) continue;

        //Vector3 q = Vector3::cross(s, edge0);
		__m128 q_x = _mm_sub_ps(_mm_mul_ps(s_y, edge0_z), _mm_mul_ps(s_z, edge0_y));
		__m128 q_y = _mm_sub_ps(_mm_mul_ps(s_z, edge0_x), _mm_mul_ps(s_x, edge0_z));
		__m128 q_z = _mm_sub_ps(_mm_mul_ps(s_x, edge0_y), _mm_mul_ps(s_y, edge0_x));
        //float v = f * Vector3::dot(ray.direction, q);
		__m128 v = _mm_mul_ps(f, _mm_add_ps(_mm_add_ps(_mm_mul_ps(ray_direction_x, q_x), _mm_mul_ps(ray_direction_y, q_y)), _mm_mul_ps(ray_direction_z, q_z)));

		// If the barycentric coordinate on the edge between vertices i and i+2 
		// is outside the interval [0, 1] we know no intersection is possible
        //if (v < 0.0f || u + v > 1.0f) continue;
		mask |= _mm_movemask_ps(_mm_cmplt_ps(v,                zero));
		mask |= _mm_movemask_ps(_mm_cmpgt_ps(_mm_add_ps(u, v), one));
		if (mask == ALL_TRUE_128) continue;

        //float t = f * Vector3::dot(edge1, q);
		union { __m128 t; float ts[4]; };
		t = _mm_mul_ps(f, _mm_add_ps(_mm_add_ps(_mm_mul_ps(edge1_x, q_x), _mm_mul_ps(edge1_y, q_y)), _mm_mul_ps(edge1_z, q_z)));

		// Check if we are in the right distance range
        //if (t < EPSILON || t > ray_hit.distance) continue;
		mask |= _mm_movemask_ps(_mm_cmplt_ps(t, epsilon));
		if (mask == ALL_TRUE_128) continue;

		mask = ~mask; // Invert mask

		float us[4]; _mm_store_ps(us, u);
		float vs[4]; _mm_store_ps(vs, v);

		for (int j = 0; j < 4; j++) {
			if (mask & (1 << j) && ts[j] < ray_hit.distance) {
				ray_hit.hit      = true;
				ray_hit.distance = ts[j];

				int index = i + j;

				ray_hit.point  = ray.origin + ray.direction * ts[j];
				ray_hit.normal = Vector3::normalize(Math3d::barycentric(world_normals[index], world_normals[index+4], world_normals[index+8], us[j], vs[j]));

				// Obtain u,v by barycentric interpolation of the texture coordinates of the three current vertices
				Vector2 tex_coords = Math3d::barycentric(mesh_data->tex_coords[index], mesh_data->tex_coords[index+4], mesh_data->tex_coords[index+8], us[j], vs[j]);
				ray_hit.u = tex_coords.x;
				ray_hit.v = tex_coords.y;
			
				ray_hit.material = &material;
			}
		}
	}
}

bool Mesh::intersect(const Ray & ray, float max_distance) const {
	// Iterate over all Triangles in the Mesh
	/*for (int i = 0; i < mesh_data->vertex_count; i += 3) {
		Vector3 edge0 = world_positions[i+1] - world_positions[i];
		Vector3 edge1 = world_positions[i+2] - world_positions[i];

		Vector3 h = Vector3::cross(ray.direction, edge1);
        float a = Vector3::dot(edge0, h);

		// If the ray is parallel to the plane formed by 
		// the triangle no intersection is possible
        if (a > -EPSILON && a < EPSILON) continue;

        float f = 1.0f / a;
        Vector3 s = ray.origin - world_positions[i];
        float u = f * Vector3::dot(s, h);

		// If the barycentric coordinate on the edge between vertices i and i+1 
		// is outside the interval [0, 1] we know no intersection is possible
        if (u < 0.0f || u > 1.0f) continue;

        Vector3 q = Vector3::cross(s, edge0);
        float v = f * Vector3::dot(ray.direction, q);

		// If the barycentric coordinate on the edge between vertices i and i+2 
		// is outside the interval [0, 1] we know no intersection is possible
        if (v < 0.0f || u + v > 1.0f) continue;

        float t = f * Vector3::dot(edge1, q);

		// Check if we are in the right distance range
        if (t < EPSILON || t > max_distance) continue;
		
		return true;
	}*/

	return false;
}
