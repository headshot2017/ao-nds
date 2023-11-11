#include "courtroom/shout.h"

#include <string.h>

#include <nds/arm9/background.h>
#include <nds/arm9/sound.h>

#include "courtroom/courtroom.h"
#include "mp3_shared.h"

Shout::Shout(Courtroom* pCourt)
{
	m_pCourt = pCourt;

	bgIndex = bgInit(2, BgType_Text4bpp, BgSize_T_512x256, 5, 6);
	bgHide(bgIndex);
	bgMapLen = 0;
	visible = false;

	xOffset = 0;
	yOffset = 0;

	sndShout = 0;
	sndShoutSize = 0;

	onShoutFinished = 0;
	pUserData = 0;

	ticks = -1;
}

Shout::~Shout()
{
	if (bgMapLen) dmaFillHalfWords(0, bgGetMapPtr(bgIndex), bgMapLen);
	bgHide(bgIndex);
	freeSound();
}

void Shout::setShout(const std::string& charname, int shoutMod, const std::string& custom)
{
	ticks = 1;
	freeSound();

	u32 bgGfxLen, bgPalLen;
	u8* bgGfx = 0;
	u8* bgMap = 0;
	u8* bgPal = 0;

	std::string sndFile = "/data/ao-nds/characters/" + charname;
	switch(shoutMod)
	{
		case 1:
			bgGfx = readFile("/data/ao-nds/misc/holdit_bubble.img.bin", &bgGfxLen);
			bgMap = readFile("/data/ao-nds/misc/holdit_bubble.map.bin", &bgMapLen);
			bgPal = readFile("/data/ao-nds/misc/holdit_bubble.pal.bin", &bgPalLen);
			sndFile += "/holdit.wav";
			break;

		case 2:
			bgGfx = readFile("/data/ao-nds/misc/objection_bubble.img.bin", &bgGfxLen);
			bgMap = readFile("/data/ao-nds/misc/objection_bubble.map.bin", &bgMapLen);
			bgPal = readFile("/data/ao-nds/misc/objection_bubble.pal.bin", &bgPalLen);
			sndFile += "/objection.wav";
			break;

		case 3:
			bgGfx = readFile("/data/ao-nds/misc/takethat_bubble.img.bin", &bgGfxLen);
			bgMap = readFile("/data/ao-nds/misc/takethat_bubble.map.bin", &bgMapLen);
			bgPal = readFile("/data/ao-nds/misc/takethat_bubble.pal.bin", &bgPalLen);
			sndFile += "/takethat.wav";
			break;

		case 4:
			bgGfx = readFile("/data/ao-nds/characters/" + charname + "/" + custom + ".img.bin", &bgGfxLen);
			bgMap = readFile("/data/ao-nds/characters/" + charname + "/" + custom + ".map.bin", &bgMapLen);
			bgPal = readFile("/data/ao-nds/characters/" + charname + "/" + custom + ".pal.bin", &bgPalLen);
			sndFile += "/" + custom + ".wav";
			break;
	}
	sndShout = wav_load_handle(sndFile.c_str(), &sndShoutSize);
	if (!sndShout)
		sndShout = wav_load_handle("/data/ao-nds/sounds/general/sfx-objection.wav", &sndShoutSize);

	bgHide(bgIndex);

	m_pCourt->flash(3);
	m_pCourt->getChatbox()->setVisible(false);

	if (bgGfx && bgMap && bgPal)
	{
		dmaCopy(bgGfx, bgGetGfxPtr(bgIndex), bgGfxLen);
		memcpy(bgGetMapPtr(bgIndex), bgMap, bgMapLen);
		memcpy(BG_PALETTE, bgPal, bgPalLen);
		BG_PALETTE[0] = 0;

		delete[] bgGfx;
		delete[] bgMap;
		delete[] bgPal;
	}
	else
	{
		if (bgGfx) delete[] bgGfx;
		if (bgMap) delete[] bgMap;
		if (bgPal) delete[] bgPal;
	}

	mp3_fill_buffer();
}

void Shout::setVisible(bool on)
{
	visible = on;
	(on && ticks != -1) ? bgShow(bgIndex) : bgHide(bgIndex);
}

void Shout::freeSound()
{
	if (sndShout)
	{
		delete[] sndShout;
		sndShout = 0;
	}
}

void Shout::cancelShout()
{
	ticks = -1;
	if (visible) bgHide(bgIndex);
	if (bgMapLen) dmaFillHalfWords(0, bgGetMapPtr(bgIndex), bgMapLen);
}

void Shout::update()
{
	bool shake = (ticks > 5 && ticks <= 40);
	setOffsets(
		(shake) ? -6 + rand()%12 : 0,
		(shake) ? -6 + rand()%12 : 0
	);
	bgSetScroll(bgIndex, -xOffset, -yOffset);

	if (ticks <= 0) return;

	ticks++;
	if (ticks == 5)
	{
		REG_BLDCNT = BLEND_NONE;
		REG_BLDY = 0;
	}
	else if (ticks == 6)
	{
		if (sndShout)
			soundPlaySample(sndShout, SoundFormat_16Bit, sndShoutSize, 32000, 127, 64, false, 0);
		if (visible) bgShow(bgIndex);
	}
	else if (ticks > 75)
	{
		cancelShout();
		if (onShoutFinished)
			onShoutFinished(pUserData);
	}
}
