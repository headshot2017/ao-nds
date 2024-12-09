// libadx-nds library
// by headshot2017 (Headshotnoby)
// LibADX Dreamcast library (c)2012 Josh PH3NOM Pearson
// Built on top of NDS Helix-MP3 decoder code by sverx (https://adshomebrewersdiary.blogspot.com/2012/06/mp3-streaming-on-arm7.html)

#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "libadx.h"

int getFreeChannel();

volatile adx_player *adx;
volatile adx_player_state adx_state;
static int adx_channelLeft;
static int adx_channelRight;
static int adx_volume;
static int pcm_size;
static u8* adx_readPtr;

static DSTIME ds_sound_start;
static DSTIME soundtime;
static DSTIME paintedtime;


/* Convert ADX samples to PCM samples */
static void adx_to_pcm(short *out, unsigned char *in, volatile PREV *prev)
{
	int scale = ((in[0]<<8)|(in[1]));
	int i;
	int s0,s1,s2,d;

	in+=2;
	s1 = prev->s1;
	s2 = prev->s2;
	for(i=0;i<16;i++) {
		d = in[i]>>4;
		if (d&8) d-=16;
		s0 = (BASEVOL*d*scale + 0x7298*s1 - 0x3350*s2)>>14;
		if (s0>32767) s0=32767;
		else if (s0<-32768) s0=-32768;
		*out++=s0;
		s2 = s1;
		s1 = s0;

		d = in[i]&15;
		if (d&8) d-=16;
		s0 = (BASEVOL*d*scale + 0x7298*s1 - 0x3350*s2)>>14;
		if (s0>32767) s0=32767;
		else if (s0<-32768) s0=-32768;
		*out++=s0;
		s2 = s1;
		s1 = s0;
	}
	prev->s1 = s1;
	prev->s2 = s2;

}

static DSTIME ds_time()
{
	u16 time1 = TIMER1_DATA;
	u32 time2 = TIMER2_DATA;

	return (time2 << 16) + time1;
}

static void ds_set_timer(int rate)
{
	if (rate == 0)
	{
		TIMER_CR(0) = 0;
		TIMER_CR(1) = 0;
		TIMER_CR(2) = 0;
	}
	else
	{
		TIMER_DATA(0) = 0x10000 - (0x1000000 / rate) * 2;
		TIMER_CR(0) = TIMER_ENABLE | TIMER_DIV_1;
		TIMER_DATA(1) = 0;
		TIMER_CR(1) = TIMER_ENABLE | TIMER_CASCADE | TIMER_DIV_1;
		TIMER_DATA(2) = 0;
		TIMER_CR(2) = TIMER_ENABLE | TIMER_CASCADE | TIMER_DIV_1;
	}
}

static DSTIME ds_sample_pos()
{
	DSTIME v;

	v = (ds_time() - ds_sound_start);

	return v;
}



static int adx_frame()
{
	// Do not decode any frames if the decoder
	// is asking for more data from the file.
	if (adx->flag == 1)
		return 0;

	unsigned char *left = (unsigned char*)adx->audioLeft;
	unsigned char *right = (unsigned char*)adx->audioRight;
	unsigned char adx_buf[ADX_HDR_SIZE];
	short outbuf[32*2];
	int wsize;
	int pcm_samples = adx->ADX_Info.samples;

	if (adx->ADX_Info.channels==1)
	{
		if (pcm_samples)
		{
			// If there is room in PCM buffer, decode next chunk of ADX samples
			if (pcm_size < ADX_AUDIO_BUFFER_SIZE)
			{
				// Read the current chunk
				memcpy(adx_buf, adx_readPtr, adx->ADX_Info.chunk_size);
				adx_readPtr += adx->ADX_Info.chunk_size;

				// Convert ADX chunk to PCM
				adx_to_pcm(outbuf, adx_buf, adx->prev);
				if (pcm_samples>32) wsize=32; else wsize = pcm_samples;
				pcm_samples -= wsize;

				// Copy the deocded samples to sample buffer
				memcpy(left+pcm_size, outbuf, wsize*2);
				pcm_size += wsize*2;
				paintedtime += wsize;
			}
			else
				pcm_size = 0;

			return 1;
		}
	}
	else if (adx->ADX_Info.channels==2)
	{
		if (pcm_samples)
		{
			// If there is room in PCM buffer, decode next chunk of ADX samples
			if (pcm_size < ADX_AUDIO_BUFFER_SIZE)
			{
				memcpy(adx_buf, adx_readPtr, adx->ADX_Info.chunk_size*2);
				adx_readPtr += adx->ADX_Info.chunk_size*2;

				adx_to_pcm(outbuf, adx_buf, adx->prev);
				adx_to_pcm(outbuf+32, adx_buf+adx->ADX_Info.chunk_size, adx->prev+1);
				if (pcm_samples>32) wsize=32; else wsize = pcm_samples;
				pcm_samples -= wsize;

				memcpy(left+pcm_size, outbuf, wsize*2);
				memcpy(right+pcm_size, outbuf+32, wsize*2);
				pcm_size += wsize*2;
				paintedtime += wsize;
			}
			else
				pcm_size = 0;

			return 1;
		}
	}

	return 0;
}

static void adx_frames(DSTIME endtime, u8 firstFrames)
{
	while (paintedtime < endtime)
	{
		if (firstFrames && adx->flag == 1)
			break;

		adx_frame();

		if (adx->flag == 4) break; // non-looping music ended, stop here

		// check if we moved onto the 2nd file data buffer, if so move it to the 1st one and request a refill
		if (adx_readPtr > (adx->buffer +  ADX_FILE_BUFFER_SIZE + (ADX_FILE_BUFFER_SIZE>>1)))
		{
			adx_readPtr -= ADX_FILE_BUFFER_SIZE;
			memcpy((void *)adx_readPtr, (void *)(adx_readPtr + ADX_FILE_BUFFER_SIZE), ADX_FILE_BUFFER_SIZE - (adx_readPtr-adx->buffer));
			adx->flag = 1;
		}
	}
}

static int adx_playing()
{
	DSTIME endtime;

	soundtime = ds_sample_pos();

	// check to make sure that we haven't overshot
	if (paintedtime < soundtime)
		paintedtime = soundtime;

	// mix ahead of current position
	endtime = soundtime + (adx->ADX_Info.rate>>4);

	adx_frames(endtime, 0);
	return 1;
}

static void adx_pause()
{
	if (adx == 0 || adx_channelLeft == -1)
		return;

	adx->flag = 0;
	ds_set_timer(0);
	SCHANNEL_CR(adx_channelLeft) = 0;
	SCHANNEL_CR(adx_channelRight) = 0;
	adx_channelLeft = -1;
	adx_state = ADX_PAUSED;
}

static int adx_resume()
{
	if (adx == 0 || adx_channelLeft != -1)
		return 0;

	adx->flag = 0;
	paintedtime = 0;
	pcm_size = 0;
	memset((void *)adx->audioLeft, 0, ADX_AUDIO_BUFFER_SIZE);
	memset((void *)adx->audioRight, 0, ADX_AUDIO_BUFFER_SIZE);
	adx_frames(ADX_AUDIO_BUFFER_SAMPS>>1, 1);

	adx_channelLeft = getFreeChannel();

	SCHANNEL_SOURCE(adx_channelLeft) = (u32)adx->audioLeft;
	SCHANNEL_REPEAT_POINT(adx_channelLeft) = 0;
	SCHANNEL_LENGTH(adx_channelLeft) = (ADX_AUDIO_BUFFER_SIZE)>>2;
	SCHANNEL_TIMER(adx_channelLeft) = 0x10000 - (0x1000000 / adx->ADX_Info.rate);

	if (adx->ADX_Info.channels == 1)
	{
		// Mono
		SCHANNEL_CR(adx_channelLeft) = SCHANNEL_ENABLE | SOUND_VOL(adx_volume) | SOUND_PAN(64) | (SOUND_FORMAT_16BIT) | (SOUND_REPEAT);
	}
	else
	{
		// Stereo
		// "lock" (silent) this channel, so that next getFreeChannel call gives a different one...
		SCHANNEL_CR(adx_channelLeft) = SCHANNEL_ENABLE | SOUND_VOL(0) | SOUND_PAN(0) | (SOUND_FORMAT_16BIT) | (SOUND_REPEAT);
		adx_channelRight = getFreeChannel();
		SCHANNEL_CR(adx_channelLeft) = 0;

		SCHANNEL_SOURCE(adx_channelRight) = (u32)adx->audioRight;
		SCHANNEL_REPEAT_POINT(adx_channelRight) = 0;
		SCHANNEL_LENGTH(adx_channelRight) = (ADX_AUDIO_BUFFER_SIZE)>>2;
		SCHANNEL_TIMER(adx_channelRight) = 0x10000 - (0x1000000 / adx->ADX_Info.rate);

		SCHANNEL_CR(adx_channelLeft) = SCHANNEL_ENABLE | SOUND_VOL(adx_volume) | SOUND_PAN(0) | (SOUND_FORMAT_16BIT) | (SOUND_REPEAT);
		SCHANNEL_CR(adx_channelRight) = SCHANNEL_ENABLE | SOUND_VOL(adx_volume) | SOUND_PAN(127) | (SOUND_FORMAT_16BIT) | (SOUND_REPEAT);
	}

	ds_set_timer(adx->ADX_Info.rate);
	ds_sound_start = ds_time();

	adx_state = ADX_PLAYING;
	return 1;
}

void adx_stop()
{
	if (!adx) return;
	adx = 0;

	ds_set_timer(0);
	SCHANNEL_CR(adx_channelLeft) = 0;
	SCHANNEL_CR(adx_channelRight) = 0;
	//free((void *)&SCHANNEL_SOURCE(adx_channelLeft));
	//free((void *)&SCHANNEL_SOURCE(adx_channelRight));
	SCHANNEL_SOURCE(adx_channelLeft) = 0;
	SCHANNEL_SOURCE(adx_channelRight) = 0;
	adx_channelLeft = -1;
	adx_state = ADX_IDLE;
}

static void adx_starting()
{
	adx_readPtr = adx->buffer;

	adx_resume();
	fifoSendValue32(FIFO_USER_01, 1);
}

static void adx_resuming()
{
	if (adx == 0)
	{
		adx_state = ADX_IDLE;
		return;
	}

	adx_resume();
}

static void adx_pausing()
{
	if (adx == 0)
	{
		adx_state = ADX_IDLE;
		return;
	}

	adx_pause();
}

static void adx_stopping()
{
	if (adx == 0)
	{
		adx_state = ADX_IDLE;
		fifoSendValue32(FIFO_USER_01, 0);
		return;
	}

	adx_stop();
	fifoSendValue32(FIFO_USER_01, 1);
}

static void adx_restarting()
{
	if (adx == 0)
	{
		adx_state = ADX_IDLE;
		return;
	}

	adx_pause();

	adx_readPtr = adx->buffer;

	int i;
	for (i=0; i<2; i++)
	{
		adx->prev[i].s1 = 0;
		adx->prev[i].s2 = 0;
	}

	adx_resume();
}

static void adx_set_volume(int volume)
{
	// clamp
	volume =
		(volume < 0) ? 0 :
		(volume > 127) ? 127 :
		volume;

	adx_volume = volume;

	if (adx_channelLeft == -1)
	{
		fifoSendValue32(FIFO_USER_01, 0);
		return;
	}

	SCHANNEL_CR(adx_channelLeft) = (SCHANNEL_CR(adx_channelLeft) & ~0xFF) | volume;
	if (adx->ADX_Info.channels == 2)
		SCHANNEL_CR(adx_channelRight) = (SCHANNEL_CR(adx_channelRight) & ~0xFF) | volume;

	fifoSendValue32(FIFO_USER_01, 1);
}

void adx_update()
{
	switch(adx_state)
	{
		case ADX_STARTING:
			adx_starting();
			break;
		case ADX_PLAYING:
			adx_playing();
			break;
		case ADX_PAUSING:
			adx_pausing();
			break;
		case ADX_RESUMING:
			adx_resuming();
			break;
		case ADX_STOPPING:
			adx_stopping();
			break;
		case ADX_RESTARTING:
			adx_restarting();
			break;
		case ADX_PAUSED:
		case ADX_IDLE:
		case ADX_ERROR:
			break;
	}
}

void adx_DataHandler(int bytes, void *user_data)
{
	adx_msg msg;

	fifoGetDatamsg(FIFO_USER_01, bytes, (u8*)&msg);

	switch(msg.type)
	{
		case ADX_MSG_START:
			adx = msg.player;
			adx_state = ADX_STARTING;
			break;

		case ADX_MSG_PAUSE:
			adx_state = ADX_PAUSING;
			break;

		case ADX_MSG_RESUME:
			adx_state = ADX_RESUMING;
			break;

		case ADX_MSG_STOP:
			adx_state = ADX_STOPPING;
			break;

		case ADX_MSG_RESTART:
			adx_state = ADX_RESTARTING;
			break;

		case ADX_MSG_VOLUME:
			adx_set_volume(msg.volume);
			break;
	}
}

int adx_init()
{
	adx = 0;
	adx_state = ADX_IDLE;
	adx_channelLeft = -1;
	adx_volume = 127;

	fifoSetDatamsgHandler(FIFO_USER_01, adx_DataHandler, 0);

	return 1;
}
