/*
File:   pig_wav.cpp
Author: Taylor Robbins
Date:   02\28\2022
Description: 
	** Holds the function (and supporting types and functions) that parse .wav files into WavAudioData_t structures
*/

// +--------------------------------------------------------------+
// |                         Public Types                         |
// +--------------------------------------------------------------+
enum WavError_t
{
	WavError_None = 0,
	WavError_EmptyFile,
	WavError_InvalidRIFF,
	WavError_InvalidWAVE,
	WavError_MissingFormatChunk,
	WavError_MissingDataChunks,
	WavError_UnsupportedFormat,
	WavError_DataBeforeFormat,
	WavError_InvalidDataChunkSize,
	WavError_NumCodes,
};
const char* GetWavErrorStr(WavError_t wavError)
{
	switch (wavError)
	{
		case WavError_None:                 return "None";
		case WavError_EmptyFile:            return "EmptyFile";
		case WavError_InvalidRIFF:          return "InvalidRIFF";
		case WavError_InvalidWAVE:          return "InvalidWAVE";
		case WavError_MissingFormatChunk:   return "MissingFormatChunk";
		case WavError_MissingDataChunks:    return "MissingDataChunks";
		case WavError_UnsupportedFormat:    return "UnsupportedFormat";
		case WavError_DataBeforeFormat:     return "DataBeforeFormat";
		case WavError_InvalidDataChunkSize: return "InvalidDataChunkSize";
		default: return "Unknown";
	}
}

struct WavAudioDataChunk_t
{
	WavAudioDataChunk_t* next;
	u64 numFrames;
	u64 dataSize;
	//Data is allocated immediately following this structure
};
struct WavAudioData_t
{
	MemArena_t* allocArena;
	u64 id;
	
	PlatAudioFormat_t format;
	u64 totalNumFrames;
	
	u64 numChunks;
	WavAudioDataChunk_t* firstChunk;
	WavAudioDataChunk_t* lastChunk;
};

// +--------------------------------------------------------------+
// |                 Parsing Helper Macros/Types                  |
// +--------------------------------------------------------------+
#define WAV_CHUNK_TYPE_ID(a, b, c, d) (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

START_PACK()

struct ATTR_PACKED WAV_Header_t
{
	u32 RIFFID;
	u32 size;
	u32 WAVEID;
};

struct ATTR_PACKED WAV_Chunk_t
{
	u32 id;
	u32 size;
};

struct ATTR_PACKED WAV_FormatChunk_t
{
	u16 formatTag;
	u16 numChannels;
	u32 numSamplesPerSecond;
	u32 avgBytesPerSec;
	u16 nBlockAlign;
	u16 bitsPerSample;
	u16 cbSize;
	u16 validBitsPerSample;
	u32 dwChannelMask;
	u8 subFormat[16];
};

END_PACK()

struct WAV_RiffIterator_t
{
	u8* pntr;
	u8* stop;
};

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
inline WAV_RiffIterator_t WAV_ParseChunkAt(void* dataPntr, void* stop)
{
	WAV_RiffIterator_t result;
	result.pntr = (u8*)dataPntr;
	result.stop = (u8*)stop; 
	
	return result;
}

inline WAV_RiffIterator_t WAV_NextChunk(WAV_RiffIterator_t iter)
{
	WAV_Chunk_t* chunk = (WAV_Chunk_t*)iter.pntr;
	
	u32 stepSize = chunk->size;
	if ((stepSize%2) == 1) stepSize++;
	
	iter.pntr += sizeof(WAV_Chunk_t) + stepSize;
	
	return iter;
}

inline bool32 WAV_IsValid(WAV_RiffIterator_t iter)
{
	bool32 result = (iter.pntr < iter.stop);
	return result;
}

inline void* WAV_GetChunkData(WAV_RiffIterator_t iter)
{
	void* result = (iter.pntr + sizeof(WAV_Chunk_t));
	
	return result;
}

inline u32 WAV_GetType(WAV_RiffIterator_t iter)
{
	WAV_Chunk_t* chunk = (WAV_Chunk_t*)iter.pntr;
	u32 result = chunk->id;
	
	return result;
}

inline u32 WAV_GetChunkDataSize(WAV_RiffIterator_t iter)
{
	WAV_Chunk_t* chunk = (WAV_Chunk_t*)iter.pntr;
	u32 result = chunk->size;
	
	return result;
}

// +--------------------------------------------------------------+
// |                        Free Function                         |
// +--------------------------------------------------------------+
void FreeWavAudioData(WavAudioData_t* wavData)
{
	NotNull(wavData);
	AssertIf(wavData->numChunks > 0, wavData->allocArena != nullptr);
	WavAudioDataChunk_t* chunk = wavData->firstChunk;
	for (u64 cIndex = 0; cIndex < wavData->numChunks; cIndex++)
	{
		NotNull(chunk);
		WavAudioDataChunk_t* nextChunk = chunk->next;
		FreeMem(wavData->allocArena, chunk, sizeof(WavAudioDataChunk_t) + chunk->dataSize);
		chunk = nextChunk;
	}
	ClearPointer(wavData);
}

// +--------------------------------------------------------------+
// |                       Parsing Function                       |
// +--------------------------------------------------------------+
bool TryDeserWavFile(u64 wavFileSize, const void* wavFilePntr, ProcessLog_t* log, WavAudioData_t* wavDataOut, MemArena_t* memArena)
{
	AssertSingleThreaded(); //TODO: This doesn't have to be single threaded if we make the nextWavOggAudioDataId thread safe
	NotNull(log);
	AssertIf(wavFileSize > 0, wavFilePntr != nullptr);
	NotNull(wavDataOut);
	NotNull(memArena);
	
	SetProcessLogName(log, NewStr("DeserWavFile"));
	LogWriteLine_N(log, "Entering TryDeserWavFile...");
	
	if (wavFileSize < sizeof(WAV_Header_t))
	{
		LogWriteLine_E(log, "An empty file is not a valid WAV file");
		LogExitFailure(log, WavError_EmptyFile);
		return false;
	}
	
	// const u8* wavFileBytes = (const u8*)wavFilePntr; //TODO: Do we need this?
	WAV_Header_t* wavHeader = (WAV_Header_t*)wavFilePntr;
	
	if (wavHeader->RIFFID != WAV_CHUNK_TYPE_ID('R', 'I', 'F', 'F'))
	{
		LogWriteLine_E(log, "Invalid RIFF code at start of file. Is this really a WAV file?");
		LogExitFailure(log, WavError_InvalidRIFF);
		return false;
	}
	if (wavHeader->WAVEID != WAV_CHUNK_TYPE_ID('W', 'A', 'V', 'E'))
	{
		LogWriteLine_E(log, "Invalid WAVE code at start of file. Is this really a WAV file?");
		LogExitFailure(log, WavError_InvalidWAVE);
		return false;
	}
	
	ClearPointer(wavDataOut);
	wavDataOut->allocArena = memArena;
	if (pig->nextWavOggAudioDataId == 0) { pig->nextWavOggAudioDataId = 1; }
	wavDataOut->id = pig->nextWavOggAudioDataId;
	pig->nextWavOggAudioDataId++;
	wavDataOut->numChunks = 0;
	wavDataOut->firstChunk = nullptr;
	wavDataOut->lastChunk = nullptr;
	
	bool foundFormat = false;
	
	const u32 WAV_ChunkId_fmt  = WAV_CHUNK_TYPE_ID('f', 'm', 't', ' ');
	const u32 WAV_ChunkId_data = WAV_CHUNK_TYPE_ID('d', 'a', 't', 'a');
	
	for (WAV_RiffIterator_t iter = WAV_ParseChunkAt(wavHeader + 1, (u8*)(wavHeader+1) + wavHeader->size-4); 
		WAV_IsValid(iter);
		iter = WAV_NextChunk(iter))
	{
		switch (WAV_GetType(iter))
		{
			// +==============================+
			// |          fmt Chunk           |
			// +==============================+
			case WAV_ChunkId_fmt:
			{
				WAV_FormatChunk_t* format = (WAV_FormatChunk_t*)WAV_GetChunkData(iter);
				// Assert(format->nBlockAlign == sizeof(i16)*format->numChannels); //TODO: Why was this enabled? Should it be re-enabled?
				
				if (format->formatTag != 0x01)
				{
					LogPrintLine_E(log, "The formatTag in the WAV file is unsupported. We currently only support uncompressed PCM data. Found format 0x%02X", format->formatTag);
					LogExitFailure(log, WavError_UnsupportedFormat);
					FreeWavAudioData(wavDataOut);
					return false;
				}
				if (format->numChannels != 1 && format->numChannels != 2)
				{
					LogPrintLine_E(log, "We don't support %u channel wav files!", format->numChannels);
					LogExitFailure(log, WavError_UnsupportedFormat);
					FreeWavAudioData(wavDataOut);
					return false;
				}
				if (format->bitsPerSample != 16)
				{
					LogPrintLine_E(log, "We don't support %ubit wav files!", format->bitsPerSample);
					LogExitFailure(log, WavError_UnsupportedFormat);
					FreeWavAudioData(wavDataOut);
					return false;
				}
				
				//TODO: Should we put any restrictions on what samplesPerSecond values we support?
				
				wavDataOut->format.samplesPerSecond = (u64)format->numSamplesPerSecond;
				wavDataOut->format.bitsPerSample = (u64)format->bitsPerSample;
				wavDataOut->format.numChannels = (u64)format->numChannels;
				
				foundFormat = true;
			} break;
			
			// +==============================+
			// |          data Chunk          |
			// +==============================+
			case WAV_ChunkId_data:
			{
				if (!foundFormat)
				{
					LogWriteLine_E(log, "\"data\" chunk came before \"fmt\" chunk!");
					LogExitFailure(log, WavError_DataBeforeFormat);
					FreeWavAudioData(wavDataOut);
					return false;
				}
				
				u32 sampleDataSize = WAV_GetChunkDataSize(iter);
				Assert(sampleDataSize > 0);
				u64 formatFrameSize = (wavDataOut->format.bitsPerSample/8) * wavDataOut->format.numChannels;
				if ((sampleDataSize % formatFrameSize) != 0)
				{
					LogPrintLine_E(log, "Data chunk contained an invalid number of bytes for %llu channel %llubit audio: %u bytes", wavDataOut->format.numChannels, wavDataOut->format.bitsPerSample, sampleDataSize);
					LogExitFailure(log, WavError_InvalidDataChunkSize);
					FreeWavAudioData(wavDataOut);
					return false;
				}
				
				u8* newChunkSpace = (u8*)AllocMem(memArena, sizeof(WavAudioDataChunk_t) + sampleDataSize);
				NotNull(newChunkSpace); //TODO: Should we handle this as an error?
				WavAudioDataChunk_t* newChunk = (WavAudioDataChunk_t*)newChunkSpace;
				u8* newChunkDataPntr = (newChunkSpace + sizeof(WavAudioDataChunk_t));
				ClearPointer(newChunk);
				newChunk->next = nullptr;
				newChunk->numFrames = (sampleDataSize / formatFrameSize);
				newChunk->dataSize = sampleDataSize;
				MyMemCopy(newChunkDataPntr, WAV_GetChunkData(iter), sampleDataSize);
				
				if (wavDataOut->firstChunk == nullptr)
				{
					wavDataOut->firstChunk = newChunk;
					wavDataOut->lastChunk = newChunk;
				}
				else
				{
					wavDataOut->lastChunk->next = newChunk;
					wavDataOut->lastChunk = newChunk;
				}
				wavDataOut->numChunks++;
				wavDataOut->totalNumFrames += newChunk->numFrames;
			} break;
			
			// +==============================+
			// |        Unknown Chunk         |
			// +==============================+
			default:
			{
				LogPrintLine_W(log, "Unknown chunk ID in wav file: \"%.4s\"", (char*)iter.pntr);
				log->hadWarnings = true;
			};
		}
	}
	
	if (!foundFormat)
	{
		LogWriteLine_E(log, "We did not find the format chunk in the WAV file");
		LogExitFailure(log, WavError_MissingFormatChunk);
		FreeWavAudioData(wavDataOut);
		return false;
	}
	if (wavDataOut->totalNumFrames == 0)
	{
		LogWriteLine_E(log, "We did not find any data chunks in the WAV file");
		LogExitFailure(log, WavError_MissingDataChunks);
		FreeWavAudioData(wavDataOut);
		return false;
	}
	
	return true;
}
