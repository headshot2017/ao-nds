extern "C" {
#define DR_WAV_IMPLEMENTATION
#include <nds.h>
#include <stdio.h>
#include <fat.h>
#include <nds/arm9/sound.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>
#include <dr_wav.h>
#include "mp3dec.h"
#include <mp3_shared.h>

volatile mp3_player		*mp3;
FILE	*mp3_file;
float32 mp3_length;
float32 mp3_loopsec;
u8		*mp3_buffer;
u16		*mp3_audioLeft;
u16		*mp3_audioRight;

int filled = 0;

void dump_buffer() {
        FILE *f = fopen("test.txt","wb");
        fwrite((void *)mp3->audioLeft,1,MP3_AUDIO_BUFFER_SIZE,f);
        // I'm not dumping right channel
        fclose(f);
}


void mp3_fill_buffer() {
        int n;
        //iprintf("in\n");
        //mp3_print();
        if(mp3 && mp3->flag) {
				switch(mp3->flag)
				{
					case 1: // stream more mp3 data
						n = fread((void *)(mp3->buffer + MP3_FILE_BUFFER_SIZE), 1, MP3_FILE_BUFFER_SIZE, mp3_file);
						filled += n;
						if(n < MP3_FILE_BUFFER_SIZE) {
								if (mp3->loop)
								{
										float bytes;
										if (!mp3_length) mp3_length = (float)mp3->soundtime/(float)mp3->rate;
										bytes = (1.f / mp3_length) * mp3->filesize;
										
										fseek (mp3_file, (int)(mp3_loopsec * bytes) / MP3_FILE_BUFFER_SIZE * MP3_FILE_BUFFER_SIZE, SEEK_SET);
										n = fread((void *)(mp3->buffer + MP3_FILE_BUFFER_SIZE + n), 1, MP3_FILE_BUFFER_SIZE-n, mp3_file);
										filled += n;
								}
						}
						break;
					
					case 2: // non-looping mp3 reached the end
						mp3_stop();
						break;
				}
				mp3->flag = 0;
        }
        //iprintf("out\n");
}

void *uncached_malloc(size_t count) {
        void *p = malloc(count);
        return ((p == 0) ? 0 : memUncached(p));
}

int ds_filelength (FILE *f)
{
        int             pos;
        int             end;

        pos = ftell (f);
        fseek (f, 0, SEEK_END);
        end = ftell (f);
        fseek (f, pos, SEEK_SET);

        return end;
}

int mp3_play_file(FILE *file, int loop, float loopsec){

        mp3_msg msg;
        int ret = 0;

        if(mp3 == 0)
		{
            return false;
        }

        memset((void *)mp3,0,sizeof(*mp3));
		int err;
		mp3->hMP3Decoder = MP3InitDecoder(&err);
        mp3->buffer = mp3_buffer;
        mp3->audioLeft = mp3_audioLeft;
        mp3->audioRight = mp3_audioRight;

        mp3_file = file;
        mp3->filesize = ds_filelength(mp3_file);
        mp3->loop = loop;
		mp3_loopsec = loopsec;


        msg.type = MP3_MSG_START;
        msg.player = mp3;

        filled += fread((void *)(mp3->buffer), 1, MP3_FILE_BUFFER_SIZE*2, mp3_file);

        fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);

        while(!fifoCheckValue32(FIFO_USER_01));

        ret = (int)fifoGetValue32(FIFO_USER_01);

        return ret;
}

int mp3_play(const char *filename, int loop, float loopsec){

        FILE *file = fopen(filename,"rb");

        return mp3_play_file(file, loop, loopsec);
}

int mp3_pause() {
        mp3_msg msg;

        if(mp3 == 0) {
                return 1;
        }

        msg.type = MP3_MSG_PAUSE;

        fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);
        while(!fifoCheckValue32(FIFO_USER_01));

        return (int)fifoGetValue32(FIFO_USER_01);
}
int mp3_stop() {
        mp3_msg msg;
        int ret;

        if(mp3 == 0) {
                return 1;
        }

        msg.type = MP3_MSG_STOP;

        fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);
        while(!fifoCheckValue32(FIFO_USER_01));

        ret = (int)fifoGetValue32(FIFO_USER_01);

        if(mp3_file) {
                fclose(mp3_file);
                mp3_file = 0;
        }

		MP3FreeDecoder(mp3->hMP3Decoder);
		mp3->hMP3Decoder = 0;

        return ret;
}
int mp3_resume() {
        mp3_msg msg;

        if(mp3 == 0) {
                return 1;
        }

        msg.type = MP3_MSG_RESUME;

        fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);
        while(!fifoCheckValue32(FIFO_USER_01));

        return (int)fifoGetValue32(FIFO_USER_01);
}
int mp3_set_volume(int volume) {
        mp3_msg msg;

        msg.type = MP3_MSG_VOLUME;
        msg.volume = volume;

        fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);
        while(!fifoCheckValue32(FIFO_USER_01));

        return (int)fifoGetValue32(FIFO_USER_01);
}

bool mp3_is_playing()
{
	return (mp3_file != 0);
}

int mp3_init() {
	mp3 = (mp3_player *)uncached_malloc(sizeof(mp3_player));
	mp3_buffer = (u8 *)uncached_malloc(MP3_FILE_BUFFER_SIZE*2);
	mp3_audioLeft = (u16 *)malloc(MP3_AUDIO_BUFFER_SIZE);
	mp3_audioRight = (u16 *)malloc(MP3_AUDIO_BUFFER_SIZE);
	if(mp3 == 0 || mp3_buffer == 0 || mp3_audioLeft == 0 || mp3_audioRight == 0) {
			if (mp3 == 0) iprintf("mp3 ");
			if (mp3_buffer == 0) iprintf("mp3_buffer ");
			if (mp3_audioLeft == 0) iprintf("mp3_audioLeft ");
			if (mp3_audioRight == 0) iprintf("mp3_audioRight ");
			mp3 = 0;
			mp3_buffer = 0;
			mp3_audioLeft = 0;
			mp3_audioRight = 0;
			iprintf("failed to allocate buffers\n");
			return 0;
	}

	memset((void *)mp3,0,sizeof(*mp3));
	memset((void *)mp3_buffer,0,MP3_FILE_BUFFER_SIZE*2);
	memset((void *)mp3_audioLeft,0,MP3_AUDIO_BUFFER_SIZE);
	memset((void *)mp3_audioRight,0,MP3_AUDIO_BUFFER_SIZE);

	return 1;
}

// WAV
volatile wav_stuffs* wav_load_handle(const char *filename, int* error_output)
{
	drwav wavfp;
	volatile wav_stuffs *wav2;

	if (!drwav_init_file(&wavfp, filename, NULL)) {
		*error_output = 1;
		return 0;
	}

	wav2 = (volatile wav_stuffs *)malloc(sizeof(wav_stuffs));

	u32* pSampleData = (u32*)malloc((u32)wavfp.totalPCMFrameCount * wavfp.channels * sizeof(u32));
	if (pSampleData == 0)
	{
		drwav_uninit(&wavfp);
		free((void*)wav2);
		*error_output = 2;
		return 0;
	}
	if (drwav_read_pcm_frames(&wavfp, wavfp.totalPCMFrameCount, pSampleData) == 0)
	{
		drwav_uninit(&wavfp);
		free((void*)wav2);
		free(pSampleData);
		*error_output = 3;
		return 0;
	}

	if (wavfp.bitsPerSample == 8) // 8 bit
	{
		u32* _8bitdata = (u32*)malloc((u32)wavfp.totalPCMFrameCount * wavfp.channels * sizeof(u32));
		drwav_u8_to_s16((drwav_int16*)_8bitdata, (drwav_uint8*)pSampleData, wavfp.totalPCMFrameCount);
		free(pSampleData);
		pSampleData = _8bitdata;
	}

	wav2->channels = wavfp.channels;
	wav2->samplerate = wavfp.sampleRate;
	wav2->length = wavfp.totalPCMFrameCount;
	wav2->data = pSampleData;
	wav2->volume = 127;
	wav2->state = WAV_IDLE;
	drwav_uninit(&wavfp);

	mp3_msg msg;
	msg.type = WAV_MSG_ADD;
	msg.wavplayer = wav2;

	fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);
	while(!fifoCheckValue32(FIFO_USER_01));
	u32 result = fifoGetValue32(FIFO_USER_01);

	if (result == 0) // no slots
	{
		drwav_uninit(&wavfp);
		free((void*)wav2);
		free(pSampleData);
		*error_output = 4;
		return 0;
	}

	*error_output = 0;
	return wav2;
}

void wav_free_handle(volatile wav_stuffs* wav, int* error_output)
{
	mp3_msg msg;
	msg.type = WAV_MSG_REMOVE;
	msg.slot = wav->slot;

	fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);
	while(!fifoCheckValue32(FIFO_USER_01));
	u32 result = fifoGetValue32(FIFO_USER_01);

	if (result == 0)
	{
		*error_output = 1;
		return;
	}

	*error_output = 0;
	if (wav->data) drwav_free(wav->data, NULL);
	if (wav) free((void*)wav);
	wav = NULL;
}

int wav_play_handle(volatile wav_stuffs* wav, int channel, bool loop)
{
	wav->channel = channel;
	wav->loop = loop;
	wav->state = WAV_STARTING;

	return 0;
}

int wav_unload(volatile wav_stuffs* wav)
{
	if (wav->state == WAV_PLAYING)
	{
		wav->state = WAV_STOPPING;
		wav->channel = wav->lastChannel;
	}
	return 0;
}

void wav_set_volume(volatile wav_stuffs* handle, u16 new_volume)
{
	handle->volume = (new_volume > 127) ? 127 : new_volume;
}

float wav_get_length(volatile wav_stuffs* handle)
{
	if (!handle) return 0;
	return (float)(handle->length) / handle->samplerate;
}

u8 wav_get_free_slot()
{
	mp3_msg msg;
	msg.type = WAV_MSG_GETSLOT;

	fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);
	while(!fifoCheckValue32(FIFO_USER_01));
	u8 result = fifoGetValue32(FIFO_USER_01);
	return result;
}

} // extern "C"