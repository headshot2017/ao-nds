#include "courtroom/character.h"

#include <math.h>
#include <stdio.h>
#include <algorithm>

#include <nds/decompress.h>
#include <nds/dma.h>
#include <nds/timers.h>
#include <nds/interrupts.h>
#include <nds/arm9/input.h>

#include "arm9_math_alt.h"
#include "courtroom/courtroom.h"
#include "global.h"
#include "content.h"
#include "libadx.h"

//the speed of the timer when using ClockDivider_1024
#define TIMER_SPEED div32(BUS_CLOCK,1024)

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
    adx_update();
}

void readFrameDurations(const std::string& value, std::vector<u32>& output)
{
	std::size_t lastPos = 0;
	std::size_t delimiterPos = value.find(",");

	output.clear();
	while (lastPos != std::string::npos)
	{
		adx_update();

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
		adx_update();

		frameCount++;
		std::string val = value.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos - (frameCount==1 ? 0 : 1));
		u16 ind = std::stoi(val);
		output.push_back(ind);

		lastPos = delimiterPos;
		delimiterPos = value.find(",", delimiterPos+1);
	}
}


Character::Character(Courtroom* pCourt, int start, int isPair)
{
	m_pCourt = pCourt;
	oamStart = start;
	pair = isPair;

	camOffset = 0;
	charTicks = 0;
	sfxTicks = 0;
	currFrame = 0;
	loop = false;
	gfxInUse = 0;

	charData = 0;
	frameInfo.frameCount = 0;

	sfx = 0;
	sfxPlayed = true;
	sfxDelay = 0;

	for (int i=0; i<4*3; i++)
	{
		int x = (i%4) * 64;
		int y = (i/4) * 64;

		charGfx[i] = 0;
		oamSet(&oamMain, oamStart+i, x, y, 2, 2+pair, SpriteSize_64x64, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
	}

	setShakes(0, 0);
	setOffsets(0, 0);
	setFlip(false);

	setVisible(false);
	setOnAnimFinishedCallback(0, 0);
}

Character::~Character()
{
	for (int i=0; i<4*3; i++)
	{
		oamSet(&oamMain, oamStart+i, 0, 0, 0, 0, SpriteSize_64x64, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
		if (charGfx[i]) oamFreeGfx(&oamMain, charGfx[i]);
	}

	if (charData) delete[] charData;
	if (sfx) wav_free_handle(sfx);
	clearFrameData();

	timerStop(pair);
}

void Character::clearFrameData()
{
	for (auto const& cached : cachedFrameSFX)
		wav_free_handle(cached.second); // second is wav_handle*

	cachedFrameSFX.clear();
	frameSFX.clear();
	frameFlash.clear();
	frameShake.clear();
}

void Character::setCharImage(std::string charname, std::string relativeFile, bool doLoop)
{
	std::string contentPath = Content::getFile("characters/" + charname);
	std::string NDScfg = contentPath + "/nds.cfg";
	std::string IMGbin = contentPath + "/" + relativeFile + ".img.bin";
	std::string PALbin = contentPath + "/" + relativeFile + ".pal.bin";
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

	timerStop(pair);

	if (charData)
	{
		delete[] charData;
		charData = 0;
	}

	// load palette
	u32 palSize;
	u8* charPalette = readFile(PALbin.c_str(), &palSize);
	if (!charPalette)
	{
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
		delete[] charPalette;
		return;
	}
	readFrameDurations(animInfos.get(relativeFile + "_durations"), frameInfo.frameDurations);
	readFrameIndexes(animInfos.get(relativeFile + "_indexes"), frameInfo.frameIndexes);
	frameInfo.frameCount = frameInfo.frameIndexes.size();
	u32 gfxCount = std::stoi(animInfos.get(relativeFile + "_framegfxcount"));

	if (frameInfo.frameCount == 1 && !frameInfo.frameDurations[0])
		frameInfo.frameDurations[0] = 100;

	adx_update();

	frameInfo.realW = ceil(frameInfo.frameW/64.f);
	frameInfo.realH = ceil(frameInfo.frameH/64.f);

	frameInfo.streaming = (animInfos.get(relativeFile + "_stream") == "1");
	if (!frameInfo.streaming)
	{
		// decompress gfx and copy palette to slot 2
		stream.unload();

		charData = new u8[frameInfo.realW*64 * frameInfo.realH*64 * gfxCount];
		adx_update();

		if (!charData)
		{
			delete[] charPalette;
			return;
		}

		readAndDecompressLZ77Stream(IMGbin.c_str(), charData);
	}
	else
	{
		stream.loadFile(IMGbin.c_str(), frameInfo.realW, frameInfo.realH, 64, 64);
	}
	adx_update();

	clearFrameData();

	for (int i=0; i<4*3; i++)
	{
		oamSet(&oamMain, oamStart+i, 0, 0, 0, 0, SpriteSize_64x64, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
		if (charGfx[i])
		{
			oamFreeGfx(&oamMain, charGfx[i]);
			charGfx[i] = 0;
		}
	}

	gfxInUse = frameInfo.realW*frameInfo.realH;
	u8* ptr = (!frameInfo.streaming) ? charData : stream.getFrame(0);

	for (int i=0; i<gfxInUse; i++)
	{
		charGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_256Color);
		if (!charGfx[i])
		{
			oamSetHidden(&oamMain, oamStart+i, true);
			continue;
		}

		int x = ((flip) ?
			((gfxInUse-i-1) % frameInfo.realW)*64 + 256-(frameInfo.realW*64)-frameInfo.offsetX :
			(i%frameInfo.realW)*64 + frameInfo.offsetX)
			+ shakeX + offsetX;

		int y = (i/frameInfo.realW)*64 + frameInfo.offsetY + shakeY + offsetY;

		if (y <= -64 || y >= 192)
		{
			oamSetHidden(&oamMain, oamStart+i, true);
			continue;
		}

		u8* offset = ptr + i*64*64;
		dmaCopy(offset, charGfx[i], 64*64);
		adx_update();

		oamSet(&oamMain, oamStart+i, x, y, 2, 2+pair, SpriteSize_64x64, SpriteColorFormat_256Color, charGfx[i], -1, false, false, flip, false, false);
	}

	vramSetBankF(VRAM_F_LCD);
	dmaCopy(charPalette, &VRAM_F_EXT_SPR_PALETTE[2+pair], palSize);
	vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	delete[] charPalette;

	oamUpdate(&oamMain);
	loop = doLoop;
	charTicks = 0;
	currFrame = 0;
}

void Character::setSound(const std::string& filename, int delay)
{
	unloadSound();

	sfx = wav_load_handle(Content::getFile(filename).c_str());
	sfxPlayed = false;
	sfxTicks = 0;
	sfxDelay = delay * TIME_MOD;
}

void Character::setFrameSFX(const std::string& data)
{
	/// (a)franziska-mad|12=sfx-deskslam|16=sfx-deskslam|4=sfx-deskslam
	u32 args = std::count(data.begin(), data.end(), '|');

	for (u32 i=1; i<=args; i++)
	{
		std::string arg = argumentAt(data, i, '|');

		int frame = std::stoi(argumentAt(arg, 0, '='));
		std::string sfx = argumentAt(arg, 1, '=');

		frameSFX[frame] = sfx;

		if (cachedFrameSFX.count(sfx)) continue;
		std::string filename = Content::getFile("sounds/general/" + sfx + ".wav");
		wav_handle* handle = wav_load_handle(filename.c_str());
		cachedFrameSFX[sfx] = handle;
	}
}

void Character::setFrameFlash(const std::string& data)
{
	u32 args = std::count(data.begin(), data.end(), '|');

	for (u32 i=1; i<=args; i++)
	{
		std::string arg = argumentAt(data, i, '|');
		int frame = std::stoi(argumentAt(arg, 0, '='));

		frameFlash.insert(frame);
	}
}

void Character::setFrameShake(const std::string& data)
{
	u32 args = std::count(data.begin(), data.end(), '|');

	for (u32 i=1; i<=args; i++)
	{
		std::string arg = argumentAt(data, i, '|');
		int frame = std::stoi(argumentAt(arg, 0, '='));

		frameShake.insert(frame);
	}
}

void Character::setVisible(bool on)
{
	visible = on;
	for (int i=0; i<gfxInUse; i++)
		oamSetHidden(&oamMain, oamStart+i, !on);
}

void Character::unload()
{
	if (charData)
	{
		delete[] charData;
		charData = 0;
	}
	if (sfx)
	{
		wav_free_handle(sfx);
		sfx = 0;
	}
	frameInfo.frameDurations.clear();
	frameInfo.frameIndexes.clear();
	stream.unload();

	for (int i=0; i<gfxInUse; i++)
	{
		oamSetHidden(&oamMain, oamStart+i, true);
		if (charGfx[i])
		{
			oamFreeGfx(&oamMain, charGfx[i]);
			charGfx[i] = 0;
		}
	}
	gfxInUse = 0;
}

void Character::unloadSound()
{
	if (sfx)
	{
		wav_free_handle(sfx);
		sfx = 0;
	}
}

void Character::triggerFrameData()
{
	if (frameSFX.count(currFrame))
		wav_play(cachedFrameSFX[frameSFX[currFrame]]);
	if (frameShake.count(currFrame))
		m_pCourt->shake(5, 15);
	if (frameFlash.count(currFrame))
		m_pCourt->flash(3);
}

void Character::update()
{
	for (int i=0; i<gfxInUse; i++)
	{
		int x = ((flip) ?
			((gfxInUse-i-1) % frameInfo.realW)*64 + 256-(frameInfo.realW*64)-frameInfo.offsetX :
			(i%frameInfo.realW)*64 + frameInfo.offsetX)
			+ shakeX + offsetX + camOffset;

		int y = (i/frameInfo.realW)*64 + frameInfo.offsetY + shakeY + offsetY;

		if (y <= -64 || y >= 192)
		{
			oamSetHidden(&oamMain, oamStart+i, true);
			continue;
		}

		oamSetXY(&oamMain, oamStart+i, x, y);
	}

	if (!loop && currFrame >= frameInfo.frameCount)
		return;

	if (!(TIMER_CR(pair) & TIMER_ENABLE))
		timerStart(pair, ClockDivider_1024, 0, NULL);

	u32 elapsed = timerElapsed(pair);
	u32 ms = f32toint(mulf32(divf32(inttof32(elapsed), inttof32(TIMER_SPEED)), inttof32(1000)));

	if (!sfxPlayed && sfx)
	{
		sfxTicks += ms;
		if (sfxTicks >= sfxDelay)
		{
			wav_play(sfx);
			sfxPlayed = true;
		}
	}

	if (!frameInfo.frameCount) return;
	charTicks += ms;

	while (charTicks >= frameInfo.frameDurations[currFrame])
	{
		charTicks -= frameInfo.frameDurations[currFrame];

		if (loop)
			currFrame = (currFrame+1) % (frameInfo.frameCount);
		else
		{
			currFrame++;

			if (currFrame >= frameInfo.frameCount)
			{
				if (onAnimFinished) onAnimFinished(pUserData);
				return;
			}
		}

		triggerFrameData();

		// copy new frame to sprite gfx
		u8* ptr;
		if (!frameInfo.streaming)
		{
			int frameOffset = frameInfo.frameIndexes[currFrame]*gfxInUse;
			ptr = charData + frameOffset*64*64;
		}
		else
			ptr = stream.getFrame(frameInfo.frameIndexes[currFrame]);

		for (int i=0; i<gfxInUse; i++)
		{
			u8* offset = ptr + i*64*64;
			dmaCopy(offset, charGfx[i], 64*64);
			adx_update();
		}
	}
}
