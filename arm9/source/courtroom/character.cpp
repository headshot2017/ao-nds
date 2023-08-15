#include "courtroom/character.h"

#include <math.h>

#include <nds/dma.h>
#include <nds/timers.h>

#include "global.h"

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

void readFrameDurations(const std::string& value, std::vector<u32>& output)
{
	output.clear();
	std::size_t lastPos = 0;
	std::size_t delimiterPos = value.find(",");

	while (lastPos != std::string::npos)
	{
		u32 dur = std::stoi(value.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1));
		output.push_back(dur);

		lastPos = delimiterPos;
		delimiterPos = value.find(",", delimiterPos+1);
	}
}


Character::Character()
{
	timerTicks = 0;
	currFrame = 0;
	frameW = 0;
	frameH = 0;
	gfxInUse = 0;

	charData = 0;

	for (int i=0; i<8*6; i++)
	{
		int x = (i%8) * 32;
		int y = (i/8) * 32;

		charGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x32, SpriteColorFormat_256Color);
		charGfxVisible[i] = false;
		oamSet(&oamMain, 50+i, x, y, 2, 2, SpriteSize_32x32, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
	}

	setVisible(false);
}

Character::~Character()
{
	for (int i=0; i<8*6; i++)
	{
		oamSet(&oamMain, 50+i, 0, 0, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
		oamFreeGfx(&oamMain, charGfx[i]);
	}

	timerStop(0);
}

void Character::setCharImage(std::string charname, std::string relativeFile)
{
	std::string NDScfg = "/data/ao-nds/characters/" + charname + "/nds.cfg";
	std::string IMGbin = "/data/ao-nds/characters/" + charname + "/" + relativeFile + ".img.bin";
	std::string PALbin = "/data/ao-nds/characters/" + charname + "/" + relativeFile + ".pal.bin";
	if (!fileExists(NDScfg) || !fileExists(IMGbin) || !fileExists(PALbin))
		return;

	u32 palSize;
	charData = readFile(IMGbin.c_str(), &charSize);
	u8* charPalette = readFile(PALbin.c_str(), &palSize);
	if (currCharacter != charname)
	{
		currCharacter = charname;
		animInfos.load(NDScfg);
	}

	vramSetBankF(VRAM_F_LCD);
	dmaCopy(charPalette, &VRAM_F_EXT_SPR_PALETTE[2], palSize);
	vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	delete[] charPalette;

	readFrameSize(animInfos.get(relativeFile + "_size"), &frameW, &frameH);
	readFrameDurations(animInfos.get(relativeFile + "_durations"), frameDurations);

	for (int i=0; i<gfxInUse; i++)
	{
		oamSet(&oamMain, 50+i, 0, 0, 0, 0, SpriteSize_32x32, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
		charGfxVisible[i] = false;
	}

	int realW = ceil(frameW/32.f);
	int realH = ceil(frameH/32.f);
	gfxInUse = realW*realH;

	for (int i=0; i<gfxInUse; i++)
	{
		int x = (i%(realW)) * 32;
		int y = (i/(realW)) * 32;

		int animFrame = 0*realW*realH;
		u8* offset = charData + animFrame*32*32 + i * 32*32;
		dmaCopy(offset, charGfx[i], 32*32);

		oamSet(&oamMain, 50+i, x+128-(frameW/2), y+192-frameH, 2, 2, SpriteSize_32x32, SpriteColorFormat_256Color, charGfx[i], -1, false, false, false, false, false);
		charGfxVisible[i] = true;
	}

	timerTicks = 0;
	timerStop(0);
	timerStart(0, ClockDivider_1024, 0, NULL);
}

void Character::setVisible(bool on)
{
	visible = on;
}

void Character::update()
{
	timerTicks += timerElapsed(0);
	u32 ms = (float)timerTicks/TIMER_SPEED*1000;

	if (ms >= frameDurations[currFrame])
	{
		currFrame = (currFrame+1) % (frameDurations.size());
		timerTicks = 0;
		timerPause(0);

		for (int i=0; i<gfxInUse; i++)
		{
			int frameOffset = currFrame*gfxInUse;
			u8* offset = charData + frameOffset*32*32 + i * 32*32;
			dmaCopy(offset, charGfx[i], 32*32);
		}

		timerUnpause(0);
	}
}
