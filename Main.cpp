#include <cstdio>

#include <GL/glew.h>

#include "Window.h"
#include "Scene.h"

// Forces NVIDIA driver to be used 
extern "C" { _declspec(dllexport) unsigned NvOptimusEnablement = 0x00000001; }

// OpenGL Debug Callback function to report errors
void GLAPIENTRY glMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * userParam) {
	printf("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "", type, severity, message);

	__debugbreak();
}

#define SCREEN_WIDTH  512
#define SCREEN_HEIGHT 512

int main(int argument_count, char ** arguments) {
	Window window(SCREEN_WIDTH, SCREEN_HEIGHT, "Raytracer");

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(glMessageCallback, NULL);
#endif

	Uint64 now  = 0;
	Uint64 last = 0;
	float inv_perf_freq = 1.0f / (float)SDL_GetPerformanceFrequency();
	float delta_time = 0;

	last = SDL_GetPerformanceCounter();

	Scene scene;
	scene.camera.resize(SCREEN_WIDTH, SCREEN_HEIGHT);

	while (!window.is_closed) {
		window.clear();

		scene.camera.update(delta_time, SDL_GetKeyboardState(0));
		scene.update(window);

		window.update();

		now = SDL_GetPerformanceCounter();
		delta_time = float(now - last) * inv_perf_freq;
		last = now;

		printf("Delta: %f ms\n", delta_time * 1000.0f);
	}

	return 0;
}
