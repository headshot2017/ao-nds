#include "global.h"

#include <stdio.h>

u8* readFile(const std::string& filename, u32* outLen)
{
	FILE* f = fopen(filename.c_str(), "rb");
	if (!f)
	{
		if (outLen) *outLen = 0;
		return 0;
	}

	fseek(f, 0, SEEK_END);
	u32 len = ftell(f);
	if (outLen) *outLen = len;
	fseek(f, 0, SEEK_SET);

	u8* data = new u8[len];
	fread(data, len, 1, f);
	fclose(f);

	return data;
}

u32 bmpIndexTo256SpriteIndex(u32 x, u32 y, u32 w, u32 h, SpriteSize size, bool* oobFlag)
{
	// converts an array index from a bitmap (i.e. one created with malloc(x * y);
	// to an array index that can be used with 256-color sprites GFX created with oamAllocateGfx().

	if (x >= w || y >= h || x < 0 || y < 0)
	{
		if (oobFlag) *oobFlag = true;
		return 0;
	}

	u32 tilePixelX = x%8;
	u32 tilePixelY = y%8;
	u32 metaTileX = x/8;
	u32 metaTileY = y/8;

	if (oobFlag) *oobFlag = false;

	return ((tilePixelY*8) + (metaTileX*64) + (metaTileY*64 * (w/8)) + tilePixelX) / 2;
	//gfx[targetInd] = (leftOrRight) ?
	//	(gfx[targetInd] & 0xf) | (1<<8) : // assign palette index right
	//	1 | ((gfx[targetInd] >> 8) << 8); // assign palette index left
}
