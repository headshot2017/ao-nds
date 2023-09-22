#ifndef ANIMSTREAM_H_INCLUDED
#define ANIMSTREAM_H_INCLUDED

#include <stdio.h>

#include <nds/ndstypes.h>

class animStream
{
	FILE* f;
	u8* data;

	int frameSize;
	int tileW;
	int tileH;
	int sprW;
	int sprH;

public:
	animStream() : f(0), data(0) {}
	~animStream();

	void unload();
	void loadFile(const char* filename, int tW, int tH, int sW, int sH);
	u8* getFrame(int i);
};

#endif // ANIMSTREAM_H_INCLUDED
