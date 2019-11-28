#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Scene.h"
#include "Window.h"

#define THREAD_COUNT 8

// Threading code is based on code presented in INFOMOV slides
namespace WorkerThreads {
	inline HANDLE go_signal  [THREAD_COUNT];
	inline HANDLE done_signal[THREAD_COUNT];

	inline volatile LONG remaining;

	struct Params {
		int thread_id;
		const Scene  * scene;
		const Window * window;
	};

	void init_core_info();

	ULONG __stdcall worker_thread(LPVOID parameters);
}
