#include "courtroom/shout.h"

#include <nds/arm9/background.h>

#include "courtroom/courtroom.h"
#include "content.h"
#include "libadx.h"
#include "mem.h"

Shout::Shout(Courtroom* pCourt)
{
	m_pCourt = pCourt;

	bgIndex = bgInit(2, BgType_Text4bpp, BgSize_T_512x256, 4, 6);
	bgHide(bgIndex);
	bgMapLen = 0;
	visible = false;

	xOffset = 0;
	yOffset = 0;

	sndShout = 0;

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

	std::string contentPath = Content::getFile("characters/" + charname);
	std::string& sndFile = contentPath;
	switch(shoutMod)
	{
		case 1:
			if (fileExists(contentPath + "/holdit_bubble.img.bin"))
			{
				bgGfx = readFile(contentPath + "/holdit_bubble.img.bin", &bgGfxLen);
				bgMap = readFile(contentPath + "/holdit_bubble.map.bin", &bgMapLen);
				bgPal = readFile(contentPath + "/holdit_bubble.pal.bin", &bgPalLen);
			}
			else
			{
				bgGfx = readFile("/data/ao-nds/misc/holdit_bubble.img.bin", &bgGfxLen);
				bgMap = readFile("/data/ao-nds/misc/holdit_bubble.map.bin", &bgMapLen);
				bgPal = readFile("/data/ao-nds/misc/holdit_bubble.pal.bin", &bgPalLen);
			}
			sndFile += "/holdit.wav";
			break;

		case 2:
			if (fileExists(contentPath + "/objection_bubble.img.bin"))
			{
				bgGfx = readFile(contentPath + "/objection_bubble.img.bin", &bgGfxLen);
				bgMap = readFile(contentPath + "/objection_bubble.map.bin", &bgMapLen);
				bgPal = readFile(contentPath + "/objection_bubble.pal.bin", &bgPalLen);
			}
			else
			{
				bgGfx = readFile("/data/ao-nds/misc/objection_bubble.img.bin", &bgGfxLen);
				bgMap = readFile("/data/ao-nds/misc/objection_bubble.map.bin", &bgMapLen);
				bgPal = readFile("/data/ao-nds/misc/objection_bubble.pal.bin", &bgPalLen);
			}
			sndFile += "/objection.wav";
			break;

		case 3:
			if (fileExists(contentPath + "/takethat_bubble.img.bin"))
			{
				bgGfx = readFile(contentPath + "/takethat_bubble.img.bin", &bgGfxLen);
				bgMap = readFile(contentPath + "/takethat_bubble.map.bin", &bgMapLen);
				bgPal = readFile(contentPath + "/takethat_bubble.pal.bin", &bgPalLen);
			}
			else
			{
				bgGfx = readFile("/data/ao-nds/misc/takethat_bubble.img.bin", &bgGfxLen);
				bgMap = readFile("/data/ao-nds/misc/takethat_bubble.map.bin", &bgMapLen);
				bgPal = readFile("/data/ao-nds/misc/takethat_bubble.pal.bin", &bgPalLen);
			}
			sndFile += "/takethat.wav";
			break;

		case 4:
			bgGfx = readFile(contentPath + "/" + custom + ".img.bin", &bgGfxLen);
			bgMap = readFile(contentPath + "/" + custom + ".map.bin", &bgMapLen);
			bgPal = readFile(contentPath + "/" + custom + ".pal.bin", &bgPalLen);
			sndFile += "/" + custom + ".wav";
			break;
	}
	sndShout = wav_load_handle(sndFile.c_str());
	if (!sndShout)
		sndShout = wav_load_handle("/data/ao-nds/sounds/general/sfx-objection.wav");

	bgHide(bgIndex);

	m_pCourt->flash(3);
	m_pCourt->getChatbox()->setVisible(false);

	if (bgGfx && bgMap && bgPal)
	{
		u16 oldPal = BG_PALETTE[0];
		dmaCopy(bgGfx, bgGetGfxPtr(bgIndex), bgGfxLen);
		dmaCopy(bgMap, bgGetMapPtr(bgIndex), bgMapLen);
		dmaCopy(bgPal, BG_PALETTE, bgPalLen);
		BG_PALETTE[0] = oldPal;
	}

	if (bgGfx) mem_free(bgGfx);
	if (bgMap) mem_free(bgMap);
	if (bgPal) mem_free(bgPal);

	adx_update();
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
		wav_free_handle(sndShout);
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
		wav_play(sndShout);
		if (visible) bgShow(bgIndex);
	}
	else if (ticks > 75)
	{
		cancelShout();
		if (onShoutFinished)
			onShoutFinished(pUserData);
	}
}
