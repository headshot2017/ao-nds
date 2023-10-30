#include "courtroom/chatbox.h"

#include <string.h>

#include <nds/arm9/background.h>
#include <nds/arm9/decompress.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>
#include <nds/arm9/sound.h>

#include "courtroom/courtroom.h"
#include "global.h"
#include "fonts.h"
#include "colors.h"
#include "mp3_shared.h"

Chatbox::Chatbox(Courtroom* pCourt)
{
	m_pCourt = pCourt;

	textCanvas = new u8[32*16];
	blipSnd = 0;

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
	bgIndex = bgInit(1, BgType_Text4bpp, BgSize_T_512x256, 2, 5);
	bgSetPriority(bgIndex, 1);
	bgSetScroll(bgIndex, 0, -192+80);
	bgHide(bgIndex);
	bgUpdate();

	vramSetBankF(VRAM_F_LCD);

	arrowX = 243;
	arrowXadd = 1;
	arrowTicks = 0;

	u8* chatboxArrowImg = readFile("/data/ao-nds/ui/spr_chatboxArrow.img.bin");
	u8* chatboxArrowPal = readFile("/data/ao-nds/ui/spr_chatboxArrow.pal.bin");
	spr_arrowGfx = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
	dmaCopy(chatboxArrowImg, spr_arrowGfx, 16*16);
	memcpy(&VRAM_F_EXT_SPR_PALETTE[3], chatboxArrowPal, 512);
	oamSet(&oamMain, 127, arrowX, 174, 0, 3, SpriteSize_16x16, SpriteColorFormat_256Color, spr_arrowGfx, -1, false, false, false, false, false);
	delete[] chatboxArrowImg;
	delete[] chatboxArrowPal;

	VRAM_F_EXT_SPR_PALETTE[0][COLOR_WHITE] = 	PAL_WHITE;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_GREEN] = 	PAL_GREEN;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_RED] = 		PAL_RED;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_ORANGE] = 	PAL_ORANGE;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_BLUE] = 	PAL_BLUE;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_YELLOW] = 	PAL_YELLOW;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_BLACK] = 	PAL_BLACK;

	u32 dataLen;
	u8* bgData = readFile("/data/ao-nds/misc/chatbox.img.bin", &dataLen);
	bgMap = readFile("/data/ao-nds/misc/chatbox.map.bin", &mapLen);
	bgPal = readFile("/data/ao-nds/misc/chatbox.pal.bin");

	dmaCopy(bgData, bgGetGfxPtr(bgIndex), dataLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), mapLen);
	memcpy(BG_PALETTE, bgPal, 512);
	BG_PALETTE[0] = 0;

	delete[] bgData;

	vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

	visible = false;
	setOnChatboxFinishedCallback(0, 0);
}

Chatbox::~Chatbox()
{
	bgHide(bgIndex);
	dmaFillHalfWords(0, BG_PALETTE, 512);
	delete[] bgPal;
	delete[] bgMap;

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

	oamFreeGfx(&oamMain, spr_arrowGfx);
}

void Chatbox::setVisible(bool on)
{
	visible = on;
	(on) ? bgShow(bgIndex) : bgHide(bgIndex);
	if (on)
	{
		dmaCopy(bgMap, bgGetMapPtr(bgIndex), mapLen);
		memcpy(BG_PALETTE, bgPal, 512);
		BG_PALETTE[0] = 0;
	}
	else
		dmaFillHalfWords(0, bgGetMapPtr(bgIndex), mapLen);

	for (int i=0; i<2; i++)
		oamSetHidden(&oamMain, 24+i, !on);

	for (int i=0; i<8*3; i++)
		oamSetHidden(&oamMain, 26+i, !on);

	oamSetHidden(&oamMain, 127, !on);
}

void Chatbox::setName(std::string name)
{
	// clear old text
	memset(textCanvas, 0, 32*16);
	for (int i=0; i<2; i++)
		dmaFillHalfWords((0<<8)|0, nameGfx[i], 32*16);
	nameWidth = getTextWidth(0, name.c_str());
	renderText(0, name.c_str(), COLOR_WHITE, 32, 16, textCanvas, SpriteSize_32x16, nameGfx, 2);

	for (int i=0; i<2; i++)
		oamSet(&oamMain, 24+i, 1+(i*32) + 36-(nameWidth/2), 115, 0, 0, SpriteSize_32x16, SpriteColorFormat_256Color, nameGfx[i], -1, false, !visible, false, false, false);
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

	oamSetHidden(&oamMain, 127, true);

	memset(textCanvas, 0, 32*16);
	for (int i=0; i<8*3; i++)
		dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);
}

void Chatbox::additiveText(std::string text, int color)
{
	int line = currTextGfxInd/8;
	currTextGfxInd = (line+1) * 8;

	currText = text;
	textColor = color;

	currTextInd = 0;
	textX = 0;
	textTicks = 0;
	textSpeed = 2;
	blipTicks = 0;

	memset(textCanvas, 0, 32*16);
	oamSetHidden(&oamMain, 127, true);

	if (currTextGfxInd >= 8*3)
	{
		currTextGfxInd = 0;
		for (int i=0; i<8*3; i++)
			dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);
	}
}

void Chatbox::update()
{
	if (!visible)
		return;

	// makes the chatbox transparent.
	// chatbox is in bg1 and we want to see bg0 (court) and sprites behind it
	REG_BLDCNT = BLEND_ALPHA | BLEND_SRC_BG1 | BLEND_DST_BG0 | BLEND_DST_SPRITE;
	REG_BLDALPHA = 0x70f;

	bgSetScroll(bgIndex, -xOffset, -192+80-yOffset);

	for (int i=0; i<2; i++)
		oamSetXY(&oamMain, 24+i, 1+(i*32) + 36-(nameWidth/2) + xOffset, 115+yOffset);

	arrowTicks++;
	if (arrowTicks >= 3)
	{
		arrowTicks = 0;
		arrowX += arrowXadd;
		if (arrowX >= 246 || arrowX <= 243)
			arrowXadd = -arrowXadd;
		oamSetXY(&oamMain, 127, arrowX, 174);
	}

	// handle chatbox text typewriter
	if (isFinished())
	{
		if (visible) oamSetHidden(&oamMain, 127, false);
		return;
	}

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
			currTextGfxInd = 0;
			for (int i=0; i<8*3; i++)
				dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);
		}

		bool lastBox = (currTextGfxInd % 8 == 7);
		int boxWidth = lastBox ? 20 : 32;
		int oobFlag;
		int outWidth;
		int new_x = renderChar(1, currText.c_str()+currTextInd, textColor, textX, 32, boxWidth, 16, textCanvas, SpriteSize_32x16, textGfx[currTextGfxInd], lastBox, &oobFlag, &outWidth);

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
				textX = renderChar(1, currText.c_str()+currTextInd, textColor, textX, 32, boxWidth, 16, textCanvas, SpriteSize_32x16, textGfx[currTextGfxInd], lastBox, &oobFlag, &outWidth);
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
		if (isFinished() && onChatboxFinished)
		{
			oamSetHidden(&oamMain, 127, false);
			onChatboxFinished(pUserData);
		}
	}
}
