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
#pragma warning(disable:4100) //unreferenced formal parameter
#include "stb/stb_vorbis.c"
#pragma warning(pop)

enum OggError_t
{
	OggError_None = 0,
	OggError_MissingFile,
	OggError_UnknownExtension,
	OggError_EmptyFile,
	OggError_DecodeError,
	OggError_NoSamples,
	OggError_NoDecodedData,
	OggError_InvalidChannelCount,
	OggError_InvalidSampleRate,
	OggError_NotEnoughMemory,
	OggError_NumCodes,
};
const char* GetOggErrorStr(OggError_t oggError)
{
	switch (oggError)
	{
		case OggError_None:                return "None";
		case OggError_MissingFile:         return "MissingFile";
		case OggError_UnknownExtension:    return "UnknownExtension";
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

const char* GetOggVorbisErrorStr(int errorCode)
{
	switch (errorCode)
	{
		case VORBIS__no_error:                        return "_no_error";
		case VORBIS_need_more_data:                   return "need_more_data";
		case VORBIS_invalid_api_mixing:               return "invalid_api_mixing";
		case VORBIS_outofmem:                         return "outofmem";
		case VORBIS_feature_not_supported:            return "feature_not_supported";
		case VORBIS_too_many_channels:                return "too_many_channels";
		case VORBIS_file_open_failure:                return "file_open_failure";
		case VORBIS_seek_without_length:              return "seek_without_length";
		case VORBIS_unexpected_eof:                   return "unexpected_eof";
		case VORBIS_seek_invalid:                     return "seek_invalid";
		case VORBIS_invalid_setup:                    return "invalid_setup";
		case VORBIS_invalid_stream:                   return "invalid_stream";
		case VORBIS_missing_capture_pattern:          return "missing_capture_pattern";
		case VORBIS_invalid_stream_structure_version: return "invalid_stream_structure_version";
		case VORBIS_continued_packet_flag_invalid:    return "continued_packet_flag_invalid";
		case VORBIS_incorrect_stream_serial_number:   return "incorrect_stream_serial_number";
		case VORBIS_invalid_first_page:               return "invalid_first_page";
		case VORBIS_bad_packet_type:                  return "bad_packet_type";
		case VORBIS_cant_find_last_page:              return "cant_find_last_page";
		case VORBIS_seek_failed:                      return "seek_failed";
		case VORBIS_ogg_skeleton_not_supported:       return "ogg_skeleton_not_supported";
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
	if (oggData->samplesSize > 0) { FreeMem(oggData->allocArena, oggData->samples, oggData->samplesSize); }
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

#if 0
//If this returns true, then ownership of the openFile and fifo contents are passed to the AudioStream_t
bool CreateOggAudioStream(ProcessLog_t* log, PlatOpenFile_t* openFile, Fifo_t* fifo, AudioStream_t* streamOut)
{
	AssertSingleThreaded(); //TODO: This doesn't have to be single threaded if we make the nextWavOggAudioDataId thread safe
	NotNull4(log, openFile, fifo, streamOut);
	Assert(openFile->isOpen);
	Assert(openFile->cursorIndex == 0);
	Assert(fifo->allocArena == nullptr || fifo->allocArena != TempArena);
	int stbError = 0;
	
	SetProcessLogName(log, NewStr("DeserOggFileForStream"));
	LogWriteLine_N(log, "Entering CreateOggAudioStream...");
	LogPrintLine_N(log, "We expect the ogg file to be %s", FormatBytesNt(openFile->fileSize, TempArena));
	
	ClearPointer(streamOut);
	if (pig->nextWavOggAudioDataId == 0) { pig->nextWavOggAudioDataId = 1; }
	streamOut->id = pig->nextWavOggAudioDataId;
	pig->nextWavOggAudioDataId++;
	
	u64 numBytesRead = 0;
	u64 numBytesConsumed = 0;
	u8 readBuffer[4096];
	
	numBytesRead = plat->ReadFromFile(openFile, ArrayCount(readBuffer), &readBuffer[0], false);
	LogPrintLine_D(log, "Tried to read %s, actually read %s", FormatBytesNt(ArrayCount(readBuffer), TempArena), FormatBytesNt(numBytesRead, TempArena));
	
	int numInitialBytesConsumed = 0;
	stb_vorbis* stbHandle = stb_vorbis_open_pushdata(
         &readBuffer[0],
         (int)numBytesRead,
         &numInitialBytesConsumed,
         &stbError,
         nullptr);
	
	if (stbHandle == nullptr)
	{
		LogPrintLine_E(log, "Failed to start parsing the ogg file! Maybe this isn't a valid ogg file? Maybe our initial read buffer isn't large enough? Stb Error: %s", GetOggVorbisErrorStr(stbError));
		LogExitFailure(log, OggError_DecodeError);
		return false;
	}
	
	stb_vorbis_info vorbisInfo = stb_vorbis_get_info(stbHandle);
	LogPrintLine_I(log, "SampleRate: %u", vorbisInfo.sample_rate);
	LogPrintLine_I(log, "NumChannels: %d", vorbisInfo.channels);
	LogPrintLine_I(log, "MemRequired: %u", vorbisInfo.setup_memory_required);
	LogPrintLine_I(log, "TempMemRequired: %u", vorbisInfo.temp_memory_required);
	LogPrintLine_I(log, "MaxFrameSize: %d", vorbisInfo.max_frame_size);
	log->hadWarnings = true; //TODO: Remove me!
	
	stb_vorbis_close(stbHandle);
	
	MyMemCopy(&streamOut->openFile, openFile, sizeof(PlatOpenFile_t));
	MyMemCopy(&streamOut->samplesFifo, fifo, sizeof(Fifo_t));
	return false; //TODO: Change me to true!
}
#endif
