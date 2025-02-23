#include <stdio.h>
#include <stdlib.h>
#include <nds.h>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "wav_nds.h"
#include "mem.h"

wav_handle* wav_load_handle(const char *filename)
{
	drwav wavfp;

	if (!drwav_init_file(&wavfp, filename, NULL))
	{
		return 0;
	}

	s16* pSampleData = (s16*)mem_alloc(sizeof(s16) * (u32)wavfp.totalPCMFrameCount * wavfp.channels);
	if (pSampleData == 0)
	{
		drwav_uninit(&wavfp);
		return 0;
	}
	u32 totalRead = drwav_read_pcm_frames(&wavfp, wavfp.totalPCMFrameCount, pSampleData);
	if (!totalRead)
	{
		drwav_uninit(&wavfp);
		mem_free(pSampleData);
		return 0;
	}

	if (wavfp.bitsPerSample == 8) // 8 bit
	{
		s16* _8bitdata = (s16*)mem_alloc(sizeof(s16) * (u32)wavfp.totalPCMFrameCount * wavfp.channels);
		drwav_u8_to_s16((drwav_int16*)_8bitdata, (drwav_uint8*)pSampleData, wavfp.totalPCMFrameCount);
		mem_free(pSampleData);
		pSampleData = _8bitdata;
	}

	wav_handle* handle = (wav_handle*)mem_alloc(sizeof(wav_handle));
	handle->data = pSampleData;
	handle->size = totalRead*2;
	handle->samprate = wavfp.sampleRate;
	drwav_uninit(&wavfp);

	return handle;
}

void wav_free_handle(wav_handle* handle)
{
	if (!handle || !handle->data) return;

	mem_free(handle->data);
	handle->data = 0;

	mem_free(handle);
}

int wav_play(wav_handle* handle)
{
	if (!handle || !handle->data) return -1;

	return soundPlaySample(handle->data, SoundFormat_16Bit, handle->size, handle->samprate, 127, 64, false, 0);
}
