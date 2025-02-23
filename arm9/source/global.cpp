#include "global.h"

#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>

#include <nds/arm9/cache.h>
#include <nds/arm9/input.h>
#include <nds/decompress.h>
#include <nds/interrupts.h>
#include <nds/cothread.h>

#include "mem.h"
#include "utf8.h"
#include "arm9_math_alt.h"
#include "libadx.h"
#include "ui/label.h"

void debugPressA(const char* msg)
{
	printf("%s\n", msg);
	while (1)
	{
		scanKeys();
		if (keysDown() & KEY_A) break;
		cothread_yield_irq(IRQ_VBLANK);
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
		scanKeys();
		if (keysDown() & KEY_A) break;
		cothread_yield_irq(IRQ_VBLANK);
	}

	delete dbg;
}


std::string escapes[] = {"<and>", "<percent>", "<num>", "<dollar>"};
std::string unescapes[] = {"&", "%", "#", "$"};
void AOdecode(std::string& s)
{
	for (int i=0; i<4; i++)
	{
		size_t pos = 0;
		while((pos = s.find(escapes[i], pos)) != std::string::npos)
		{
			adx_update();
			s.replace(pos, escapes[i].length(), unescapes[i]);
			pos += unescapes[i].length();
		}
		adx_update();
	}
}

void AOdecode(std::u16string& s)
{
	for (int i=0; i<4; i++)
	{
		std::u16string escape16 = utf8::utf8to16(escapes[i]);
		size_t pos = 0;
		while((pos = s.find(escape16, pos)) != std::u16string::npos)
		{
			adx_update();
			s.replace(pos, escape16.length(), utf8::utf8to16(unescapes[i]));
			pos += unescapes[i].length();
		}
		adx_update();
	}
}

void AOencode(std::string& s)
{
	for (int i=0; i<4; i++)
	{
		size_t pos = 0;
		while((pos = s.find(unescapes[i], pos)) != std::string::npos)
		{
			adx_update();
			s.replace(pos, unescapes[i].length(), escapes[i]);
			pos += escapes[i].length();
		}
		adx_update();
	}
}

std::string argumentAt(const std::string& s, int id, char delimiter)
{
	std::size_t lastPos = 0;
	std::size_t delimiterPos = s.find(delimiter);
	int i=0;

	while (lastPos != std::string::npos && i < id)
	{
		adx_update();

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
	adx_update();
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

	u8* data = (u8*)mem_alloc(len);
	if (!data)
	{
		fclose(f);
		if (outLen) *outLen = 0;
		return 0;
	}

	fread(data, len, 1, f);
	fclose(f);

	DC_FlushRange(data, len);

	adx_update();

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



static int getHeader(uint8 *source, uint16 *dest, uint32 arg) {
	return *(uint32*)source;
}
static uint8 readByteFile(uint8 *source);
TDecompressionStream decompressStreamCBs =
{
	getHeader,
	0,
	readByteFile
};

#ifdef LZ77_STREAM
static FILE* streamFile = 0;
static u8* streamData = 0;
static u32 streamPos = 0;
static u32 streamSize = 4096;

static uint8 readByteFile(uint8 *source) {
	source -= streamPos;
	u8 thisByte = *source;
	if ((u32)(source-streamData) >= streamSize-1)
	{
		streamPos += streamSize;
		fread(streamData, streamSize, 1, streamFile);
	}
	adx_update();
	return thisByte;
}

void readAndDecompressLZ77Stream(const char* filename, u8* dest)
{
	streamPos = 0;
	streamFile = fopen(filename, "rb");
	streamData = (u8*)mem_alloc(streamSize);
	fread(streamData, streamSize, 1, streamFile);

	swiDecompressLZSSVram(streamData, dest, 0, &decompressStreamCBs);

	fclose(streamFile);
	mem_free(streamData);
}
#else
static uint8 readByteFile(uint8 *source) {
	adx_update();
	return *source;
}

void readAndDecompressLZ77Stream(const char* filename, u8* dest)
{
	u8* streamData = readFile(filename);
	swiDecompressLZSSVram(streamData, dest, 0, &decompressStreamCBs);
	mem_free(streamData);
}
#endif
