#pragma once
#include <cstdio>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "Math.h"
#include "Vector3.h"

// OpenGL Debug Callback function to report errors
inline void GLAPIENTRY glMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * userParam) {
	printf("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "", type, severity, message);

	__debugbreak();
}

struct Window {
private:
	SDL_Window *  window;
	SDL_GLContext context;
	
	unsigned * frame_buffer;
	GLuint     frame_buffer_handle;

public:
	const int width;
	const int height;
	
	const int tile_width  = 32;
	const int tile_height = 32;

	const int tile_count_x;
	const int tile_count_y;

	bool is_closed = false;

	Window(int width, int height, const char * title);
	~Window();

	void clear();

	void update();

	inline void plot(int x, int y, unsigned colour) const {
		frame_buffer[x + width * y] = colour;
	}

	inline void plot(int x, int y, const Vector3 & colour) const {
		assert(x < width);
		assert(y < height);

		int r = int(Math::clamp(colour.x * 255.0f, 0.0f, 255.0f));
		int g = int(Math::clamp(colour.y * 255.0f, 0.0f, 255.0f));
		int b = int(Math::clamp(colour.z * 255.0f, 0.0f, 255.0f));

		frame_buffer[x + width * y] = (r << 16) | (g << 8) | b;
	}

	inline void set_title(const char * title) {
		SDL_SetWindowTitle(window, title);
	}
};
