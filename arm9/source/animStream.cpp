#include "animStream.h"
#include "mem.h"

animStream::~animStream()
{
	unload();
}

void animStream::unload()
{
	if (f)
	{
		fclose(f);
		f = 0;
		mem_free(data);
		data = 0;
	}
}

void animStream::loadFile(const char* filename, int tW, int tH, int sW, int sH)
{
	unload();

	frameSize = tW*tH*sW*sH;
	tileW = tW;
	tileH = tH;
	sprW = sW;
	sprH = sH;

	f = fopen(filename, "rb");
	data = (u8*)mem_alloc(frameSize);
}

u8* animStream::getFrame(int i)
{
	if (!f || !data)
		return 0;

	int frameOffset = i*frameSize;
	fseek(f, frameOffset, SEEK_SET);
	fread(data, frameSize, 1, f);
	return data;
}
