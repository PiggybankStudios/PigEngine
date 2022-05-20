/*
File:   win32_audio.cpp
Author: Taylor Robbins
Date:   01\13\2022
Description: 
	** Holds the functions that help us initialize and manage our audio output on windows
	** This file currently uses the Win32 Core Audio APIs to do audio output
*/

//TODO: Maybe we want to #define COBJMACROS and CINTERFACE
//Allan's implemntation of WASAPI https://github.com/4th-dimention/app_template/blob/master/source/win32/win32_wasapi.c

THREAD_FUNCTION_DEF(Win32_AudioThreadFunc, userPntr);

// +--------------------------------------------------------------+
// |                           Cleanup                            |
// +--------------------------------------------------------------+
void Win32_ClearAudioDevices()
{
	VarArrayLoop(&Platform->audioDevices, dIndex)
	{
		VarArrayLoopGet(PlatAudioDevice_t, device, &Platform->audioDevices, dIndex);
		NotNull(device->allocArena);
		FreeString(device->allocArena, &device->name);
		FreeString(device->allocArena, &device->deviceId);
	}
	VarArrayClear(&Platform->audioDevices);
}

PlatAudioDevice_t* Win32_GetAudioDeviceByIdStr(MyStr_t idStr)
{
	NotNullStr(&idStr);
	VarArrayLoop(&Platform->audioDevices, dIndex)
	{
		VarArrayLoopGet(PlatAudioDevice_t, audioDevice, &Platform->audioDevices, dIndex);
		if (StrEquals(audioDevice->deviceId, idStr))
		{
			return audioDevice;
		}
	}
	return nullptr;
}

void Win32_EnumerateAudioDevices()
{
	bool isInitializing = (InitPhase < Win32InitPhase_AudioInitialized);
	if (isInitializing)
	{
		CreateVarArray(&Platform->audioDevices, &Platform->mainHeap, sizeof(PlatAudioDevice_t));
	}
	Win32_ClearAudioDevices();
	
	IMMDeviceCollection* deviceCollection = nullptr;
	HRESULT enumEndpointsResult = Platform->audioDeviceEnumerator->EnumAudioEndpoints(
		eRender,              //dataFlow
		// DEVICE_STATEMASK_ALL, //dwStateMask
		DEVICE_STATE_ACTIVE, //dwStateMask
		&deviceCollection     //ppDevices
	);
	if (enumEndpointsResult != S_OK)
	{
		if (isInitializing)
		{
			Win32_InitError("EnumAudioEndpoints failed!");
		}
		else
		{
			WriteLine_E("Failed to start enumerating audio devices! EnumAudioEndpoints failed!");
		}
	}
	
	UINT numDevices = 0;
	HRESULT getCountResult = deviceCollection->GetCount(&numDevices);
	if (getCountResult != S_OK)
	{
		if (isInitializing)
		{
			Win32_InitError("Audio Device Collection GetCount failed!");
		}
		else
		{
			WriteLine_E("Failed to get audio device count! GetCount failed!");
		}
	}
	
	IMMDevice* defaultDevicePntr = nullptr;
	HRESULT getDefaultDevice = Platform->audioDeviceEnumerator->GetDefaultAudioEndpoint(
		eRender,           //dataFlow,
		eConsole,          //role,
		&defaultDevicePntr //ppEndpoint
	);
	Assert(getDefaultDevice == S_OK);
	
	WCHAR* defaultDeviceIdWide = nullptr;
	HRESULT getDefaultIdResult = defaultDevicePntr->GetId(&defaultDeviceIdWide);
	Assert(getDefaultIdResult == S_OK);
	NotNull(defaultDeviceIdWide);
	MyStr_t defaultDeviceId = ConvertUcs2StrToUtf8Nt(TempArena, defaultDeviceIdWide);
	CoTaskMemFree(defaultDeviceIdWide);
	
	bool foundDefaultDeviceIndex = false;
	Platform->defaultAudioDeviceIndex = 0;
	for (u64 dIndex = 0; dIndex < numDevices; dIndex++)
	{
		IMMDevice* devicePntr = nullptr;
		HRESULT getItemResult = deviceCollection->Item((UINT)dIndex, &devicePntr);
		if (getItemResult == S_OK)
		{
			WCHAR* deviceIdWide = nullptr;
			HRESULT getIdResult = devicePntr->GetId(&deviceIdWide);
			Assert(getIdResult == S_OK);
			NotNull(deviceIdWide);
			
			// DWORD deviceState = 0;
			// HRESULT getStateResult = devicePntr->GetState(&deviceState);
			// Assert(getStateResult == S_OK);
			
			IPropertyStore* propertiesStore = nullptr;
			HRESULT getPropStore = devicePntr->OpenPropertyStore(STGM_READ, &propertiesStore);
			Assert(getPropStore == S_OK);
			
			PROPVARIANT deviceNameProperty = {};
			HRESULT getNameResult = propertiesStore->GetValue(PKEY_Device_FriendlyName, &deviceNameProperty);
			Assert(getNameResult == S_OK);
			WCHAR* deviceNameWide = deviceNameProperty.pwszVal;
			NotNull(deviceNameWide);
			
			PlatAudioDevice_t* newDevice = VarArrayAdd(&Platform->audioDevices, PlatAudioDevice_t);
			NotNull(newDevice);
			ClearPointer(newDevice);
			newDevice->allocArena = &Platform->mainHeap;
			newDevice->isDefaultDevice = false;
			newDevice->name = ConvertUcs2StrToUtf8Nt(newDevice->allocArena, deviceNameWide);
			newDevice->deviceId = ConvertUcs2StrToUtf8Nt(newDevice->allocArena, deviceIdWide);
			if (StrCompareIgnoreCase(defaultDeviceId, newDevice->deviceId) == 0)
			{
				Assert(!foundDefaultDeviceIndex);
				Platform->defaultAudioDeviceIndex = dIndex;
				newDevice->isDefaultDevice = true;
				foundDefaultDeviceIndex = true;
			}
			newDevice->devicePntr = devicePntr;
			
			PrintLine_D("Device[%llu]: \"%.*s\"%s \"%.*s\"", dIndex, newDevice->name.length, newDevice->name.pntr, newDevice->isDefaultDevice ? " (Default)" : "", newDevice->deviceId.length, newDevice->deviceId.pntr);
			
			CoTaskMemFree(deviceIdWide);
			PropVariantClear(&deviceNameProperty);
			propertiesStore->Release();
		}
		else
		{
			PrintLine_E("Failed to get device[%llu] from device collection", dIndex);
		}
	}
	Assert(foundDefaultDeviceIndex);
	defaultDevicePntr->Release();
	
	PrintLine_D("Found %llu device(s)", Platform->audioDevices.length);
	
	deviceCollection->Release();
}

// +--------------------------------------------------------------+
// |                        Initialization                        |
// +--------------------------------------------------------------+
void Win32_AudioInit()
{
	HRESULT coInitResult = CoInitialize(NULL);
	if (coInitResult != S_OK &&
		coInitResult != S_FALSE && //already initialized on this thread
		coInitResult != RPC_E_CHANGED_MODE) //Something to do with concurrency model changing from previous call to CoInitialize
	{
		Win32_InitError("CoInitialize failed!");
	}
	
	HRESULT coCreateResult = CoCreateInstance(
		__uuidof(MMDeviceEnumerator),            //rclsid
		NULL,                                    //pUnkOuter
		CLSCTX_ALL,                              //dwClsContext
		__uuidof(IMMDeviceEnumerator),           //riid
		(void**)&Platform->audioDeviceEnumerator //ppv
	);
	if (coCreateResult != S_OK)
	{
		Win32_InitError("Failed to call CoCreateInstance for audio initialization");
	}
	
	Win32_CreateMutex(&Platform->audioOutputMutex);
	
	Win32_EnumerateAudioDevices();
	
	InPlaceNew(AudioCallbackClass_c, &Platform->audioDeviceCallback);
	HRESULT registerCallbackResult = Platform->audioDeviceEnumerator->RegisterEndpointNotificationCallback(&Platform->audioDeviceCallback);
	if (registerCallbackResult != S_OK)
	{
		Win32_InitError("RegisterEndpointNotificationCallback failed!");
	}
}

// +--------------------------------------------------------------+
// |                           Helpers                            |
// +--------------------------------------------------------------+
void Win32_ServiceAudioWithSineWave(AudioServiceInfo_t* serviceInfo)
{
	for (u32 sIndex = 0; sIndex < serviceInfo->numFramesNeeded; sIndex++)
	{
		u8* framePntr = &serviceInfo->bufferPntr[sIndex * serviceInfo->format.numChannels * (serviceInfo->format.bitsPerSample/8)];
		r64 sampleTime = (r64)((serviceInfo->audioFrameIndex + sIndex) % serviceInfo->format.samplesPerSecond) / (r64)serviceInfo->format.samplesPerSecond;
		const r64 volume = 0.2;
		if (serviceInfo->format.bitsPerSample == 32)
		{
			i32 sampleValue = (i32)RoundR64i(SinR64(sampleTime * TwoPi64 * FREQUENCY_MIDDLE_C) * (r64)INT32_MAX * volume);
			if (serviceInfo->format.numChannels == 2)
			{
				i32* sampleLeft  = (i32*)&framePntr[sizeof(i32) * 0];
				i32* sampleRight = (i32*)&framePntr[sizeof(i32) * 1];
				*sampleLeft = sampleValue;
				*sampleRight = sampleValue;
			}
			else if (serviceInfo->format.numChannels == 1)
			{
				i32* sample = (i32*)framePntr;
				*sample = sampleValue;
			}
		}
		else if (serviceInfo->format.bitsPerSample == 16)
		{
			i16 sampleValue = (i16)RoundR32i(SinR32((r32)sampleTime * TwoPi32 * 264.0f) * (r32)INT16_MAX * (r32)volume);
			if (serviceInfo->format.numChannels == 2)
			{
				i16* sampleLeft  = (i16*)&framePntr[sizeof(i16) * 0];
				i16* sampleRight = (i16*)&framePntr[sizeof(i16) * 1];
				*sampleLeft = sampleValue;
				*sampleRight = sampleValue;
			}
			else if (serviceInfo->format.numChannels == 1)
			{
				i16* sample = (i16*)framePntr;
				*sample = sampleValue;
			}
		}
	}
	serviceInfo->numFramesFilled = serviceInfo->numFramesNeeded;
}

void Win32_FillWaveFormatEx(const PlatAudioFormat_t* format, WAVEFORMATEX* formatOut)
{
	NotNull(format);
	NotNull(formatOut);
	Assert(format->numChannels == 1 || format->numChannels == 2);
	Assert(format->bitsPerSample == 16 || format->bitsPerSample == 32);
	Assert(format->samplesPerSecond > 0); //TODO: Can we put more restrictions on this?
	ClearPointer(formatOut);
	formatOut->wFormatTag      = WAVE_FORMAT_PCM;
	formatOut->nChannels       = (WORD)format->numChannels;
	formatOut->nSamplesPerSec  = (DWORD)(format->samplesPerSecond);
	formatOut->nBlockAlign     = (WORD)((format->numChannels * format->bitsPerSample) / 8);
	formatOut->nAvgBytesPerSec = (DWORD)(formatOut->nBlockAlign * format->samplesPerSecond);
	formatOut->wBitsPerSample  = (WORD)format->bitsPerSample;
	formatOut->cbSize = 0;
}

//TODO: Does this function need to support changing devices after the program has already started outputting on a device?
void Win32_StartAudioOutput(u64 deviceIndex, const PlatAudioFormat_t* format)
{
	PlatAudioDevice_t* audioDevice = VarArrayGet(&Platform->audioDevices, deviceIndex, PlatAudioDevice_t);
	NotNull(audioDevice);
	
	HRESULT activateResult = audioDevice->devicePntr->Activate(
		__uuidof(IAudioClient),        //iid
		CLSCTX_ALL,                    //dwClsCtx
		NULL,                          //pActivationParams
		(void**)&Platform->audioClient //ppInterface
	);
	Assert(activateResult == S_OK);
	Assert(Platform->audioClient != nullptr);
	
	WAVEFORMATEX* deviceAudioFormat = nullptr;
	HRESULT getFormatResult = Platform->audioClient->GetMixFormat(&deviceAudioFormat);
	Assert(getFormatResult == S_OK);
	
	WriteLine_D("Audio device format:");
	PrintLine_D("  wFormatTag:      %u", deviceAudioFormat->wFormatTag);
	PrintLine_D("  nChannels:       %u", deviceAudioFormat->nChannels);
	PrintLine_D("  nSamplesPerSec:  %u", deviceAudioFormat->nSamplesPerSec);
	PrintLine_D("  nAvgBytesPerSec: %u", deviceAudioFormat->nAvgBytesPerSec);
	PrintLine_D("  nBlockAlign:     %u", deviceAudioFormat->nBlockAlign);
	PrintLine_D("  wBitsPerSample:  %u", deviceAudioFormat->wBitsPerSample);
	PrintLine_D("  cbSize:          %u", deviceAudioFormat->cbSize);
	
	WAVEFORMATEX requestFormat;
	Win32_FillWaveFormatEx(format, &requestFormat);
	MyMemCopy(&Platform->audioFormat, format, sizeof(PlatAudioFormat_t));
	
	//TODO: Use IsFormatSupported
	//HRESULT IsFormatSupported(
	// 	[in]  AUDCLNT_SHAREMODE  ShareMode,
	// 	[in]  const WAVEFORMATEX *pFormat,
	// 	[out] WAVEFORMATEX       **ppClosestMatch
	// );
	
	REFERENCE_TIME bufferSize = (LONGLONG)AUDIO_OUTPUT_BUFFER_DURATION * 1000ULL * 10ULL; //100-nanosecond units
	HRESULT initializeResult = Platform->audioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,     //ShareMode (TODO: Should this be SHARED??)
		AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY, //StreamFlags
		bufferSize,                   //hnsBufferDuration
		0,                            //hnsPeriodicity 0=default (TODO: Do we want to use this?)
		&requestFormat,               //pFormat
		NULL                          //AudioSessionGuid (TODO: Is our IAudioClient our Session? Do we want to explcitly create our session?)
	);
	if (initializeResult != S_OK)
	{
		switch (initializeResult)
		{
			case AUDCLNT_E_ALREADY_INITIALIZED:          WriteLine_D("AUDCLNT_E_ALREADY_INITIALIZED");
			case AUDCLNT_E_WRONG_ENDPOINT_TYPE:          WriteLine_D("AUDCLNT_E_WRONG_ENDPOINT_TYPE");
			case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED:      WriteLine_D("AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED");
			case AUDCLNT_E_BUFFER_SIZE_ERROR:            WriteLine_D("AUDCLNT_E_BUFFER_SIZE_ERROR");
			case AUDCLNT_E_CPUUSAGE_EXCEEDED:            WriteLine_D("AUDCLNT_E_CPUUSAGE_EXCEEDED");
			case AUDCLNT_E_DEVICE_INVALIDATED:           WriteLine_D("AUDCLNT_E_DEVICE_INVALIDATED");
			case AUDCLNT_E_DEVICE_IN_USE:                WriteLine_D("AUDCLNT_E_DEVICE_IN_USE");
			case AUDCLNT_E_ENDPOINT_CREATE_FAILED:       WriteLine_D("AUDCLNT_E_ENDPOINT_CREATE_FAILED");
			case AUDCLNT_E_INVALID_DEVICE_PERIOD:        WriteLine_D("AUDCLNT_E_INVALID_DEVICE_PERIOD");
			case AUDCLNT_E_UNSUPPORTED_FORMAT:           WriteLine_D("AUDCLNT_E_UNSUPPORTED_FORMAT");
			case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:   WriteLine_D("AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED");
			case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL: WriteLine_D("AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL");
			case AUDCLNT_E_SERVICE_NOT_RUNNING:          WriteLine_D("AUDCLNT_E_SERVICE_NOT_RUNNING");
			case E_POINTER:                              WriteLine_D("E_POINTER");
			case E_INVALIDARG:                           WriteLine_D("E_INVALIDARG");
			case E_OUTOFMEMORY:                          WriteLine_D("E_OUTOFMEMORY");
			default: Assert(false);
		}
	}
	Assert(initializeResult == S_OK);
	
	HRESULT getServiceResult = Platform->audioClient->GetService(
		__uuidof(IAudioRenderClient),        //riid
		(void**)&Platform->audioRenderClient //ppv
	);
	Assert(getServiceResult == S_OK);
	NotNull(Platform->audioRenderClient);
	
	u32 bufferFrameCount = (u32)(((AUDIO_OUTPUT_BUFFER_DURATION / 1000ULL) * format->samplesPerSecond) / format->numChannels);
	
	BYTE* bufferPntr = nullptr;
	HRESULT getBufferResult = Platform->audioRenderClient->GetBuffer(
		bufferFrameCount,
		&bufferPntr
	);
	Assert(getBufferResult == S_OK);
	NotNull(bufferPntr);
	
	ClearStruct(Platform->audioServiceInfo);
	Platform->audioServiceInfo.thread          = nullptr;
	Platform->audioServiceInfo.format          = Platform->audioFormat;
	Platform->audioServiceInfo.audioFrameIndex = Platform->audioFrameIndex;
	Platform->audioServiceInfo.numFramesNeeded = bufferFrameCount;
	Platform->audioServiceInfo.bufferPntr      = (u8*)bufferPntr;
	Platform->audioServiceInfo.bufferSize      = bufferFrameCount * (Platform->audioFormat.bitsPerSample/8) * Platform->audioFormat.numChannels;
	Platform->audioServiceInfo.fillWithSilence = false;
	Platform->audioServiceInfo.numFramesFilled = 0;
	{
		#if 0
		Win32_ServiceAudioWithSineWave(&Platform->audioServiceInfo);
		#else
		Platform->audioServiceInfo.numFramesFilled = Platform->audioServiceInfo.numFramesNeeded;
		Platform->audioServiceInfo.fillWithSilence = true;
		#endif
	}
	
	HRESULT releaseBufferResult = Platform->audioRenderClient->ReleaseBuffer(
		bufferFrameCount,         //NumFramesWritten
		0 //dwFlags (AUDCLNT_BUFFERFLAGS_SILENT) TODO: Remove this flag once we implement the loop above
	);
	Assert(releaseBufferResult == S_OK);
	
	HRESULT startResult = Platform->audioClient->Start();
	Assert(startResult == S_OK);
	
	Platform->audioThread = Win32_CreateThread(Win32_AudioThreadFunc);
	Assert(Platform->audioThread != nullptr);
	PrintLine_I("Audio thread started (ID 0x%08X or %u)", Platform->audioThread->win32_id, Platform->audioThread->win32_id);
}

void Win32_UpdateAudio()
{
	//TODO: Any sort of managment we need to do in here?
}

// +--------------------------------------------------------------+
// |              Callback Interface Implementation               |
// +--------------------------------------------------------------+
//TODO: Implementation details/examples can be found at: https://docs.microsoft.com/en-us/windows/win32/coreaudio/device-events
ULONG AudioCallbackClass_c::AddRef()
{
	Unimplemented(); //TODO: Implement me!
	return 0;
}
HRESULT AudioCallbackClass_c::QueryInterface(REFIID riid, void** ppvObject)
{
	UNUSED(riid);
	UNUSED(ppvObject);
	Unimplemented(); //TODO: Implement me!
	return 0;
}
ULONG AudioCallbackClass_c::Release()
{
	Unimplemented(); //TODO: Implement me!
	return 0;
}

HRESULT AudioCallbackClass_c::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
	NotNull(pwstrDefaultDeviceId);
	ThreadId_t threadId = Win32_GetThisThreadId();
	Win32_LockMutex(&Platform->threadSafeHeapMutex, MUTEX_LOCK_INFINITE);
	MyStr_t idStr = ConvertUcs2StrToUtf8Nt(&Platform->threadSafeHeap, pwstrDefaultDeviceId);
	PlatAudioDevice_t* audioDevice = Win32_GetAudioDeviceByIdStr(idStr);
	MyStr_t nameStr = (audioDevice != nullptr) ? audioDevice->name : NewStr("[Unknown]");
	const char* flowStr = "[Unknown]";
	switch (flow)
	{
		case eRender: flowStr = "Render"; break;
		case eCapture: flowStr = "Capture"; break;
	}
	const char* roleStr = "[Unknown]";
	switch (role)
	{
		case eConsole: roleStr = "Console"; break;
		case eMultimedia: roleStr = "Multimedia"; break;
		case eCommunications: roleStr = "Communications"; break;
	}
	MyStr_t printStr = PrintInArenaStr(&Platform->threadSafeHeap, "OnDefaultDeviceChanged: thread %u %s %s \"%.*s\"", threadId, flowStr, roleStr, nameStr.length, nameStr.pntr);
	WriteLine_N(printStr.pntr);
	FreeString(&Platform->threadSafeHeap, &printStr);
	Win32_UnlockMutex(&Platform->threadSafeHeapMutex);
	// Unimplemented(); //TODO: Implement me!
	return 0;
}
HRESULT AudioCallbackClass_c::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
	NotNull(pwstrDeviceId);
	ThreadId_t threadId = Win32_GetThisThreadId();
	Win32_LockMutex(&Platform->threadSafeHeapMutex, MUTEX_LOCK_INFINITE);
	MyStr_t idStr = ConvertUcs2StrToUtf8Nt(&Platform->threadSafeHeap, pwstrDeviceId);
	PlatAudioDevice_t* audioDevice = Win32_GetAudioDeviceByIdStr(idStr);
	MyStr_t nameStr = (audioDevice != nullptr) ? audioDevice->name : NewStr("[Unknown]");
	MyStr_t printStr = PrintInArenaStr(&Platform->threadSafeHeap, "OnDeviceAdded: thread %u \"%.*s\"", threadId, nameStr.length, nameStr.pntr);
	WriteLine_N(printStr.pntr);
	FreeString(&Platform->threadSafeHeap, &printStr);
	Win32_UnlockMutex(&Platform->threadSafeHeapMutex);
	// Unimplemented(); //TODO: Implement me!
	return 0;
}
HRESULT AudioCallbackClass_c::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
	NotNull(pwstrDeviceId);
	ThreadId_t threadId = Win32_GetThisThreadId();
	Win32_LockMutex(&Platform->threadSafeHeapMutex, MUTEX_LOCK_INFINITE);
	MyStr_t idStr = ConvertUcs2StrToUtf8Nt(&Platform->threadSafeHeap, pwstrDeviceId);
	PlatAudioDevice_t* audioDevice = Win32_GetAudioDeviceByIdStr(idStr);
	MyStr_t nameStr = (audioDevice != nullptr) ? audioDevice->name : NewStr("[Unknown]");
	MyStr_t printStr = PrintInArenaStr(&Platform->threadSafeHeap, "OnDeviceRemoved: thread %u \"%.*s\"", threadId, nameStr.length, nameStr.pntr);
	WriteLine_N(printStr.pntr);
	FreeString(&Platform->threadSafeHeap, &printStr);
	Win32_UnlockMutex(&Platform->threadSafeHeapMutex);
	// Unimplemented(); //TODO: Implement me!
	return 0;
}
HRESULT AudioCallbackClass_c::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
	NotNull(pwstrDeviceId);
	ThreadId_t threadId = Win32_GetThisThreadId();
	Win32_LockMutex(&Platform->threadSafeHeapMutex, MUTEX_LOCK_INFINITE);
	MyStr_t idStr = ConvertUcs2StrToUtf8Nt(&Platform->threadSafeHeap, pwstrDeviceId);
	PlatAudioDevice_t* audioDevice = Win32_GetAudioDeviceByIdStr(idStr);
	MyStr_t nameStr = (audioDevice != nullptr) ? audioDevice->name : NewStr("[Unknown]");
	MyStr_t printStr = PrintInArenaStr(&Platform->threadSafeHeap, "OnDeviceStateChanged: thread %u 0x%08X \"%.*s\"", threadId, dwNewState, nameStr.length, nameStr.pntr);
	WriteLine_N(printStr.pntr);
	FreeString(&Platform->threadSafeHeap, &printStr);
	Win32_UnlockMutex(&Platform->threadSafeHeapMutex);
	// Unimplemented(); //TODO: Implement me!
	return 0;
}
HRESULT AudioCallbackClass_c::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
	UNUSED(key);
	NotNull(pwstrDeviceId);
	// WriteLine_N("OnPropertyValueChanged");
	// Unimplemented(); //TODO: Implement me!
	return 0;
}

// +--------------------------------------------------------------+
// |                    Audio Thread Function                     |
// +--------------------------------------------------------------+
void Win32_FillAudioBufferToMeetTargetFill()
{
	UINT32 audioPadding = 0;
	HRESULT getPaddingResult = Platform->audioClient->GetCurrentPadding(&audioPadding);
	Assert(getPaddingResult == S_OK);
	#if DEBUG_BUILD
	if (audioPadding == 0) { Platform->numAudioFrameDrops++; }
	#endif
	// PrintLine_D("Audio Padding: %lu", audioPadding);
	
	u32 audioFillFrameCount = (u32)((AUDIO_FILL_TIME * Platform->audioFormat.samplesPerSecond) / 1000);
	if (audioPadding < audioFillFrameCount)
	{
		u32 numFramesToFill = (audioFillFrameCount - audioPadding);
		// PrintLine_D("Filling %lu frames", numFramesToFill);
		
		BYTE* bufferPntr = nullptr;
		HRESULT getBufferResult = Platform->audioRenderClient->GetBuffer(
			numFramesToFill,
			&bufferPntr
		);
		Assert(getBufferResult == S_OK);
		NotNull(bufferPntr);
		
		ClearStruct(Platform->audioServiceInfo);
		Platform->audioServiceInfo.thread          = Platform->audioThread;
		Platform->audioServiceInfo.format          = Platform->audioFormat;
		Platform->audioServiceInfo.audioFrameIndex = Platform->audioFrameIndex;
		Platform->audioServiceInfo.numFramesNeeded = numFramesToFill;
		Platform->audioServiceInfo.bufferPntr      = (u8*)bufferPntr;
		Platform->audioServiceInfo.bufferSize      = numFramesToFill * (Platform->audioFormat.bitsPerSample/8) * Platform->audioFormat.numChannels;
		Platform->audioServiceInfo.fillWithSilence = false;
		Platform->audioServiceInfo.numFramesFilled = 0;
		
		if (Platform->engine.isValid && Platform->engine.AudioService != nullptr &&
			InitPhase >= Win32InitPhase_Initialized && !Platform->audioWaitForFirstUpdateAfterReload)
		{
			Platform->engine.AudioService(&Platform->audioServiceInfo);
		}
		else
		{
			#if 0
			Win32_ServiceAudioWithSineWave(&Platform->audioServiceInfo);
			#else
			Platform->audioServiceInfo.numFramesFilled = Platform->audioServiceInfo.numFramesNeeded;
			Platform->audioServiceInfo.fillWithSilence = true;
			#endif
		}
		Assert_(Platform->audioServiceInfo.numFramesFilled <= Platform->audioServiceInfo.numFramesNeeded);
		
		HRESULT releaseBufferResult = Platform->audioRenderClient->ReleaseBuffer(
			(UINT32)Platform->audioServiceInfo.numFramesFilled,                           //NumFramesWritten
			(Platform->audioServiceInfo.fillWithSilence ? AUDCLNT_BUFFERFLAGS_SILENT : 0) //dwFlags
		);
		Assert(releaseBufferResult == S_OK);
		
		Platform->audioFrameIndex += Platform->audioServiceInfo.numFramesFilled;
	}
}

// +==============================+
// |    Win32_AudioThreadFunc     |
// +==============================+
THREAD_FUNCTION_DEF(Win32_AudioThreadFunc, userPntr) //pre-declared at top of file
{
	UNUSED(userPntr);
	WriteLine_I("Audio thread is starting...");
	while (InitPhase < Win32InitPhase_Closing)
	{
		if (Win32_LockMutex(&Platform->audioOutputMutex, MUTEX_LOCK_INFINITE))
		{
			Win32_FillAudioBufferToMeetTargetFill();
			Win32_UnlockMutex(&Platform->audioOutputMutex);
		}
		Win32_SleepForMs(AUDIO_SLEEP_TIME); //TODO: Adjust this time based off how long calculating samples took
	}
	WriteLine_W("Audio thread is exiting...");
	return 0;
}
