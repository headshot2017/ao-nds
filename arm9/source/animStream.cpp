#include "animStream.h"

#include "libadx.h"

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
		delete[] data;
		data = 0;
		adx_update();
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
	adx_update();
	data = new u8[frameSize];
	adx_update();
}

u8* animStream::getFrame(int i)
{
	int frameOffset = i*frameSize;
	fseek(f, frameOffset, SEEK_SET);
	adx_update();
	fread(data, frameSize, 1, f);
	adx_update();
	return data;
}
