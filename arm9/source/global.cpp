#include "global.h"

#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>

#include <nds/arm9/cache.h>
#include <nds/arm9/decompress.h>
#include <nds/arm9/input.h>
#include <nds/interrupts.h>

#include "arm9_math_alt.h"
#include "mp3_shared.h"
#include "ui/label.h"

void debugPressA(const char* msg)
{
	iprintf("%s\n", msg);
	while (1)
	{
		mp3_fill_buffer();
		scanKeys();
		if (keysDown() & KEY_A) break;
		swiWaitForVBlank();
	}
}

void debugLabelPressA(const char* msg)
{
	UILabel* dbg = new UILabel(&oamMain, 120, 6, 1, RGB15(31,0,0), 15, 0);
	dbg->setPos(0,0);
	dbg->setText(msg);
	oamUpdate(&oamMain);

	while (1)
	{
		mp3_fill_buffer();
		scanKeys();
		if (keysDown() & KEY_A) break;
		swiWaitForVBlank();
	}

	delete dbg;
}

void AOdecode(std::string& s)
{
	std::string escapes[] = {"<and>", "<percent>", "<num>", "<dollar>"};
	std::string unescapes[] = {"&", "%", "#", "$"};
	for (int i=0; i<4; i++)
	{
		size_t pos = 0;
		while((pos = s.find(escapes[i], pos)) != std::string::npos)
		{
			mp3_fill_buffer();
			s.replace(pos, escapes[i].length(), unescapes[i]);
			pos += unescapes[i].length();
		}
		mp3_fill_buffer();
	}
}

void AOencode(std::string& s)
{
	std::string escapes[] = {"<and>", "<percent>", "<num>", "<dollar>"};
	std::string unescapes[] = {"&", "%", "#", "$"};
	for (int i=0; i<4; i++)
	{
		size_t pos = 0;
		while((pos = s.find(unescapes[i], pos)) != std::string::npos)
		{
			mp3_fill_buffer();
			s.replace(pos, unescapes[i].length(), escapes[i]);
			pos += escapes[i].length();
		}
		mp3_fill_buffer();
	}
}

std::string argumentAt(const std::string& s, int id, char delimiter)
{
	std::size_t lastPos = 0;
	std::size_t delimiterPos = s.find(delimiter);
	int i=0;

	while (lastPos != std::string::npos && i < id)
	{
		mp3_fill_buffer();

		i++;
		lastPos = delimiterPos;
		delimiterPos = s.find(delimiter, delimiterPos+1);
	}

	return s.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos - (id==0 ? 0 : 1));
}

void fillArguments(std::vector<std::string>& out, std::string& s, int id, char delimiter)
{
	std::size_t lastPos = 0;
	std::size_t delimiterPos = s.find(delimiter);
	int i=0;

	while (i < id)
	{
		i++;
		lastPos = delimiterPos;
		delimiterPos = s.find(delimiter, delimiterPos+1);
	}

	while (lastPos != std::string::npos)
	{
		out.push_back(s.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos - (lastPos==0 ? 0 : 1)));

		lastPos = delimiterPos;
		delimiterPos = s.find(delimiter, delimiterPos+1);
	}
}

u32 totalArguments(const std::string& s, char delimiter)
{
	std::size_t lastPos = 0;
	std::size_t delimiterPos = s.find(delimiter);
	u32 i=0;

	while (lastPos != std::string::npos)
	{
		i++;
		lastPos = delimiterPos;
		delimiterPos = s.find(delimiter, delimiterPos+1);
	}

	return i;
}

bool fileExists(const std::string& filename)
{
	struct stat buffer;
	bool exist = stat(filename.c_str(), &buffer) == 0;
	mp3_fill_buffer();
	return exist;
}

u8* readFile(const std::string& filename, u32* outLen, const char* mode)
{
	FILE* f = fopen(filename.c_str(), mode);
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
	if (!data)
	{
		fclose(f);
		if (outLen) *outLen = 0;
		return 0;
	}

	fread(data, len, 1, f);
	fclose(f);

	DC_FlushRange(data, len);

	mp3_fill_buffer();

	return data;
}

u32 bmpIndexTo256SpriteIndex(int x, int y, int w, int h, SpriteSize size, bool* oobFlag)
{
	// converts an array index from a bitmap (i.e. one created with malloc(x * y);
	// to an array index that can be used with 256-color sprites GFX created with oamAllocateGfx().

	if (x >= w || y >= h || x < 0 || y < 0)
	{
		if (oobFlag) *oobFlag = true;
		return 0;
	}

	int tilePixelX = mod32(x, 8);
	int tilePixelY = mod32(y, 8);
	int metaTileX = div32(x, 8);
	int metaTileY = div32(y, 8);

	if (oobFlag) *oobFlag = false;

	return div32((tilePixelY*8) + (metaTileX*64) + (metaTileY*64 * (swiDivide(w, 8))) + tilePixelX, 2);
	//gfx[targetInd] = (leftOrRight) ?
	//	(gfx[targetInd] & 0xf) | (1<<8) : // assign palette index right
	//	1 | ((gfx[targetInd] >> 8) << 8); // assign palette index left
}



FILE* streamFile = 0;
u8* streamData = 0;
u32 streamPos = 0;
u32 streamSize = 4096;

static int getHeader(uint8 *source, uint16 *dest, uint32 arg) {
	return *(uint32*)source;
}

static uint8 readByteFile(uint8 *source) {
	/*source -= streamPos;
	u8 thisByte = *source;
	if ((u32)(source-streamData) >= streamSize-1)
	{
		streamPos += streamSize;
		fread(streamData, streamSize, 1, streamFile);
	}
	return thisByte;*/
	mp3_fill_buffer();
	return *source;
}

TDecompressionStream decompressStreamCBs =
{
	getHeader,
	0,
	readByteFile
};

void readAndDecompressLZ77Stream(const char* filename, u8* dest)
{
	//streamPos = 0;
	//streamFile = fopen(filename, "rb");
	//streamData = new u8[streamSize];
	streamData = readFile(filename);
	fread(streamData, streamSize, 1, streamFile);

	swiDecompressLZSSVram(streamData, dest, 0, &decompressStreamCBs);

	//fclose(streamFile);
	delete[] streamData;
}
