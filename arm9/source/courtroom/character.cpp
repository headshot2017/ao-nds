#include "courtroom/character.h"

#include <math.h>

#include <nds/dma.h>
#include <nds/timers.h>
#include <nds/interrupts.h>
#include <nds/arm9/input.h>
#include <nds/arm9/decompress.h>

#include "global.h"
#include "mp3_shared.h"

//the speed of the timer when using ClockDivider_1024
#define TIMER_SPEED (BUS_CLOCK/1024)

void readFrameSize(const std::string& value, int* w, int* h)
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
}

u32* readFrameDurations(const std::string& value, u32* sizeOutput)
{
	std::size_t lastPos = 0;
	std::size_t delimiterPos = value.find(",");

	u32 frameCount = 0;
	u32* output = 0;
	while (lastPos != std::string::npos)
	{
		frameCount++;
		output = (u32*)realloc(output, sizeof(u32) * frameCount);
		u32 dur = std::stoi(value.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1));
		output[frameCount-1] = dur;

		lastPos = delimiterPos;
		delimiterPos = value.find(",", delimiterPos+1);
	}

	*sizeOutput = frameCount;
	return output;
}


Character::Character()
{
	timerTicks = 0;
	currFrame = 0;
	realW = 0;
	frameW = 0;
	frameH = 0;
	loop = false;
	gfxInUse = 0;

	charData = 0;
	frameDurations = 0;
	frameCount = 0;

	for (int i=0; i<8*6; i++)
	{
		int x = (i%8) * 32;
		int y = (i/8) * 32;

		//charGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
		charGfx[i] = 0;
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
		if (charGfx[i]) oamFreeGfx(&oamMain, charGfx[i]);
	}

	timerStop(0);
}

void Character::setCharImage(std::string charname, std::string relativeFile, bool doLoop)
{
	std::string NDScfg = "/data/ao-nds/characters/" + charname + "/nds.cfg";
	std::string IMGbin = "/data/ao-nds/characters/" + charname + "/" + relativeFile + ".img.bin";
	std::string PALbin = "/data/ao-nds/characters/" + charname + "/" + relativeFile + ".pal.bin";
	if (!fileExists(NDScfg) || !fileExists(IMGbin) || !fileExists(PALbin))
		return;

	timerStop(0);

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
	int oldW = frameW;
	int oldH = frameH;

	readFrameSize(animInfos.get(relativeFile + "_size"), &frameW, &frameH);
	if (frameW == 0 && frameH == 0)
	{
		frameW = oldW;
		frameH = oldH;
		delete[] charDataLZ77;
		delete[] charPalette;
		return;
	}
	free(frameDurations);
	frameDurations = readFrameDurations(animInfos.get(relativeFile + "_durations"), &frameCount);
	mp3_fill_buffer();

	realW = ceil(frameW/32.f);
	int realH = ceil(frameH/32.f);

	if (charData)
	{
		delete[] charData;
		charData = 0;
	}

	// decompress gfx and copy palette to slot 2
	charData = new u8[realW*32 * realH*32 * frameCount];
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
		oamFreeGfx(&oamMain, charGfx[i]);
		charGfx[i] = 0;
	}

	gfxInUse = realW*realH;

	for (int i=0; i<gfxInUse; i++)
	{
		int x = (i%realW) * 32;
		int y = (i/realW) * 32;

		charGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);

		u8* offset = charData + i*32*32;
		dmaCopy(offset, charGfx[i], 32*32);

		oamSet(&oamMain, 50+i, x+128-(frameW/2), y+192-frameH, 2, 2, SpriteSize_32x32, SpriteColorFormat_256Color, charGfx[i], -1, false, false, false, false, false);
		charGfxVisible[i] = true;
	}

	loop = doLoop;
	timerTicks = 0;
	timerStart(0, ClockDivider_1024, 0, NULL);
}

void Character::setVisible(bool on)
{
	visible = on;
}

void Character::update()
{
	if (!loop && currFrame >= frameCount)
		return;

	timerTicks += timerElapsed(0);
	u32 ms = (float)timerTicks/TIMER_SPEED*1000;

	for (int i=0; i<gfxInUse; i++)
	{
		int x = (i%realW) * 32;
		int y = (i/realW) * 32;
		oamSetXY(&oamMain, 50+i, x+128-(frameW/2) + xOffset, y+192-frameH + yOffset);
	}

	if (charData && frameCount && ms >= frameDurations[currFrame])
	{
		timerTicks = 0;
		timerPause(0);

		if (loop)
			currFrame = (currFrame+1) % (frameCount);
		else
		{
			currFrame++;
			if (currFrame >= frameCount)
			{
				timerStop(0);
				if (onAnimFinished) onAnimFinished(pUserData);
			}
		}

		// copy new frame to sprite gfx
		for (int i=0; i<gfxInUse; i++)
		{
			int frameOffset = currFrame*gfxInUse;
			u8* offset = charData + frameOffset*32*32 + i*32*32;
			dmaCopy(offset, charGfx[i], 32*32);
		}

		timerUnpause(0);
	}
}
