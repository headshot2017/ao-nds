#ifdef __cplusplus
extern "C" {
#endif

#ifndef __wav_nds_h__
#define __wav_nds_h__

struct _wav_handle
{
	s16* data;
	u32 size;
	u32 samprate;
};
typedef struct _wav_handle wav_handle;

wav_handle* wav_load_handle(const char *filename);
void wav_free_handle(wav_handle* handle);
int wav_play(wav_handle* handle);

#endif // __wav_nds_h__

#ifdef __cplusplus
}
#endif
