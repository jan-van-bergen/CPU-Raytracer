#pragma once

#include "Scene.h"
#include "Window.h"

// Threading code is based on code presented in INFOMOV slides
namespace WorkerThreads {
	// Initializes WorkerThreads, should be called only once!
	void init(const Scene & scene, const Window & window);

	void wake_up_worker_threads(int job_count);
	void wait_on_worker_threads();
}
