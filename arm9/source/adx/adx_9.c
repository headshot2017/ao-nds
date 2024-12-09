// libadx-nds library
// by headshot2017 (Headshotnoby)
// LibADX Dreamcast library (c)2012 Josh PH3NOM Pearson
// Built on top of NDS Helix-MP3 decoder code by sverx (https://adshomebrewersdiary.blogspot.com/2012/06/mp3-streaming-on-arm7.html)

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <nds.h>

#include "libadx.h"

/////////////////////////////////////////////////////////////////////////////////////////

static FILE* adx_in;
static u8* adx_buffer;
static u16* adx_audioLeft;
static u16* adx_audioRight;
volatile adx_player* adx;

/////////////////////////////////////////////////////////////////////////////////////////

static int read_be16(unsigned char *buf)     /* ADX File Format is Big Endian */
{
	return (buf[0]<<8)|buf[1];
}

static long read_be32(unsigned char *buf)
{
	return (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
}

// Read and parse the ADX File header then seek to the begining sample offset
static int adx_parse( unsigned char *buf )
{
    fseek( adx_in, 0, SEEK_SET );          // Read the ADX Header into memory
	fread( buf, 1, ADX_HDR_SIZE, adx_in );
	if(buf[0]!=ADX_HDR_SIG ) return -1;           // Check ADX File Signature
	
    // Parse the ADX File header
	adx->ADX_Info.sample_offset = read_be16(buf+ADX_ADDR_START)-2;
	adx->ADX_Info.chunk_size    = buf[ADX_ADDR_CHUNK];
    adx->ADX_Info.channels      = buf[ADX_ADDR_CHAN];
	adx->ADX_Info.rate          = read_be32(buf+ADX_ADDR_RATE);
	adx->ADX_Info.samples       = read_be32(buf+ADX_ADDR_SAMP);
	adx->ADX_Info.loop_type     = buf[ADX_ADDR_TYPE]; 
	
	// Two known variations for possible loop informations: type 3 and type 4
    if( adx->ADX_Info.loop_type == 3 )
	    adx->ADX_Info.loop = read_be32(buf+ADX_ADDR_LOOP);
    else if( adx->ADX_Info.loop_type == 4 )
	    adx->ADX_Info.loop = read_be32(buf+ADX_ADDR_LOOP+0x0c);
    if( adx->ADX_Info.loop > 1 || adx->ADX_Info.loop < 0 )    // Invalid header check
        adx->ADX_Info.loop = 0;      
    if( adx->ADX_Info.loop && adx->ADX_Info.loop_type == 3 )
    {
        adx->ADX_Info.loop_samp_start = read_be32(buf+ADX_ADDR_SAMP_START);
        adx->ADX_Info.loop_start      = read_be32(buf+ADX_ADDR_BYTE_START);
        adx->ADX_Info.loop_samp_end   = read_be32(buf+ADX_ADDR_SAMP_END);
        adx->ADX_Info.loop_end        = read_be32(buf+ADX_ADDR_BYTE_END);
    }
    else if( adx->ADX_Info.loop && adx->ADX_Info.loop_type == 4  )
    {
        adx->ADX_Info.loop_samp_start = read_be32(buf+ADX_ADDR_SAMP_START+0x0c);
        adx->ADX_Info.loop_start      = read_be32(buf+ADX_ADDR_BYTE_START+0x0c);
        adx->ADX_Info.loop_samp_end   = read_be32(buf+ADX_ADDR_SAMP_END+0x0c);
        adx->ADX_Info.loop_end        = read_be32(buf+ADX_ADDR_BYTE_END+0x0c);
    }
    if( adx->ADX_Info.loop )
     adx->ADX_Info.loop_samples = adx->ADX_Info.loop_samp_end-adx->ADX_Info.loop_samp_start;
    
    fseek( adx_in, adx->ADX_Info.sample_offset, SEEK_SET ); // CRI File Signature
	fread( buf, 1, 6, adx_in );
       	
	if ( memcmp(buf, "(c)CRI", 6) )
		return -1;

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void *uncached_malloc(size_t count)
{
	void *p = malloc(count);
	return ((p == 0) ? 0 : memUncached(p));
}

int adx_init()
{
	adx_in = 0;
	adx = (adx_player*)uncached_malloc(sizeof(adx_player));
	adx_buffer = (u8*)uncached_malloc(ADX_FILE_BUFFER_SIZE*2);
	adx_audioLeft = (u16*)malloc(ADX_AUDIO_BUFFER_SIZE);
	adx_audioRight = (u16*)malloc(ADX_AUDIO_BUFFER_SIZE);
	if (!adx || !adx_buffer || !adx_audioLeft || !adx_audioRight)
		return 0;

	memset((void*)adx, 0, sizeof(adx_player));
	memset((void*)adx_buffer, 0, ADX_FILE_BUFFER_SIZE*2);
	memset((void*)adx_audioLeft, 0, ADX_AUDIO_BUFFER_SIZE);
	memset((void*)adx_audioRight, 0, ADX_AUDIO_BUFFER_SIZE);

	return 1;
}

void adx_update()
{
	if (!adx) return;

	int n;

	switch(adx->flag)
	{
		case 1: // stream more adx data
			// adx->buffer has two buffer chunks (ADX_FILE_BUFFER_SIZE*2)
			// read the data into the 2nd chunk
			n = fread((void *)(adx->buffer + ADX_FILE_BUFFER_SIZE), 1, ADX_FILE_BUFFER_SIZE, adx_in);

			if (adx->ADX_Info.loop && ftell( adx_in ) >= adx->ADX_Info.loop_end)
			{
				// handle sample-based looping
				fseek(adx_in, -n, SEEK_CUR);
				n = adx->ADX_Info.loop_end - ftell(adx_in);
				fread((void *)(adx->buffer + ADX_FILE_BUFFER_SIZE), 1, n, adx_in);

				// seek to specific sample position
				fseek(adx_in, adx->ADX_Info.loop_start, SEEK_SET);
				adx->ADX_Info.samples = adx->ADX_Info.loop_samples;

				fread((void *)(adx->buffer + ADX_FILE_BUFFER_SIZE + n), 1, ADX_FILE_BUFFER_SIZE-n, adx_in);
			}
			else if (n < ADX_FILE_BUFFER_SIZE)
			{
				if (adx->loop)
				{
					if (adx->ADX_Info.loop)
					{
						// start at specific sample position
						fseek(adx_in, adx->ADX_Info.loop_start, SEEK_SET);
						adx->ADX_Info.samples = adx->ADX_Info.loop_samples;
					}
					else
					{
						// start at beginning
						fseek(adx_in, adx->ADX_Info.sample_offset+ADX_CRI_SIZE, SEEK_SET);
					}

					n = fread((void *)(adx->buffer + ADX_FILE_BUFFER_SIZE + n), 1, ADX_FILE_BUFFER_SIZE-n, adx_in);
				}
			}
			adx->flag = 0;
			break;
	}
}

int adx_play(const char* adx_file, int loop_enable)
{
	if (!adx || adx_in) return 0;

	memset((void*)adx, 0, sizeof(adx_player));

	adx_in = fopen(adx_file, "rb");
	if (!adx_in)
        return 0;

	unsigned char adx_buf[ADX_HDR_SIZE];
    if (!adx_parse(adx_buf))
	{
        fclose(adx_in);
        return 0;
	}

	adx_msg msg;

	adx->buffer = adx_buffer;
	adx->audioLeft = adx_audioLeft;
	adx->audioRight = adx_audioRight;
	adx->loop = loop_enable;

	msg.type = ADX_MSG_START;
	msg.player = adx;

	fread((void *)(adx_buffer), 1, ADX_FILE_BUFFER_SIZE*2, adx_in);

	fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);

	int timeout = 60*3;
	while (!fifoCheckValue32(FIFO_USER_01) && --timeout)
		swiWaitForVBlank();

	int ret = (!timeout) ? -1 : (int)fifoGetValue32(FIFO_USER_01);

	return ret;
}

int adx_pause()
{
	if (!adx || !adx_in) return 0;

    adx_msg msg;
	msg.type = ADX_MSG_PAUSE;

	fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);

	return 1;
}

int adx_resume()
{
	if (!adx || !adx_in) return 0;

    adx_msg msg;
	msg.type = ADX_MSG_RESUME;

	fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);

	return 1;
}

int adx_set_volume(int volume)
{
	if (!adx || !adx_in) return 0;

	adx_msg msg;

	msg.type = ADX_MSG_VOLUME;
	msg.volume = volume;

	fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);
	while(!fifoCheckValue32(FIFO_USER_01));

	return (int)fifoGetValue32(FIFO_USER_01);
}

int adx_restart()
{
	if (!adx || !adx_in) return 0;
    
    adx_msg msg;
	msg.type = ADX_MSG_RESTART;

	fseek( adx_in, adx->ADX_Info.sample_offset+ADX_CRI_SIZE, SEEK_SET );
	fread((void *)(adx_buffer), 1, ADX_FILE_BUFFER_SIZE*2, adx_in);

	fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);

	return 1;
}

int adx_stop()
{
	if (!adx || !adx_in) return 1;

    adx_msg msg;

	msg.type = ADX_MSG_STOP;

	fifoSendDatamsg(FIFO_USER_01, sizeof(msg), (u8*)&msg);
	while(!fifoCheckValue32(FIFO_USER_01));

	int ret = (int)fifoGetValue32(FIFO_USER_01);

	fclose(adx_in);
	adx_in = 0;

	adx->flag = 0;

	return ret;
}
