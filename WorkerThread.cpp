#include "WorkerThread.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define USE_MULTITHREADING true

int thread_count; 

HANDLE * go_signal;
HANDLE * done_signal;

volatile LONG remaining;

struct Params {
	int thread_id;
	const Scene  * scene;
	const Window * window;
} * parameters;

// This is the actual function that will run on each Worker Thread
// It will wait until work becomes available, execute it and notify when the work is done
ULONG WINAPI worker_thread(LPVOID parameters) {
	Params params = *reinterpret_cast<Params *>(parameters);

	HANDLE thread = GetCurrentThread(); 

	WCHAR thread_name[32];
	wsprintfW(thread_name, L"WorkerThread_%d", params.thread_id);
	SetThreadDescription(thread, thread_name);
	
	// Set the Thread Affinity to pin it to 1 logical core
#if USE_MULTITHREADING
	DWORD_PTR thread_affinity_mask     = 1 << params.thread_id;
	DWORD_PTR thread_affinity_mask_old = SetThreadAffinityMask(thread, thread_affinity_mask);

	// Check validity of Thread Affinity
	if ((thread_affinity_mask & thread_affinity_mask_old) == 0) {
		printf("Unable to set Process Affinity Mask!\n");

		abort();
	}
#endif

	while (true) {
		// Wait until the Worker Thread is signalled by the main thread
		WaitForSingleObject(go_signal[params.thread_id], INFINITE);

		while (remaining > 0) { 
			int task = (int)InterlockedDecrement(&remaining); 
			
			if (task >= 0) { 
				int x = task % params.window->tile_count_x;
				int y = task / params.window->tile_count_x; 
				
				params.scene->render_tile(*params.window, x * params.window->tile_width, y * params.window->tile_height);

				//printf("Task %i done\n", task);
			} 
		}
		
		// Signal the main thread that we are done
		SetEvent(done_signal[params.thread_id]);
	}
}

void WorkerThreads::init(const Scene & scene, const Window & window) {
#if USE_MULTITHREADING
	thread_count = 0;

	SYSTEM_LOGICAL_PROCESSOR_INFORMATION info[64];
	DWORD buffer_length = 64 * sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
	GetLogicalProcessorInformation(info, &buffer_length);
	
	// Count the number of physical cores
	for (int i = 0; i < buffer_length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); i++) {
		if (info[i].Relationship == LOGICAL_PROCESSOR_RELATIONSHIP::RelationProcessorCore) {
			for (int j = 0; j < 32; j++) {
				if (info[i].ProcessorMask >> j & 1) {
					thread_count++;
				}
			}
		}
	}
#else
	thread_count = 1;
#endif

	go_signal   = new HANDLE[thread_count];
	done_signal = new HANDLE[thread_count];

	parameters = new Params[thread_count];

	// Spawn the appropriate number of Worker Threads.
	for (int i = 0; i < thread_count; i++) {
		go_signal  [i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		done_signal[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		parameters[i].thread_id = i;
		parameters[i].scene     = &scene;
		parameters[i].window    = &window;
		
		CreateThread(nullptr, 0, worker_thread, &parameters[i], 0, nullptr);
	}
}

void WorkerThreads::wake_up_worker_threads(int job_count) {
	remaining = job_count;

	for (int i = 0; i < thread_count; i++) {
		SetEvent(go_signal[i]);
	}
}

void WorkerThreads::wait_on_worker_threads() {
	WaitForMultipleObjects(thread_count, done_signal, true, INFINITE);
}
