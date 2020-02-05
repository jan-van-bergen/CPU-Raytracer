#pragma once

#include "Raytracer.h"

// Threading code is based on code presented in INFOMOV slides
namespace WorkerThreads {
	// Initializes WorkerThreads, should be called only once!
	void init(const Raytracer & raytracer, const Window & window);

	void wake_up_worker_threads(int job_count);
	void wait_on_worker_threads();
}
