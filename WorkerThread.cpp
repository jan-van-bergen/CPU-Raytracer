#include "WorkerThread.h"

using namespace WorkerThreads;

int threads_per_processor;

int   processor_count = 0;	         // Number of Physical Processors
ULONG processor_masks[THREAD_COUNT]; // Stores the mask of each Physical Processor

void WorkerThreads::init_core_info() {
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
}

ULONG __stdcall WorkerThreads::worker_thread(LPVOID parameters) {
	Params params = *reinterpret_cast<Params *>(parameters);

	// Set the Thread Affinity to two logical cores that belong to the same physical core
	HANDLE thread = GetCurrentThread(); 

	WCHAR thread_name[32];
	wsprintfW(thread_name, L"WorkerThread_%d", params.thread_id);
	SetThreadDescription(thread, thread_name);

	DWORD_PTR thread_affinity_mask     = processor_masks[params.thread_id / threads_per_processor];
	DWORD_PTR thread_affinity_mask_old = SetThreadAffinityMask(thread, thread_affinity_mask);

	// Check validity of Thread Affinity
    if ((thread_affinity_mask & thread_affinity_mask_old) == 0) {
		printf("Unable to set Process Affinity Mask!\n");

		abort();
	}

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
