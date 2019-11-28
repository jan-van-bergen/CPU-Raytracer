#include "WorkerThread.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace WorkerThreads;

#define THREAD_COUNT 1

int threads_per_processor;

int   processor_count = 0;	         // Number of Physical Processors
ULONG processor_masks[THREAD_COUNT]; // Stores the mask of each Physical Processor

HANDLE go_signal  [THREAD_COUNT];
HANDLE done_signal[THREAD_COUNT];

volatile LONG remaining;

struct Params {
	int thread_id;
	const Scene  * scene;
	const Window * window;
} parameters[THREAD_COUNT];

// This is the actual function that will run on each Worker Thread
// It will wait until work becomes available, execute it and notify when the work is done
ULONG WINAPI worker_thread(LPVOID parameters) {
	Params params = *reinterpret_cast<Params *>(parameters);

	HANDLE thread = GetCurrentThread(); 

	WCHAR thread_name[32];
	wsprintfW(thread_name, L"WorkerThread_%d", params.thread_id);
	SetThreadDescription(thread, thread_name);
	
	// Set the Thread Affinity to two logical cores that belong to the same physical core
	if (THREAD_COUNT > 1) {
		DWORD_PTR thread_affinity_mask     = processor_masks[params.thread_id / threads_per_processor];
		DWORD_PTR thread_affinity_mask_old = SetThreadAffinityMask(thread, thread_affinity_mask);

		// Check validity of Thread Affinity
		if ((thread_affinity_mask & thread_affinity_mask_old) == 0) {
			printf("Unable to set Process Affinity Mask!\n");

			abort();
		}
	}

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
	// Get logical and physical core count on this machine
	SYSTEM_LOGICAL_PROCESSOR_INFORMATION info[64];
	DWORD buffer_length = 64 * sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
	GetLogicalProcessorInformation(info, &buffer_length);
	
	// Count the number of physical cores and store their thread masks
	for (int i = 0; i < buffer_length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); i++) {
		if (info[i].Relationship == LOGICAL_PROCESSOR_RELATIONSHIP::RelationProcessorCore) {
			processor_masks[processor_count++] = info[i].ProcessorMask;
		}
	}

	threads_per_processor = THREAD_COUNT / processor_count;

	// Spawn the appropriate number of Worker Threads.
	for (int i = 0; i < THREAD_COUNT; i++) {
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

	for (int i = 0; i < THREAD_COUNT; i++) {
		SetEvent(go_signal[i]);
	}
}

void WorkerThreads::wait_on_worker_threads() {
	WaitForMultipleObjects(THREAD_COUNT, done_signal, true, INFINITE);
}
