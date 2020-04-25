#include <cstdio>
#include <cstdlib>

#include <Imgui/imgui.h>

#include "Raytracer.h"

#include "WorkerThread.h"

// Forces NVIDIA driver to be used 
extern "C" { _declspec(dllexport) unsigned NvOptimusEnablement = true; }

#define TOTAL_TIMING_COUNT 100
float timings[TOTAL_TIMING_COUNT];
int   current_frame = 0;

int main(int argument_count, char ** arguments) {
	Window window(SCREEN_WIDTH, SCREEN_HEIGHT, "Raytracer");

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(glMessageCallback, NULL);
#endif

	Texture::init();
	MaterialBuffer::init();

	// Initialize timing stuff
	Uint64 now  = 0;
	Uint64 last = 0;
	float inv_perf_freq = 1.0f / (float)SDL_GetPerformanceFrequency();
	float delta_time = 0;

	float second = 0.0f;
	int frames = 0;
	int fps    = 0;

	// Initialize Scene
	Scene scene;
	scene.camera.resize(SCREEN_WIDTH, SCREEN_HEIGHT);

	Raytracer raytracer;
	raytracer.scene = &scene;

	// Initialize multi threading stuff
	WorkerThreads::init(raytracer, window);

	last = SDL_GetPerformanceCounter();

	// Game loop
	while (!window.is_closed) {
		//window.clear();

		scene.update(delta_time);
		
		WorkerThreads::wake_up_worker_threads(window.tile_count_x * window.tile_count_y);
		WorkerThreads::wait_on_worker_threads();

		window.draw_quad();

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

		// Calculate fps
		frames++;

		second += delta_time;
		while (second >= 1.0f) {
			second -= 1.0f;

			fps = frames;
			frames = 0;
		}

		PerformanceStats performance_stats = WorkerThreads::sum_performance_stats();

		// Convert to MegaRays / Second
		float num_primary_rays    = float(performance_stats.num_primary_rays    * fps) * 1e-6f;
		float num_shadow_rays     = float(performance_stats.num_shadow_rays     * fps) * 1e-6f;
		float num_reflection_rays = float(performance_stats.num_reflection_rays * fps) * 1e-6f;
		float num_refraction_rays = float(performance_stats.num_refraction_rays * fps) * 1e-6f;

		float num_total_rays = num_primary_rays + num_shadow_rays + num_reflection_rays + num_refraction_rays;
		
		window.gui_begin();

		ImGui::Begin("Raytracer");
		ImGui::Text("Frame: %i", current_frame);
		ImGui::Text("FPS: %i", fps);
		ImGui::Text("Delta: %.2f ms", delta_time * 1000.0f);
		ImGui::Text("Avg:   %.2f ms", avg        * 1000.0f);
		
		if (ImGui::CollapsingHeader("Ray Counters", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Total:      %.2f MRays/s", num_total_rays);
			ImGui::Text("Primary:    %.2f MRays/s", num_primary_rays);
			ImGui::Text("Shadow:     %.2f MRays/s", num_shadow_rays);
			ImGui::Text("Reflection: %.2f MRays/s", num_reflection_rays);
			ImGui::Text("Refraction: %.2f MRays/s", num_refraction_rays);
		}

		ImGui::End();

		window.gui_end();

		window.swap();
	}

	return EXIT_SUCCESS;
}
