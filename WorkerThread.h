#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Scene.h"
#include "Window.h"

#define THREAD_COUNT 8

HANDLE go_signal  [THREAD_COUNT];
HANDLE done_signal[THREAD_COUNT];

volatile LONG remaining;

struct Params {
	int thread_id;
	const Scene  * scene;
	const Window * window;
};

ULONG __stdcall worker_thread(LPVOID parameters) {
	Params params = *reinterpret_cast<Params *>(parameters);

	while (true) {
		// Wait until the Worker Thread is signalled by the main thread
		WaitForSingleObject(go_signal[params.thread_id], INFINITE);

		while (remaining > 0) { 
			int task = (int)InterlockedDecrement(&remaining) - 1; 
			
			if (task >= 0) { 
				int x = task % params.window->tile_count_x;
				int y = task / params.window->tile_count_x; 
				
				params.scene->render_tile(*params.window, x * params.window->tile_width, y * params.window->tile_height);
			} 
		}
		
		// Signal the main thread that we are done
		SetEvent(done_signal[params.thread_id]);
	}
}
