#include "Mesh.h"

#include "Math3d.h"

void Mesh::update() {
	transform.calc_world_matrix();

	for (int i = 0; i < mesh_data->vertex_count; i++) {
		world_positions[i] = Matrix4::transform_position (transform.world_matrix, mesh_data->positions[i]);
		world_normals  [i] = Matrix4::transform_direction(transform.world_matrix, mesh_data->normals  [i]);
	}
}

void Mesh::trace(const Ray & ray, RayHit & ray_hit) const {
	// Iterate over all Triangles in the Mesh
	for (int i = 0; i < mesh_data->vertex_count; i += 3) {
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
        if (t < EPSILON || t > ray_hit.distance) continue;
		
		ray_hit.hit      = true;
		ray_hit.distance = t;

		ray_hit.point  = ray.origin + ray.direction * t;
        ray_hit.normal = Vector3::normalize(Math3d::barycentric(world_normals[i], world_normals[i+1], world_normals[i+2], u, v));

		// Obtain u,v by barycentric interpolation of the texture coordinates of the three current vertices
        Vector2 tex_coords = Math3d::barycentric(mesh_data->tex_coords[i], mesh_data->tex_coords[i+1], mesh_data->tex_coords[i+2], u, v);
		ray_hit.u = tex_coords.x;
		ray_hit.v = tex_coords.y;
			
		ray_hit.material = &material;
	}
}

bool Mesh::intersect(const Ray & ray, float max_distance) const {
	// Iterate over all Triangles in the Mesh
	for (int i = 0; i < mesh_data->vertex_count; i += 3) {
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
	}

	return false;
}
