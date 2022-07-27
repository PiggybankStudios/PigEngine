/*
File:   pig_ogg.cpp
Author: Taylor Robbins
Date:   07\10\2022
Description: 
	** Holds the function (and supporting types and functions) that parse .ogg files into OggAudioData_t structures 
*/

// #define STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_NO_STDIO
// #define STB_VORBIS_NO_PUSHDATA_API
// #define STB_VORBIS_NO_PULLDATA_API
// #define STB_VORBIS_NO_INTEGER_CONVERSION
// #define STB_VORBIS_NO_FAST_SCALED_FLOAT
// #define STB_VORBIS_MAX_CHANNELS 16

#pragma warning(push)
#pragma warning(disable:4244) //'=': conversion from 'int' to 'uint8', possible loss of data
#pragma warning(disable:4245) //'=': conversion from 'int' to 'uint32', signed/unsigned mismatch
#pragma warning(disable:4456) //declaration of 'z' hides previous local declaration
#pragma warning(disable:4457) //declaration of 'n' hides function parameter
#pragma warning(disable:4701) //potentially uninitialized local variable 'mid' used
#include "stb/stb_vorbis.c"
#pragma warning(pop)

enum OggError_t
{
	OggError_None = 0,
	OggError_EmptyFile,
	OggError_DecodeError,
	OggError_NoSamples,
	OggError_NoDecodedData,
	OggError_InvalidChannelCount,
	OggError_InvalidSampleRate,
	OggError_NotEnoughMemory,
	OggError_NumCodes,
};
const char* GetOggErrorStr(OggError_t wavError)
{
	switch (wavError)
	{
		case OggError_None:                return "None";
		case OggError_EmptyFile:           return "EmptyFile";
		case OggError_DecodeError:         return "DecodeError";
		case OggError_NoSamples:           return "NoSamples";
		case OggError_NoDecodedData:       return "NoDecodedData";
		case OggError_InvalidChannelCount: return "InvalidChannelCount";
		case OggError_InvalidSampleRate:   return "InvalidSampleRate";
		case OggError_NotEnoughMemory:     return "NotEnoughMemory";
		default: return "Unknown";
	}
}

struct OggAudioData_t
{
	MemArena_t* allocArena;
	u64 id;
	
	PlatAudioFormat_t format;
	u64 totalNumFrames;
	
	u64 samplesSize;
	i16* samples;
};

void FreeOggAudioData(OggAudioData_t* oggData)
{
	NotNull(oggData);
	AssertIf(oggData->samples != nullptr, oggData->allocArena != nullptr);
	FreeMem(oggData->allocArena, oggData->samples, oggData->samplesSize);
	ClearPointer(oggData);
}

bool TryDeserOggFile(u64 oggFileSize, const void* oggFilePntr, ProcessLog_t* log, OggAudioData_t* oggDataOut, MemArena_t* memArena)
{
	AssertSingleThreaded(); //TODO: This doesn't have to be single threaded if we make the nextWavOggAudioDataId thread safe
	NotNull(log);
	AssertIf(oggFileSize > 0, oggFilePntr != nullptr);
	NotNull(oggDataOut);
	NotNull(memArena);
	
	SetProcessLogName(log, NewStr("DeserOggFile"));
	LogWriteLine_N(log, "Entering TryDeserOggFile...");
	LogPrintLine_I(log, "Parsing %llu byte ogg file...", oggFileSize);
	
	if (oggFileSize == 0)
	{
		LogExitFailure(log, OggError_EmptyFile);
		return false;
	}
	
	ClearPointer(oggDataOut);
	oggDataOut->allocArena = memArena;
	if (pig->nextWavOggAudioDataId == 0) { pig->nextWavOggAudioDataId = 1; }
	oggDataOut->id = pig->nextWavOggAudioDataId;
	pig->nextWavOggAudioDataId++;
	
	int decodedNumChannels = 0;
	int decodedSampleRate = 0;
	short* decodedData = nullptr;
	int decodeResult = stb_vorbis_decode_memory(
		(const unsigned char*)oggFilePntr, //mem
		(int)oggFileSize,                  //len
		&decodedNumChannels,               //channels
		&decodedSampleRate,                //sample_rate
		&decodedData                       //output
	);
	if (decodeResult == -1)
	{
		LogWriteLine_E(log, "stb_vorbis_decode_memory returned -1 error code. Is this actually an ogg vorbis file?");
		LogExitFailure(log, OggError_DecodeError);
		return false;
	}
	if (decodeResult == 0)
	{
		LogWriteLine_E(log, "The ogg file contained no samples");
		LogExitFailure(log, OggError_NoSamples);
		return false;
	}
	if (decodedData == nullptr)
	{
		LogWriteLine_E(log, "stb_vorbis_decode_memory returned success but decodedData was not filled");
		LogExitFailure(log, OggError_NoDecodedData);
		return false;
	}
	if (decodedNumChannels != 1 && decodedNumChannels != 2)
	{
		LogPrintLine_E(log, "Unsupported channel count: %d", decodedNumChannels);
		LogExitFailure(log, OggError_InvalidChannelCount);
		return false;
	}
	//TODO: Add more supported sample rates if we can support them
	if (decodedSampleRate != platInfo->audioFormat.samplesPerSecond)
	{
		LogPrintLine_E(log, "Unsupported sample rate: %d (we only support %llu currently)", decodedSampleRate, platInfo->audioFormat.samplesPerSecond);
		LogExitFailure(log, OggError_InvalidSampleRate);
		return false;
	}
	
	LogPrintLine_I(log, "Deserialized %d samples! (%dHz %d channel%s)", decodeResult, decodedSampleRate, decodedNumChannels, (decodedNumChannels == 1) ? "" : "s");
	oggDataOut->format.numChannels      = (u64)decodedNumChannels;
	oggDataOut->format.samplesPerSecond = (u64)decodedSampleRate;
	oggDataOut->format.bitsPerSample    = sizeof(i16) * 8;
	
	//TODO: Figure out if decodeResult is num samples or num frames
	oggDataOut->totalNumFrames = (u64)decodeResult;
	u64 frameSize = (oggDataOut->format.bitsPerSample / 8) * oggDataOut->format.numChannels;
	oggDataOut->samplesSize = frameSize * oggDataOut->totalNumFrames;
	oggDataOut->samples = AllocArray(memArena, i16, oggDataOut->totalNumFrames * oggDataOut->format.numChannels);
	if (oggDataOut->samples == nullptr)
	{
		LogPrintLine_E(log, "Failed to allocate %llu bytes for %llu audio samples (%llu frames)", oggDataOut->samplesSize, oggDataOut->totalNumFrames * oggDataOut->format.numChannels, oggDataOut->totalNumFrames);
		LogExitFailure(log, OggError_NotEnoughMemory);
		return false;
	}
	
	short* readPntr = (short*)decodedData;
	i16* writePntr = (i16*)oggDataOut->samples;
	for (u64 frameIndex = 0; frameIndex < oggDataOut->totalNumFrames; frameIndex++)
	{
		for (u64 channelIndex = 0; channelIndex < oggDataOut->format.numChannels; channelIndex++)
		{
			*writePntr = *readPntr;
			writePntr++;
			readPntr++;
		}
	}
	
	free(decodedData);
	return true;
}

