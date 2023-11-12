#include <string>
#include <stdio.h>

#include <nds.h>
#include <nds/arm9/cache.h>
#include <nds/arm9/sound.h>
#include <nds/fifocommon.h>
#include <nds/fifomessages.h>

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

#include "mp3_shared.h"
#include "mp3dec.h"

extern "C" {

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


void *uncached_malloc(size_t count) {
        void *p = malloc(count);
        return ((p == 0) ? 0 : memUncached(p));
}

void mp3_fill_buffer() {
        int n;

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
						mp3->flag = 0;
						break;

					case 2: // ARM7 reaches this state if it gets stuck in mp3_frames(). give it a new buffer to read from
						n = ftell(mp3_file) - (MP3_FILE_BUFFER_SIZE*2);
						if (n < 0) n = 0;
						fseek(mp3_file, n, SEEK_SET);

						memset((void *)mp3_buffer,0,MP3_FILE_BUFFER_SIZE*2);

						n = fread((void *)(mp3_buffer), 1, MP3_FILE_BUFFER_SIZE*2, mp3_file);
						filled += n;
						if(n < MP3_FILE_BUFFER_SIZE*2) {
								if (mp3->loop)
								{
										float bytes;
										if (!mp3_length) mp3_length = (float)mp3->soundtime/(float)mp3->rate;
										bytes = (1.f / mp3_length) * mp3->filesize;

										fseek (mp3_file, (int)(mp3_loopsec * bytes) / MP3_FILE_BUFFER_SIZE * MP3_FILE_BUFFER_SIZE, SEEK_SET);
										n = fread((void *)(mp3_buffer + MP3_FILE_BUFFER_SIZE + n), 1, MP3_FILE_BUFFER_SIZE-n, mp3_file);
										filled += n;
								}
						}
						mp3->flag = 3;
						break;

					case 4: // non-looping mp3 reached the end
						mp3_stop();
						break;
				}
        }
        //iprintf("out\n");
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

        filled = fread((void *)(mp3_buffer), 1, MP3_FILE_BUFFER_SIZE*2, mp3_file);

        fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);

        while(!fifoCheckValue32(FIFO_USER_01));

        ret = (int)fifoGetValue32(FIFO_USER_01);

        return ret;
}

int mp3_play(const char *filename, int loop, float loopsec){

        FILE *file = fopen(filename, "rb");
        if (!file) return -1;

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

		if(mp3_file)
		{
				fclose(mp3_file);
				mp3_file = 0;
		}

		mp3->flag = 0;

		if (mp3->hMP3Decoder)
		{
			MP3FreeDecoder(mp3->hMP3Decoder);
			mp3->hMP3Decoder = 0;
		}

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
wav_handle* wav_load_handle(const char *filename)
{
	drwav wavfp;

	if (!drwav_init_file(&wavfp, filename, NULL))
	{
		return 0;
	}

	u32* pSampleData = new u32[(u32)wavfp.totalPCMFrameCount * wavfp.channels];
	if (pSampleData == 0)
	{
		drwav_uninit(&wavfp);
		return 0;
	}
	u32 totalRead = drwav_read_pcm_frames(&wavfp, wavfp.totalPCMFrameCount, pSampleData);
	if (!totalRead)
	{
		drwav_uninit(&wavfp);
		delete[] pSampleData;
		return 0;
	}

	if (wavfp.bitsPerSample == 8) // 8 bit
	{
		u32* _8bitdata = new u32[(u32)wavfp.totalPCMFrameCount * wavfp.channels];
		drwav_u8_to_s16((drwav_int16*)_8bitdata, (drwav_uint8*)pSampleData, wavfp.totalPCMFrameCount);
		delete[] pSampleData;
		pSampleData = _8bitdata;
	}

	wav_handle* handle = new wav_handle;
	handle->data = pSampleData;
	handle->size = totalRead*2;
	handle->samprate = wavfp.sampleRate;
	drwav_uninit(&wavfp);

	return handle;
}

void wav_free_handle(wav_handle* handle)
{
	if (!handle || !handle->data) return;

	delete[] handle->data;
	handle->data = 0;

	delete[] handle;
}

int wav_play(wav_handle* handle)
{
	if (!handle || !handle->data) return -1;

	return soundPlaySample(handle->data, SoundFormat_16Bit, handle->size, handle->samprate, 127, 64, false, 0);
}

} // extern "C"
