#include "Camera.h"

#include <SDL2/SDL.h>

void Camera::resize(int width, int height) {
	float half_width  = 0.5f * width;
	float half_height = 0.5f * height;

	// Distance to the viewing plane
	float d = half_width / tanf(0.5f * fov);

	// Initialize viewing pyramid vectors
	top_left_corner = Vector3(-half_width, half_height, d);
	x_axis = Vector3::normalize(Vector3(width,  0.0f,   0.0f));
	y_axis = Vector3::normalize(Vector3(0.0f,  -height, 0.0f));
}

Ray Camera::get_ray(float x, float y) const {
	Ray ray;
	ray.origin    = position;
	ray.direction = Vector3::normalize(top_left_corner_rotated + x * x_axis_rotated + y * y_axis_rotated);

	return ray;
}

void Camera::update(float delta, const unsigned char * keys) {
	// Move Camera around
	const float MOVEMENT_SPEED = 10.0f;
	const float ROTATION_SPEED = 2.0f;

	Vector3 right   = rotation * Vector3(1.0f, 0.0f, 0.0f);
	Vector3 up      = rotation * Vector3(0.0f, 1.0f, 0.0f);
	Vector3 forward = rotation * Vector3(0.0f, 0.0f, 1.0f);

	if (keys[SDL_SCANCODE_W]) position += forward * MOVEMENT_SPEED * delta;
	if (keys[SDL_SCANCODE_A]) position -= right   * MOVEMENT_SPEED * delta;
	if (keys[SDL_SCANCODE_S]) position -= forward * MOVEMENT_SPEED * delta;
	if (keys[SDL_SCANCODE_D]) position += right   * MOVEMENT_SPEED * delta;

	if (keys[SDL_SCANCODE_LSHIFT]) position -= up * MOVEMENT_SPEED * delta;
	if (keys[SDL_SCANCODE_SPACE])  position += up * MOVEMENT_SPEED * delta;

	if (keys[SDL_SCANCODE_UP])    rotation = Quaternion::axis_angle(right                    , -ROTATION_SPEED * delta) * rotation;
	if (keys[SDL_SCANCODE_DOWN])  rotation = Quaternion::axis_angle(right                    , +ROTATION_SPEED * delta) * rotation;
	if (keys[SDL_SCANCODE_LEFT])  rotation = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), -ROTATION_SPEED * delta) * rotation;
	if (keys[SDL_SCANCODE_RIGHT]) rotation = Quaternion::axis_angle(Vector3(0.0f, 1.0f, 0.0f), +ROTATION_SPEED * delta) * rotation;

	// Transform view pyramid according to rotation
	top_left_corner_rotated = rotation * top_left_corner;
	x_axis_rotated          = rotation * x_axis;
	y_axis_rotated          = rotation * y_axis;
}
