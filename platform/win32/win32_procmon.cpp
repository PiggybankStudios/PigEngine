/*
File:   win32_procmon.cpp
Author: Taylor Robbins
Date:   07\28\2022
Description: 
	** Procmon (Process Monitor) is a driver built from OpenProcmon which provides
	** us the ability to monitor the file and registry accesses other programs are performing
*/

#if PROCMON_SUPPORTED

class CMyEvent : public IEventCallback
{
public:
	virtual BOOL DoEvent(const CRefPtr<CEventView> pEventView)
	{
		ULONGLONG Time = pEventView->GetStartTime().QuadPart;
		
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
			DWORD eventOperator = pEventView->GetEventOperator();
			ProcmonEventType_t eventType = (ProcmonEventType_t)eventOperator;
			ProcmonEventBit_t eventBit = GetProcmonEventBit(eventType);
			
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
			
			CString eventViewPath = pEventView->GetPath();
			if (eventViewPath.GetLength() > 0)
			{
				InitMemArena_Buffer(&conversionBufferArena, sizeof(Platform->procmonConversionBuffer), &Platform->procmonConversionBuffer[0]);
				MyStr_t rawPath = ConvertUcs2StrToUtf8(&conversionBufferArena, eventViewPath.GetBuffer(), eventViewPath.GetLength());
				
				if (FindSubstring(rawPath, ".cpp")) //TODO: Remove this check!
				{
					ProcmonFile_t* touchedFile = StrHashDictGetSoft(&Platform->touchedFiles, rawPath, ProcmonFile_t);
					if (touchedFile == nullptr)
					{
						touchedFile = StrHashDictAdd(&Platform->touchedFiles, rawPath, ProcmonFile_t);
						touchedFile->id = Platform->nextProcmonFileId;
						Platform->nextProcmonFileId++;
						touchedFile->filePath = AllocString(&Platform->procmonHeap, &rawPath);
						touchedFile->numTouches = 0;
					}
					touchedFile->processId = entry->id;
					touchedFile->numTouches++;
				}
			}
			else
			{
				// MyDebugBreak();
			}
			
			Platform->nextProcmonEventId++;
		}
		else { MyDebugBreak(); }
		
		return TRUE;
	}
};


void Win32_ProcmonInit()
{
	CreateStrHashDict(&Platform->processEntries, &Platform->procmonHeap, sizeof(ProcmonEntry_t), 512);
	CreateStrHashDict(&Platform->touchedFiles, &Platform->procmonHeap, sizeof(ProcmonFile_t), 512);
	
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
		Win32_InitError("Failed to Connect to procmon driver (open process monitor driver)");
	}
	
	Platform->Monitormgr->SetMonitor(TRUE, TRUE, FALSE);
	if (!Platform->Monitormgr->Start())
	{
		Win32_InitError("Failed to start to procmon monitor (open process monitor driver)");
	}
}

#endif //PROCMON_SUPPORTED
