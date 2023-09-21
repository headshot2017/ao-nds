#include "global.h"

#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>

#include <nds/arm9/input.h>
#include <nds/interrupts.h>

#include "mp3_shared.h"

void debugPressA(const char* msg)
{
	iprintf("%s\n", msg);
	while (1)
	{
		scanKeys();
		if (keysDown() & KEY_A) break;
		swiWaitForVBlank();
	}
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

std::string argumentAt(const std::string& s, int id)
{
	std::size_t lastPos = 0;
	std::size_t delimiterPos = s.find("#");
	int i=0;

	while (lastPos != std::string::npos && i < id)
	{
		i++;
		lastPos = delimiterPos;
		delimiterPos = s.find("#", delimiterPos+1);
	}

	return s.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos - (id==0 ? 0 : 1));
}

void fillArguments(std::vector<std::string>& out, std::string& s, int id)
{
	std::size_t lastPos = 0;
	std::size_t delimiterPos = s.find("#");
	int i=0;

	while (i < id)
	{
		i++;
		lastPos = delimiterPos;
		delimiterPos = s.find("#", delimiterPos+1);
	}

	i = 0;
	while (lastPos != std::string::npos)
	{
		out.push_back(s.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1));

		lastPos = delimiterPos;
		delimiterPos = s.find("#", delimiterPos+1);
	}
}

u32 totalArguments(const std::string& s)
{
	std::size_t lastPos = 0;
	std::size_t delimiterPos = s.find("#");
	u32 i=0;

	while (lastPos != std::string::npos)
	{
		i++;
		lastPos = delimiterPos;
		delimiterPos = s.find("#", delimiterPos+1);
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
	if (!data)
	{
		fclose(f);
		if (outLen) *outLen = 0;
		return 0;
	}

	fread(data, len, 1, f);
	fclose(f);

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

	int tilePixelX = x%8;
	int tilePixelY = y%8;
	int metaTileX = x/8;
	int metaTileY = y/8;

	if (oobFlag) *oobFlag = false;

	return ((tilePixelY*8) + (metaTileX*64) + (metaTileY*64 * (w/8)) + tilePixelX) / 2;
	//gfx[targetInd] = (leftOrRight) ?
	//	(gfx[targetInd] & 0xf) | (1<<8) : // assign palette index right
	//	1 | ((gfx[targetInd] >> 8) << 8); // assign palette index left
}
