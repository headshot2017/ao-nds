#include "courtroom/character.h"

#include <math.h>
#include <stdio.h>

#include <nds/dma.h>
#include <nds/timers.h>
#include <nds/interrupts.h>
#include <nds/arm9/input.h>
#include <nds/arm9/decompress.h>

#include "global.h"
#include "mp3_shared.h"

//the speed of the timer when using ClockDivider_1024
#define TIMER_SPEED (BUS_CLOCK/1024)

void readTwoValues(const std::string& value, int* w, int* h)
{
    std::size_t delimiterPos = value.find(",");
    if (delimiterPos == std::string::npos)
    {
        *w = 0;
        *h = 0;
        return;
    }

    *w = std::stoi(value.substr(0, delimiterPos));
    *h = std::stoi(value.substr(delimiterPos + 1));
    mp3_fill_buffer();
}

void readFrameDurations(const std::string& value, std::vector<u32>& output)
{
	std::size_t lastPos = 0;
	std::size_t delimiterPos = value.find(",");

	output.clear();
	while (lastPos != std::string::npos)
	{
		mp3_fill_buffer();

		u32 dur = std::stoi(value.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1));
		output.push_back(dur);

		lastPos = delimiterPos;
		delimiterPos = value.find(",", delimiterPos+1);
	}
}

void readFrameIndexes(const std::string& value, std::vector<u16>& output)
{
	std::size_t lastPos = 0;
	std::size_t delimiterPos = value.find(",");

	output.clear();
	u32 frameCount = 0;
	while (lastPos != std::string::npos)
	{
		mp3_fill_buffer();

		frameCount++;
		std::string val = value.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos - (frameCount==1 ? 0 : 1));
		u16 ind = std::stoi(val);
		output.push_back(ind);

		lastPos = delimiterPos;
		delimiterPos = value.find(",", delimiterPos+1);
	}
}


Character::Character()
{
	timerTicks = 0;
	currFrame = 0;
	loop = false;
	gfxInUse = 0;

	charData = 0;
	frameInfo.frameCount = 0;

	for (int i=0; i<8*6; i++)
	{
		int x = (i%8) * 32;
		int y = (i/8) * 32;

		charGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
		charGfxVisible[i] = false;
		oamSet(&oamMain, 50+i, x, y, 2, 2, SpriteSize_32x32, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
	}

	setVisible(false);
	setOnAnimFinishedCallback(0, 0);
}

Character::~Character()
{
	for (int i=0; i<8*6; i++)
	{
		oamSet(&oamMain, 50+i, 0, 0, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
		oamFreeGfx(&oamMain, charGfx[i]);
	}

	if (charData)
		delete[] charData;

	timerStop(0);
}

void Character::setCharImage(std::string charname, std::string relativeFile, bool doLoop)
{
	std::string NDScfg = "/data/ao-nds/characters/" + charname + "/nds.cfg";
	std::string IMGbin = "/data/ao-nds/characters/" + charname + "/" + relativeFile + ".img.bin";
	std::string PALbin = "/data/ao-nds/characters/" + charname + "/" + relativeFile + ".pal.bin";
	if (!fileExists(NDScfg) || !fileExists(IMGbin) || !fileExists(PALbin))
	{
		// show missingno/placeholder
		charname = "placeholder";
		relativeFile = "placeholder";
		NDScfg = "/data/ao-nds/misc/nds.cfg";
		IMGbin = "/data/ao-nds/misc/placeholder.img.bin";
		PALbin = "/data/ao-nds/misc/placeholder.pal.bin";
		if (!fileExists(NDScfg) || !fileExists(IMGbin) || !fileExists(PALbin))
			return;
	}

	timerStop(0);

	if (charData)
	{
		delete[] charData;
		charData = 0;
	}

	// load gfx and palette
	u32 palSize, charSize;

	u8* charDataLZ77 = readFile(IMGbin.c_str(), &charSize);
	u8* charPalette = readFile(PALbin.c_str(), &palSize);
	if (!charDataLZ77 || !charPalette)
	{
		if (charDataLZ77) delete[] charDataLZ77;
		if (charPalette) delete[] charPalette;
		return;
	}

	currAnim = relativeFile;
	if (currCharacter != charname)
	{
		currCharacter = charname;
		animInfos.load(NDScfg);
	}

	for(auto& c : relativeFile) c = tolower(c);
	int oldW = frameInfo.frameW;
	int oldH = frameInfo.frameH;
	int oldOffsetX = frameInfo.offsetX;
	int oldOffsetY = frameInfo.offsetY;

	readTwoValues(animInfos.get(relativeFile + "_size"), &frameInfo.frameW, &frameInfo.frameH);
	readTwoValues(animInfos.get(relativeFile + "_offset"), &frameInfo.offsetX, &frameInfo.offsetY);
	if (frameInfo.frameW == 0 && frameInfo.frameH == 0)
	{
		frameInfo.frameW = oldW;
		frameInfo.frameH = oldH;
		frameInfo.offsetX = oldOffsetX;
		frameInfo.offsetY = oldOffsetY;
		delete[] charDataLZ77;
		delete[] charPalette;
		return;
	}
	readFrameDurations(animInfos.get(relativeFile + "_durations"), frameInfo.frameDurations);
	readFrameIndexes(animInfos.get(relativeFile + "_indexes"), frameInfo.frameIndexes);
	frameInfo.frameCount = frameInfo.frameIndexes.size();
	mp3_fill_buffer();

	frameInfo.realW = ceil(frameInfo.frameW/32.f);
	int realH = ceil(frameInfo.frameH/32.f);

	// decompress gfx and copy palette to slot 2
	charData = new u8[frameInfo.realW*32 * realH*32 * frameInfo.frameCount];
	if (!charData)
	{
		delete[] charDataLZ77;
		delete[] charPalette;
		return;
	}

	decompress(charDataLZ77, charData, LZ77);
	mp3_fill_buffer();

	vramSetBankF(VRAM_F_LCD);
	dmaCopy(charPalette, &VRAM_F_EXT_SPR_PALETTE[2], palSize);
	vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

	// free data from memory
	delete[] charPalette;
	delete[] charDataLZ77;

	for (int i=0; i<gfxInUse; i++)
	{
		oamSet(&oamMain, 50+i, 0, 0, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
		charGfxVisible[i] = false;
	}

	gfxInUse = frameInfo.realW*realH;

	for (int i=0; i<gfxInUse; i++)
	{
		int x = (i%frameInfo.realW) * 32;
		int y = (i/frameInfo.realW) * 32;

		u8* offset = charData + i*32*32;
		dmaCopy(offset, charGfx[i], 32*32);

		oamSet(&oamMain, 50+i, x+frameInfo.offsetX, y+frameInfo.offsetY, 2, 2, SpriteSize_32x32, SpriteColorFormat_256Color, charGfx[i], -1, false, false, false, false, false);
		charGfxVisible[i] = true;
	}

	loop = doLoop;
	timerTicks = 0;
	timerStart(0, ClockDivider_1024, 0, NULL);
	currFrame = 0;
}

void Character::setVisible(bool on)
{
	visible = on;
}

void Character::update()
{
	for (int i=0; i<gfxInUse; i++)
	{
		int x = (i%frameInfo.realW) * 32;
		int y = (i/frameInfo.realW) * 32;
		oamSetXY(&oamMain, 50+i, x+frameInfo.offsetX + shakeX, y+frameInfo.offsetY + shakeY);
	}

	if (!loop && currFrame >= frameInfo.frameCount)
		return;

	timerTicks += timerElapsed(0);
	u32 ms = (float)timerTicks/TIMER_SPEED*1000;

	if (charData && frameInfo.frameCount && ms >= frameInfo.frameDurations[currFrame])
	{
		timerTicks = 0;
		timerPause(0);

		if (loop)
			currFrame = (currFrame+1) % (frameInfo.frameCount);
		else
		{
			currFrame++;
			if (currFrame >= frameInfo.frameCount)
			{
				timerStop(0);
				if (onAnimFinished) onAnimFinished(pUserData);
			}
		}

		// copy new frame to sprite gfx
		int frameOffset = frameInfo.frameIndexes[currFrame]*gfxInUse;
		for (int i=0; i<gfxInUse; i++)
		{
			u8* offset = charData + frameOffset*32*32 + i*32*32;
			dmaCopy(offset, charGfx[i], 32*32);
		}

		timerUnpause(0);
	}
}
