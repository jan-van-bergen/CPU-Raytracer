#include <cstdio>

#include <GL/glew.h>

#include "Window.h"
#include "Scene.h"

#include "WorkerThread.h"

// Forces NVIDIA driver to be used 
extern "C" { _declspec(dllexport) unsigned NvOptimusEnablement = 0x00000001; }

// OpenGL Debug Callback function to report errors
void GLAPIENTRY glMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * userParam) {
	printf("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "", type, severity, message);

	__debugbreak();
}

#define SCREEN_WIDTH  512
#define SCREEN_HEIGHT 512

#define TOTAL_TIMING_COUNT 100
float timings[TOTAL_TIMING_COUNT];
int   current_frame = 0;

int main(int argument_count, char ** arguments) {
	Window window(SCREEN_WIDTH, SCREEN_HEIGHT, "Raytracer");

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(glMessageCallback, NULL);
#endif

	// Initialize timing stuff
	Uint64 now  = 0;
	Uint64 last = 0;
	float inv_perf_freq = 1.0f / (float)SDL_GetPerformanceFrequency();
	float delta_time = 0;

	// Initialize Scene
	Scene scene;
	scene.camera.resize(SCREEN_WIDTH, SCREEN_HEIGHT);

	init_core_info();

	Params  parameters[THREAD_COUNT];
	HANDLE workers   [THREAD_COUNT];

	// spawn worker threads
	for (int i = 0; i < THREAD_COUNT; i++) {
		go_signal  [i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		done_signal[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		parameters[i].thread_id = i;
		parameters[i].scene     = &scene;
		parameters[i].window    = &window;
		workers   [i] = CreateThread(nullptr, 0, worker_thread, &parameters[i], 0, nullptr);
	}

	last = SDL_GetPerformanceCounter();

	// Game loop
	while (!window.is_closed) {
		window.clear();

		scene.update(delta_time);
		
		remaining = window.tile_count_x * window.tile_count_y;
		for (int i = 0; i < THREAD_COUNT; i++) {
			SetEvent(go_signal[i]);
		}

		WaitForMultipleObjects(THREAD_COUNT, done_signal, true, INFINITE);

		window.update();

		// Perform frame timing
		now = SDL_GetPerformanceCounter();
		delta_time = float(now - last) * inv_perf_freq;
		last = now;

		// Calculate average of last TOTAL_TIMING_COUNT frames
		timings[current_frame++ % TOTAL_TIMING_COUNT] = delta_time;

		float avg = 0.0f;
		int count = current_frame < TOTAL_TIMING_COUNT ? current_frame : TOTAL_TIMING_COUNT;
		for (int i = 0; i < count; i++) {
			avg += timings[i];
		}
		avg /= count;

		// Report timings
		printf("%d - Delta: %f ms, Average: %f ms\n", current_frame, delta_time * 1000.0f, avg * 1000.0f);
	}

	return EXIT_SUCCESS;
}
