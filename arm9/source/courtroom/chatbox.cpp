#include "courtroom/chatbox.h"

#include <string.h>

#include <nds/arm9/background.h>
#include <nds/arm9/decompress.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>
#include <nds/arm9/sound.h>

#include "global.h"
#include "fonts.h"
#include "mp3_shared.h"

Chatbox::Chatbox()
{
	textCanvas = new u8[32*16];

	for (int i=0; i<2; i++)
	{
		nameGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x16, SpriteColorFormat_256Color);
		dmaFillHalfWords((0<<8)|0, nameGfx[i], 32*16);
		oamSet(&oamMain, 24+i, 4+(i*32), 115, 0, 0, SpriteSize_32x16, SpriteColorFormat_256Color, nameGfx[i], -1, false, false, false, false, false);
	}
	for (int i=0; i<8*3; i++)
	{
		int x = i%8;
		int y = i/8;

		textGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x16, SpriteColorFormat_256Color);
		dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);
		oamSet(&oamMain, 26+i, 8+(x*32), 132+(y*16), 0, 0, SpriteSize_32x16, SpriteColorFormat_256Color, textGfx[i], -1, false, false, false, false, false);
	}

	memset(textCanvas, 0, 32*16); // black bg


	//bgIndex = bgInit(2, BgType_ExRotation, BgSize_ER_256x256, 2, 1);
	bgIndex = bgInit(1, BgType_Text8bpp, BgSize_T_256x256, 10, 0);
	bgSetPriority(bgIndex, 1);
	bgSetScroll(bgIndex, 0, -192+80);
	bgHide(bgIndex);
	bgUpdate();

	vramSetBankE(VRAM_E_LCD);
	vramSetBankF(VRAM_F_LCD);

	u32 dataLen, mapLen, palLen;
	bgData = readFile("/data/ao-nds/misc/chatbox.img.bin", &dataLen);
	bgMap = readFile("/data/ao-nds/misc/chatbox.map.bin", &mapLen);
	bgPal = readFile("/data/ao-nds/misc/chatbox.pal.bin", &palLen);

	dmaCopy(bgData, bgGetGfxPtr(bgIndex), dataLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), mapLen);
	dmaCopy(bgPal, (void *)&VRAM_E_EXT_PALETTE[bgIndex][1], palLen);

	VRAM_F_EXT_SPR_PALETTE[0][COLOR_WHITE] = 	RGB15(31,31,31);
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_GREEN] = 	RGB15(0,31,0);
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_RED] = 		RGB15(31,0,0);
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_ORANGE] = 	RGB15(31,20,0);
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_BLUE] = 	RGB15(5,18,31);
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_YELLOW] = 	RGB15(31,31,0);
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_BLACK] = 	RGB15(0,0,0);

	vramSetBankE(VRAM_E_BG_EXT_PALETTE);
	vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

	// makes the chatbox transparent.
	// chatbox is in bg1 and we want to see bg0 (court) and sprites behind it
	REG_BLDCNT = BLEND_ALPHA | BLEND_SRC_BG1 | BLEND_DST_BG0 | BLEND_DST_SPRITE;
	REG_BLDALPHA = 0x70f;
}

Chatbox::~Chatbox()
{
	bgHide(bgIndex);

	REG_BLDCNT = BLEND_NONE;

	dmaFillHalfWords(0, (void *)&VRAM_F_EXT_PALETTE[bgIndex][0], 512);

	delete[] bgData;
	delete[] bgMap;
	delete[] bgPal;

	if (blipSnd)
		delete[] blipSnd;

	delete[] textCanvas;
	for (int i=0; i<2; i++)
	{
		oamSet(&oamMain, 24+i, 4+(i*32), 115, 0, 0, SpriteSize_32x16, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
		oamFreeGfx(&oamMain, nameGfx[i]);
	}
	for (int i=0; i<8*3; i++)
	{
		int x = i%8;
		int y = i/8;
		oamSet(&oamMain, 26+i, 8+(x*32), 132+(y*16), 0, 0, SpriteSize_32x16, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
		oamFreeGfx(&oamMain, textGfx[i]);
	}
}

void Chatbox::setVisible(bool on)
{
	(on) ? bgShow(bgIndex) : bgHide(bgIndex);
}

void Chatbox::setName(const char* name)
{
	// clear old text
	memset(textCanvas, 0, 32*16);
	for (int i=0; i<2; i++)
		dmaFillHalfWords((0<<8)|0, nameGfx[i], 32*16);
	nameWidth = renderText(1, name, COLOR_WHITE, 32, 16, textCanvas, SpriteSize_32x16, nameGfx, 2);

	for (int i=0; i<2; i++)
		oamSet(&oamMain, 24+i, 1+(i*32) + 32-(nameWidth/2), 115, 0, 0, SpriteSize_32x16, SpriteColorFormat_256Color, nameGfx[i], -1, false, false, false, false, false);
}

void Chatbox::setText(std::string text, int color, std::string blip)
{
	currText = text;
	textColor = color;

	currTextInd = 0;
	currTextGfxInd = 0;
	textX = 0;
	textTicks = 0;
	textSpeed = 2;
	blipTicks = 0;

	if (blipSnd)
		delete[] blipSnd;
	std::string blipFile = "/data/ao-nds/sounds/blips/" + blip + ".wav";
	blipSnd = wav_load_handle(blipFile.c_str(), &blipSize);

	memset(textCanvas, 0, 32*16);
	for (int i=0; i<8*3; i++)
		dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);
}

void Chatbox::shake(int force, int ticks)
{
	shakeForce = force;
	shakeTicks = ticks;
}

void Chatbox::update()
{
	// handle bg shake
	int xShake = 0;
	int yShake = 0;

	if (shakeTicks > 0)
	{
		shakeTicks--;
		xShake = -shakeForce + rand()%(shakeForce*2);
		yShake = -shakeForce + rand()%(shakeForce*2);
	}

	bgSetScroll(bgIndex, -xShake, -192+80-yShake);

	for (int i=0; i<2; i++)
		oamSetXY(&oamMain, 24+i, 1+(i*32) + 32-(nameWidth/2) + xShake, 115+yShake);


	// handle chatbox text typewriter
	if (currTextInd >= currText.size())
		return;

	textTicks++;
	if (blipTicks > 0) blipTicks--;

	if (textTicks > textSpeed)
	{
		textTicks = 0;

		char currChar = currText.c_str()[currTextInd];
		if (blipSnd && currChar != ' ' && blipTicks <= 0)
		{
			soundPlaySample(blipSnd, SoundFormat_16Bit, blipSize, 32000, 127, 64, false, 0);
			blipTicks = 6-textSpeed;
		}

		if (currTextGfxInd >= 8*3)
		{
			currTextInd++;
			return;
		}

		bool lastBox = (currTextGfxInd % 8 == 7);
		int boxWidth = lastBox ? 20 : 32;
		int oobFlag;
		int outWidth;
		int new_x = renderChar(2, currText.c_str()+currTextInd, textColor, textX, 32, boxWidth, 16, textCanvas, SpriteSize_32x16, textGfx[currTextGfxInd], lastBox, &oobFlag, &outWidth);

		if (oobFlag)
		{
			currTextGfxInd++;
			memset(textCanvas, 0, 32*16);

			if (currTextGfxInd % 8 == 0)
			{
				// entered a new line
				textX = 0;
				if (oobFlag == 2)
					currTextInd--;
			}
			else
			{
				textX -= boxWidth;
				textX = renderChar(2, currText.c_str()+currTextInd, textColor, textX, 32, boxWidth, 16, textCanvas, SpriteSize_32x16, textGfx[currTextGfxInd], lastBox, &oobFlag, &outWidth);
			}
		}
		else
		{
			textX = new_x;
			if (textX > boxWidth)
			{
				currTextGfxInd++;
				if (currTextGfxInd % 8 == 0)
					textX = 0;
				else
					textX -= boxWidth;
			}
		}

		currTextInd++;
	}
}
