/*
File:   win32_procmon.cpp
Author: Taylor Robbins
Date:   07\28\2022
Description: 
	** Procmon (Process Monitor) is a driver built from OpenProcmon which provides
	** us the ability to monitor the file and registry accesses other programs are performing
*/

#if PROCMON_SUPPORTED

BOOL HandleProcmonEvent(const CRefPtr<CEventView> pEventView)
{
	Platform->procmonThreadId = Win32_GetThisThreadId();
	MemArena_t* scratch = GetScratchArena();
	if (scratch == nullptr)
	{
		InitThreadLocalScratchArenasVirtual(MAIN_SCRATCH_ARENA_MAX_SIZE, MAIN_SCRATCH_ARENA_MAX_NUM_MARKS);
		scratch = GetScratchArena();
		NotNull(scratch);
	}
	
	// ULONGLONG Time = pEventView->GetStartTime().QuadPart;
	
	// PrintLine_D("PROCMON: %llu Process %s Do 0x%x for %s",
	// 	Time,
	// 	pEventView->GetProcessName().GetBuffer(),
	// 	pEventView->GetEventOperator(),
	// 	pEventView->GetPath().GetBuffer()
	// );
	//m_viewList.push_back(pEventView);
	
	DWORD eventClass = pEventView->GetEventClass();
	if (eventClass == MONITOR_TYPE_PROCESS) { /* TODO: Do something with these events? */ }
	else if (eventClass == MONITOR_TYPE_FILE)
	{
		// LPCTSTR operationStr = StrMapOperation(pEventView->GetPreEventEntry());
		
		#if 0
		{
			MemArena_t conversionBufferArena;
			InitMemArena_Buffer(&conversionBufferArena, sizeof(Platform->procmonConversionBuffer), &Platform->procmonConversionBuffer[0]);
			MyStr_t rawProcessName = ConvertUcs2StrToUtf8(&conversionBufferArena, pEventView->GetProcessName().GetBuffer(), MyWideStrLength(pEventView->GetProcessName().GetBuffer()));
			
			ProcmonEntry_t* entry = StrHashDictGetSoft(&Platform->processEntries, rawProcessName, ProcmonEntry_t);
			if (entry == nullptr)
			{
				entry = StrHashDictAdd(&Platform->processEntries, rawProcessName, ProcmonEntry_t);
				entry->id = Platform->nextProcmonEntryId;
				Platform->nextProcmonEntryId++;
				entry->processName = AllocString(&Platform->procmonHeap, &rawProcessName);
				entry->numEvents = 1;
				// ClearArray(entry->lastFewEvents);
				// entry->lastFewEvents[0] = eventType;
				// entry->eventIndex = 1;
				entry->eventBits = eventBit;
			}
			else
			{
				entry->numEvents++;
				// entry->lastFewEvents[entry->eventIndex] = eventType;
				// entry->eventIndex = (entry->eventIndex + 1) % ArrayCount(entry->lastFewEvents);
				entry->eventBits |= eventBit;
			}
		}s
		#endif
		
		if (InitPhase >= Win32InitPhase_PostFirstUpdate && InitPhase < Win32InitPhase_Closing)
		{
			ProcmonEvent_t event = {};
			
			event.id = Platform->nextProcmonEventId;
			Platform->nextProcmonEventId++;
			
			event.type = (ProcmonEventType_t)(pEventView->GetEventOperator() - 20);
			
			CString eventViewPath = pEventView->GetPath();
			event.path = ConvertUcs2StrToUtf8(scratch, eventViewPath.GetBuffer(), eventViewPath.GetLength());
			CString eventViewDetail = pEventView->GetDetail();
			event.detail = ConvertUcs2StrToUtf8(scratch, eventViewDetail.GetBuffer(), eventViewDetail.GetLength());
			
			event.processId = (u64)pEventView->GetProcessId();
			CString eventViewProcessName = pEventView->GetProcessName();
			event.processName = ConvertUcs2StrToUtf8(scratch, eventViewProcessName.GetBuffer(), eventViewProcessName.GetLength());
			CString eventViewImagePath = pEventView->GetImagePath();
			event.processImagePath = ConvertUcs2StrToUtf8(scratch, eventViewImagePath.GetBuffer(), eventViewImagePath.GetLength());
			
			Platform->engine.HandleProcmonEvent(&Platform->info, &Platform->api, &Platform->engineMemory, &event, &Platform->procmonTempArena);
		}
	}
	else { MyDebugBreak(); }
	
	FreeScratchArena(scratch);
	return TRUE;
}

class CMyEvent : public IEventCallback
{
public:
	virtual BOOL DoEvent(const CRefPtr<CEventView> pEventView)
	{
		return HandleProcmonEvent(pEventView);
	}
};

void Win32_ProcmonInit(u64 tempArenaSize, u64 tempArenaMarkCount)
{
	bool wasRunInAdministratorMode = Win32_WasProgramRunInAdministratorMode();
	
	void* tempArenaSpace = malloc(tempArenaSize); //TODO: Should we allocate this using a windows specific call?
	NotNull(tempArenaSpace);
	InitMemArena_MarkedStack(&Platform->procmonTempArena, tempArenaSize, tempArenaSpace, tempArenaMarkCount);
	
	if (wasRunInAdministratorMode)
	{
		Platform->Optmgr     = &Singleton<CEventMgr>::getInstance();
		Platform->Monitormgr = &Singleton<CMonitorContoller>::getInstance();
		Platform->Drvload    = &Singleton<CDrvLoader>::getInstance();
		
		if (!Platform->Drvload->Init(TEXT("PROCMON24"), TEXT("procmon.sys")))
		{
			Win32_InitError("Failed to load procmon.sys (open process monitor driver)");
		}
		
		Platform->Optmgr->RegisterCallback(new CMyEvent);
		
		if (!Platform->Monitormgr->Connect())
		{
			Win32_InitError("Failed to Connect to procmon driver (open process monitor driver).\n\nMake sure you aren't running multiple instances of the program at the same time.");
		}
		
		Platform->Monitormgr->SetMonitor(TRUE, TRUE, FALSE);
		if (!Platform->Monitormgr->Start())
		{
			Win32_InitError("Failed to start the procmon monitor (open process monitor driver).\n\nMake sure you aren't running multiple instances of the program at the same time.");
		}
	}
	else
	{
		WriteLine_E("Failed to start Procmon Driver because the application was not started with Administrator privelages");
		// Win32_InitError("This application needs administrator privelages so it can start a system driver that monitors file access events.\n\nPlease restart this program in Administrator Mode");
	}
}

void Win32_ProcmonShutdown()
{
	if (Platform->info.wasRunInAdministratorMode)
	{
		Platform->Monitormgr->Stop();
		Platform->Monitormgr->Destory();
	}
}

#endif //PROCMON_SUPPORTED
