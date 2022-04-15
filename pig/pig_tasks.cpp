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
		// |                   PigTask_ReadFileContents                   |
		// +--------------------------------------------------------------+
		case PigTask_ReadFileContents:
		{
			NotNullStr(&task->input.inputStr);
			Assert(task->input.inputSize1 == sizeof(PlatFileContents_t));
			NotNull(task->input.inputPntr1);
			PlatFileContents_t* fileOut = (PlatFileContents_t*)task->input.inputPntr1;
			task->result.success = plat->ReadFileContents(task->input.inputStr, fileOut);
			task->result.resultSize1 = sizeof(PlatFileContents_t);
			task->result.resultPntr1 = fileOut;
		} break;
		
		// +--------------------------------------------------------------+
		// |                   PigTask_WriteEntireFile                    |
		// +--------------------------------------------------------------+
		case PigTask_WriteEntireFile:
		{
			NotNullStr(&task->input.inputStr);
			task->result.success = plat->WriteEntireFile(task->input.inputStr, task->input.inputPntr1, task->input.inputSize1);
		} break;
		
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
		// |              PigTask_ReadFileContents Completed              |
		// +--------------------------------------------------------------+
		case PigTask_ReadFileContents:
		{
			Assert(task->result.resultSize1 == sizeof(PlatFileContents_t));
			NotNull(task->result.resultPntr1);
			PlatFileContents_t* filePntr = (PlatFileContents_t*)task->result.resultPntr1;
			NotNull(task->input.callbackFunc);
			ReadFileContentsCallback_f* callback = (ReadFileContentsCallback_f*)task->input.callbackFunc;
			callback(task->result.success, task->input.inputStr, filePntr, task->input.callbackContext);
		} break;
		
		// +--------------------------------------------------------------+
		// |              PigTask_WriteEntireFile Completed               |
		// +--------------------------------------------------------------+
		case PigTask_WriteEntireFile:
		{
			if (task->input.callbackFunc != nullptr)
			{
				WriteEntireFileCallback_f* callback = (WriteEntireFileCallback_f*)task->input.callbackFunc;
				callback(task->result.success, task->input.inputStr, task->input.inputSize1, task->input.inputPntr1, task->input.callbackContext);
			}
		} break;
		
		// +--------------------------------------------------------------+
		// |                        Unimplemented                         |
		// +--------------------------------------------------------------+
		default:
		{
			AssertMsg(false, "Unimplemented task type in PigHandleCompletedTask");
		}
	}
	
	if (task->input.contextPntr != nullptr && task->input.contextArena != nullptr)
	{
		FreeMem(task->input.contextArena, task->input.contextPntr, task->input.contextSize);
	}
	if (task->input.inputStr.pntr != nullptr && task->input.inputStrArena != nullptr)
	{
		FreeString(task->input.inputStrArena, &task->input.inputStr);
	}
	if (task->input.inputPntr1 != nullptr && task->input.inputArena1 != nullptr)
	{
		FreeMem(task->input.inputArena1, task->input.inputPntr1, task->input.inputSize1);
	}
	if (task->input.inputPntr2 != nullptr && task->input.inputArena2 != nullptr)
	{
		FreeMem(task->input.inputArena2, task->input.inputPntr2, task->input.inputSize2);
	}
	if (task->input.inputPntr3 != nullptr && task->input.inputArena3 != nullptr)
	{
		FreeMem(task->input.inputArena3, task->input.inputPntr3, task->input.inputSize3);
	}
}

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
void PigAsyncReadFileContents(MyStr_t filePath, ReadFileContentsCallback_f* callbackFunc, void* callbackContext = nullptr)
{
	NotNullStr(&filePath);
	NotNull(callbackFunc);
	
	PlatTaskInput_t taskInput = {};
	taskInput.type = PigTask_ReadFileContents;
	
	taskInput.inputStr = AllocString(mainHeap, &filePath);
	taskInput.inputStrArena = mainHeap;
	
	taskInput.inputSize1 = sizeof(PlatFileContents_t);
	taskInput.inputPntr1 = AllocStruct(mainHeap, PlatFileContents_t);
	taskInput.inputArena1 = mainHeap;
	
	taskInput.callbackFunc = (void*)callbackFunc;
	taskInput.callbackContext = callbackContext;
	
	if (plat->QueueTask(&taskInput) == nullptr)
	{
		callbackFunc(false, filePath, nullptr, callbackContext);
	}
}

//contentsArena can be nullptr if the contentsPntr is going to be deallocated by the callbackFunc
void PigAsyncWriteEntireFile(MyStr_t filePath, u64 contentsSize, void* contentsPntr, MemArena_t* contentsArena, WriteEntireFileCallback_f* callbackFunc = nullptr, void* callbackContext = nullptr)
{
	NotNullStr(&filePath);
	Assert(contentsSize > 0);
	NotNull(contentsPntr);
	//If you aren't handing off responsibility for deallocation then you must have a callback to deallocate this memory
	AssertIf(contentsArena == nullptr, callbackFunc != nullptr);
	
	PlatTaskInput_t taskInput = {};
	taskInput.type = PigTask_WriteEntireFile;
	
	taskInput.inputStr = AllocString(mainHeap, &filePath);
	taskInput.inputStrArena = mainHeap;
	
	taskInput.inputSize1 = contentsSize;
	taskInput.inputPntr1 = contentsPntr;
	taskInput.inputArena1 = contentsArena;
	
	taskInput.callbackFunc = (void*)callbackFunc;
	taskInput.callbackContext = callbackContext;
	
	if (plat->QueueTask(&taskInput) == nullptr)
	{
		callbackFunc(false, filePath, contentsSize, contentsPntr, callbackContext);
	}
}

// This is a nice little wrapper around PigAsyncWriteEntireFile that will copy the contentsPntr into a new buffer allocated from contentsArena.
// The cleanup of this allocation will be handled automatically by the completed task cleanup
void PigAsyncWriteEntireFileInArena(MyStr_t filePath, u64 contentsSize, const void* contentsPntr, MemArena_t* contentsArena, WriteEntireFileCallback_f* callbackFunc = nullptr, void* callbackContext = nullptr)
{
	Assert(contentsSize > 0);
	NotNull(contentsPntr);
	NotNull(contentsArena);
	void* actualContents = AllocMem(contentsArena, contentsSize);
	NotNull(actualContents);
	MyMemCopy(actualContents, contentsPntr, contentsSize);
	PigAsyncWriteEntireFile(filePath, contentsSize, actualContents, contentsArena, callbackFunc, callbackContext);
}
