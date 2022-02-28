/*
File:   pig_tasks.cpp
Author: Taylor Robbins
Date:   02\28\2022
Description: 
	** Holds the implementation details for all pig engine task types that can be
	** performed on the separate threads using the engine provided thread pool.
	** Engine task types are designated by task enum numbers below GameTask_Base and are things
	** that all games or applications may want to do like loading a file generically on a separate thread
*/

//NOTE: This function runs on a thread pool thread
void PigHandleTask(const PlatformInfo_t* info, const PlatformApi_t* api, PlatThreadPoolThread_t* thread, PlatTask_t* task)
{
	NotNull(info);
	NotNull(api);
	NotNull(thread);
	NotNull(task);
	switch (task->input.type)
	{
		// +--------------------------------------------------------------+
		// |                         PigTask_None                         |
		// +--------------------------------------------------------------+
		// case PigTask_None:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +--------------------------------------------------------------+
		// |                        Unimplemented                         |
		// +--------------------------------------------------------------+
		default:
		{
			AssertMsg(false, "Unimplemented task type in PigHandleTask");
		}
	}
}

//NOTE: This function runs on the main thread
void PigHandleCompletedTask(PlatTask_t* task)
{
	NotNull(task);
	switch (task->input.type)
	{
		// +--------------------------------------------------------------+
		// |                         PigTask_None                         |
		// +--------------------------------------------------------------+
		// case PigTask_None:
		// {
		// 	Unimplemented(); //TODO: Implement me!
		// } break;
		
		// +--------------------------------------------------------------+
		// |                        Unimplemented                         |
		// +--------------------------------------------------------------+
		default:
		{
			AssertMsg(false, "Unimplemented task type in PigHandleCompletedTask");
		}
	}
}

