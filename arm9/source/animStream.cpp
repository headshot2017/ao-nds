#include "animStream.h"

#include "mp3_shared.h"

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
		mp3_fill_buffer();
	}
}

void animStream::loadFile(const char* filename, int tW, int tH, int sW, int sH)
{
	unload();

	currFrame = 0;
	frameSize = tW*tH*sW*sH;
	tileW = tW;
	tileH = tH;
	sprW = sW;
	sprH = sH;

	f = fopen(filename, "rb");
	mp3_fill_buffer();
	data = new u8[frameSize];
	mp3_fill_buffer();
}

u8* animStream::getFrame(int i)
{
	if (currFrame == i)
	{
		currFrame++;
	}
	else
	{
		int frameOffset = i*frameSize;
		fseek(f, frameOffset, SEEK_SET);
		mp3_fill_buffer();
	}
	fread(data, frameSize, 1, f);
	mp3_fill_buffer();
	return data;
}
