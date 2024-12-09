#ifndef LIBADX_H
#define LIBADX_H

#ifdef __cplusplus
extern "C" {
#endif

#define ADX_CRI_SIZE 0x06
#define ADX_PAD_SIZE 0x0e
#define ADX_HDR_SIZE 0x2c
#define ADX_HDR_SIG  0x80
#define ADX_EXIT_SIG 0x8001

#define ADX_ADDR_START      0x02
#define ADX_ADDR_CHUNK      0x05
#define ADX_ADDR_CHAN       0x07
#define ADX_ADDR_RATE       0x08
#define ADX_ADDR_SAMP       0x0c
#define ADX_ADDR_TYPE       0x12
#define ADX_ADDR_LOOP       0x18
#define ADX_ADDR_SAMP_START 0x1c
#define ADX_ADDR_BYTE_START 0x20
#define ADX_ADDR_SAMP_END   0x24
#define ADX_ADDR_BYTE_END   0x28

#define	BASEVOL	0x4000

#define ADX_FILE_BUFFER_SIZE (8 * 1024)
#define ADX_AUDIO_BUFFER_SAMPS (8 * 1024)
#define ADX_AUDIO_BUFFER_SIZE (ADX_AUDIO_BUFFER_SAMPS * 2)

typedef vu32 DSTIME;

typedef struct
{
    vs32 sample_offset;              
    vs32 chunk_size;
    vs32 channels;
    vs32 rate;
    vs32 samples;
    vs32 loop_type;
    vs32 loop;
    vs32 loop_start;
    vs32 loop_end;
    vs32 loop_samp_start;
    vs32 loop_samp_end;
    vs32 loop_samples;
}ADX_INFO;

typedef struct {
	vs32 s1,s2;
} PREV;

typedef enum {
        ADX_IDLE,
        ADX_STARTING,
        ADX_PLAYING,
        ADX_PAUSING,
        ADX_RESUMING,
        ADX_PAUSED,
        ADX_STOPPING,
        ADX_RESTARTING,
        ADX_ERROR=0xffffffff
} adx_player_state;

typedef struct {
	ADX_INFO ADX_Info;
	vs32 loop;
	PREV prev[2];

	u8* buffer;
	u16* audioLeft;
    u16* audioRight;
	vu32 flag;
} adx_player;

typedef struct {
	u32 type;
	union
	{
		volatile adx_player *player;
		u32	volume;
	};
} adx_msg;

enum {
        ADX_MSG_START,
        ADX_MSG_STOP,
        ADX_MSG_PAUSE,
        ADX_MSG_RESUME,
        ADX_MSG_VOLUME,
        ADX_MSG_RESTART,
        ADX_MSG_ERROR=0xffffffff,
};


// LibADX Public Function Definitions
// Return 1 on success, 0 on failure

// Initialize the library
int adx_init();

// Call this every frame
void adx_update();

#ifdef ARM9

// Start streaming the ADX in ARM7
int adx_play(const char * adx_file, int loop_enable);

// Pause ADX stream
int adx_pause();

// Resume a paused ADX stream
int adx_resume();

// Set the ADX volume from range 0-127
int adx_set_volume(int volume);

// Restart the ADX from the beginning
int adx_restart();

// Stop ADX stream
int adx_stop();

#endif // ARM9

#ifdef __cplusplus
};
#endif

#endif // LIBADX_H
