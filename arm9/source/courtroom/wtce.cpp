#include "courtroom/wtce.h"

#include <string.h>
#include <stdio.h>

#include <nds/dma.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>

#include "arm9_math_alt.h"
#include "courtroom/courtroom.h"
#include "global.h"
#include "mem.h"

#define lerp(a, b, amount) (a + (mulf32(b-a, amount)))

WTCE::WTCE(Courtroom* pCourt, int start)
{
	m_pCourt = pCourt;

	visible = false;
	oamStart = start;
	gfx = 0;
	gfxInUse = 0;
	sfx = 0;

	for (int i=0; i<2; i++)
	{
		imgGfx[i] = 0;
		imgPal[i] = 0;
	}

	spr_testimonyIndicator = 0;
	indicatorTicks = 0;

	currFrame = 0;
}

WTCE::~WTCE()
{
	clear();
}

void WTCE::setVisible(bool on)
{
	visible = on;
}

void WTCE::clear(bool keepIndicator)
{
	if (gfx)
	{
		for (int i=0; i<gfxInUse; i++)
		{
			if (gfx[i]) oamFreeGfx(&oamMain, gfx[i]);
			oamClearSprite(&oamMain, oamStart+i);
		}
		delete[] gfx;
		gfx = 0;
		gfxInUse = 0;
	}

	if (sfx)
	{
		wav_free_handle(sfx);
		sfx = 0;
	}

	for (int i=0; i<2; i++)
	{
		if (imgGfx[i])
		{
			mem_free(imgGfx[i]);
			imgGfx[i] = 0;
		}
		if (imgPal[i])
		{
			mem_free(imgPal[i]);
			imgPal[i] = 0;
		}
	}

	if (spr_testimonyIndicator && !keepIndicator)
	{
		oamFreeGfx(&oamMain, spr_testimonyIndicator);
		oamClearSprite(&oamMain, 125);
		spr_testimonyIndicator = 0;
		indicatorTicks = 0;
	}

	allLerps.clear();
}

void WTCE::play(const std::string& msg)
{
	//testimony1 - "Witness Testimony"
	//testimony2 - "Cross Examination"
	//judgeruling#0 - "Not Guilty" (since 2.6)
	//judgeruling#1 - "Guilty" (since 2.6)
	//testimony1#1 - Hides the "Testimony" indicator (since 2.9)

	std::string mode = argumentAt(msg, 1);
	std::string extra = argumentAt(msg, 2);
	clear();

	// i really don't wanna do this, but then again it's not executed every frame...
	int animMode = -1;
	if (mode == "testimony1")
	{
		if (extra == "1") // get rid of "Testimony" indicator
			return;
		animMode = 0;
	}
	else if (mode == "testimony2") animMode = 1;
	else if (mode == "judgeruling")
	{
		mode += extra;
		animMode = (extra == "1") ? 2 : 3;
	}

	u8* tempGfx = 0;
	u8* tempPal = 0;
	switch(animMode)
	{
		case 0: // Witness Testimony
			sfx = wav_load_handle("/data/ao-nds/sounds/general/sfx-testimony2.wav");
			imgGfx[0] = readFile("/data/ao-nds/ui/spr_witTest1.img.bin");
			imgPal[0] = readFile("/data/ao-nds/ui/spr_witTest1.pal.bin");
			imgGfx[1] = readFile("/data/ao-nds/ui/spr_witTest2.img.bin");
			imgPal[1] = readFile("/data/ao-nds/ui/spr_witTest2.pal.bin");
			tempGfx = readFile("/data/ao-nds/ui/spr_testimonyIndicator.img.bin");
			tempPal = readFile("/data/ao-nds/ui/spr_testimonyIndicator.pal.bin");

			if (tempGfx && tempPal)
			{
				spr_testimonyIndicator = oamAllocateGfx(&oamMain, SpriteSize_64x32, SpriteColorFormat_256Color);
				dmaCopy(tempGfx, spr_testimonyIndicator, 64*32);

				vramSetBankF(VRAM_F_LCD);
				dmaCopy(tempPal, &VRAM_F_EXT_SPR_PALETTE[10], 512);
				vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

				oamSet(&oamMain, 125, 0, 0, 0, 10, SpriteSize_64x32, SpriteColorFormat_256Color, spr_testimonyIndicator, -1, false, false, false, false, false);
			}
			if (tempGfx) mem_free(tempGfx);
			if (tempPal) mem_free(tempPal);

			if (imgGfx[0] && imgPal[0] && imgGfx[1] && imgPal[1])
			{
				gfxInUse = (5*3) + (6*3);
				gfx = new u16*[gfxInUse];
				for (int i=0; i<5*3; i++)
				{
					gfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x16, SpriteColorFormat_256Color);
					if (!gfx[i]) break;
					dmaCopy(imgGfx[0] + (i*32*16), gfx[i], 32*16);
					oamSet(&oamMain, oamStart+i, 0, 0, 0, 6, SpriteSize_32x16, SpriteColorFormat_256Color, gfx[i], -1, false, false, false, false, false);
				}
				for (int i=0; i<6*3; i++)
				{
					gfx[i+(5*3)] = oamAllocateGfx(&oamMain, SpriteSize_32x16, SpriteColorFormat_256Color);
					if (!gfx[i+(5*3)]) break;
					dmaCopy(imgGfx[1] + (i*32*16), gfx[i+(5*3)], 32*16);
					oamSet(&oamMain, oamStart+i+(5*3), 0, 0, 0, 11, SpriteSize_32x16, SpriteColorFormat_256Color, gfx[i+(5*3)], -1, false, false, false, false, false);
				}

				u8 r=0, g=0, b=0;
				u32 R=0, G=0, B=0;

				vramSetBankF(VRAM_F_LCD);

				for (int j=0; j<4; j++)
				{
					int lerpAmount = divf32(inttof32(j), inttof32(3));

					for (int i=0; i<256; i++)
					{
						fromRGB15( ((u16*)imgPal[0])[i], r, g, b);
						R = f32toint(lerp(inttof32(r), inttof32(31), lerpAmount));
						G = f32toint(lerp(inttof32(g), inttof32(31), lerpAmount));
						B = f32toint(lerp(inttof32(b), inttof32(31), lerpAmount));
						VRAM_F_EXT_SPR_PALETTE[6+j][i] = RGB15(R, G, B);
					}

					for (int i=0; i<256; i++)
					{
						fromRGB15( ((u16*)imgPal[1])[i], r, g, b);
						R = f32toint(lerp(inttof32(r), inttof32(31), lerpAmount));
						G = f32toint(lerp(inttof32(g), inttof32(31), lerpAmount));
						B = f32toint(lerp(inttof32(b), inttof32(31), lerpAmount));
						VRAM_F_EXT_SPR_PALETTE[11+j][i] = RGB15(R, G, B);
					}
				}

				vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
			}
			break;

		case 1: // Cross-Examination
			sfx = wav_load_handle("/data/ao-nds/sounds/general/sfx-testimony2.wav");
			imgGfx[0] = readFile("/data/ao-nds/ui/spr_crossExam1.img.bin");
			imgPal[0] = readFile("/data/ao-nds/ui/spr_crossExam1.pal.bin");
			imgGfx[1] = readFile("/data/ao-nds/ui/spr_crossExam2.img.bin");
			imgPal[1] = readFile("/data/ao-nds/ui/spr_crossExam2.pal.bin");

			if (imgGfx[0] && imgPal[0] && imgGfx[1] && imgPal[1])
			{
				gfxInUse = (4*3) + (6*3);
				gfx = new u16*[gfxInUse];
				for (int i=0; i<4*3; i++)
				{
					gfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x16, SpriteColorFormat_256Color);
					if (!gfx[i]) break;
					dmaCopy(imgGfx[0] + (i*32*16), gfx[i], 32*16);
					oamSet(&oamMain, oamStart+i, 0, 0, 0, 6, SpriteSize_32x16, SpriteColorFormat_256Color, gfx[i], -1, false, false, false, false, false);
				}
				for (int i=0; i<6*3; i++)
				{
					gfx[i+(4*3)] = oamAllocateGfx(&oamMain, SpriteSize_32x16, SpriteColorFormat_256Color);
					if (!gfx[i+(4*3)]) break;
					dmaCopy(imgGfx[1] + (i*32*16), gfx[i+(4*3)], 32*16);
					oamSet(&oamMain, oamStart+i+(4*3), 0, 0, 0, 11, SpriteSize_32x16, SpriteColorFormat_256Color, gfx[i+(4*3)], -1, false, false, false, false, false);
				}

				u8 r=0, g=0, b=0;
				u32 R=0, G=0, B=0;

				vramSetBankF(VRAM_F_LCD);

				for (int j=0; j<5; j++)
				{
					int lerpAmount = divf32(inttof32(j), inttof32(4));

					for (int i=0; i<256; i++)
					{
						fromRGB15( ((u16*)imgPal[0])[i], r, g, b);
						R = f32toint(lerp(inttof32(r), inttof32(31), lerpAmount));
						G = f32toint(lerp(inttof32(g), inttof32(31), lerpAmount));
						B = f32toint(lerp(inttof32(b), inttof32(31), lerpAmount));
						VRAM_F_EXT_SPR_PALETTE[6+j][i] = RGB15(R, G, B);
					}

					for (int i=0; i<256; i++)
					{
						fromRGB15( ((u16*)imgPal[1])[i], r, g, b);
						R = f32toint(lerp(inttof32(r), inttof32(31), lerpAmount));
						G = f32toint(lerp(inttof32(g), inttof32(31), lerpAmount));
						B = f32toint(lerp(inttof32(b), inttof32(31), lerpAmount));
						VRAM_F_EXT_SPR_PALETTE[11+j][i] = RGB15(R, G, B);
					}
				}

				vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
			}
			break;

		case 2: // Guilty
			sfx = wav_load_handle("/data/ao-nds/sounds/general/sfx-guilty.wav");
			imgGfx[0] = readFile("/data/ao-nds/ui/spr_guilty.img.bin");
			imgPal[0] = readFile("/data/ao-nds/ui/spr_guilty.pal.bin");

			if (imgGfx[0] && imgPal[0])
			{
				gfxInUse = 6;
				gfx = new u16*[gfxInUse];
				for (int i=0; i<gfxInUse; i++)
				{
					gfx[i] = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_256Color);
					if (!gfx[i]) break;
					dmaCopy(imgGfx[0] + (i*64*64), gfx[i], 64*64);
					oamSet(&oamMain, oamStart+i, 0, 0, 0, 6, SpriteSize_64x64, SpriteColorFormat_256Color, gfx[i], -1, false, false, false, false, false);
				}

				vramSetBankF(VRAM_F_LCD);
				dmaCopy(imgPal[0], &VRAM_F_EXT_SPR_PALETTE[6], 512);
				vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
			}
			break;

		case 3: // Not Guilty
			sfx = wav_load_handle("/data/ao-nds/sounds/general/sfx-guilty.wav");
			imgGfx[0] = readFile("/data/ao-nds/ui/spr_notGuilty.img.bin");
			imgPal[0] = readFile("/data/ao-nds/ui/spr_notGuilty.pal.bin");

			if (imgGfx[0] && imgPal[0])
			{
				gfxInUse = 9;
				gfx = new u16*[gfxInUse];
				gfx[gfxInUse-1] = 0;
				for (int i=0; i<gfxInUse-1; i++)
				{
					gfx[i] = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_256Color);
					if (!gfx[i]) break;
					dmaCopy(imgGfx[0] + (i*64*64), gfx[i], 64*64);
				}

				// oam indexes oamStart+2 and oamStart+7 correspond to letter T, in order to save some gfx space
				oamSet(&oamMain, oamStart+0, 0, 0, 0, 6, SpriteSize_64x64, SpriteColorFormat_256Color, gfx[0], -1, false, false, false, false, false);
				oamSet(&oamMain, oamStart+1, 0, 0, 0, 6, SpriteSize_64x64, SpriteColorFormat_256Color, gfx[1], -1, false, false, false, false, false);
				oamSet(&oamMain, oamStart+2, 0, 0, 0, 6, SpriteSize_64x64, SpriteColorFormat_256Color, gfx[6], -1, false, false, false, false, false);
				oamSet(&oamMain, oamStart+3, 0, 0, 0, 6, SpriteSize_64x64, SpriteColorFormat_256Color, gfx[2], -1, false, false, false, false, false);
				oamSet(&oamMain, oamStart+4, 0, 0, 0, 6, SpriteSize_64x64, SpriteColorFormat_256Color, gfx[3], -1, false, false, false, false, false);
				oamSet(&oamMain, oamStart+5, 0, 0, 0, 6, SpriteSize_64x64, SpriteColorFormat_256Color, gfx[4], -1, false, false, false, false, false);
				oamSet(&oamMain, oamStart+6, 0, 0, 0, 6, SpriteSize_64x64, SpriteColorFormat_256Color, gfx[5], -1, false, false, false, false, false);
				oamSet(&oamMain, oamStart+7, 0, 0, 0, 6, SpriteSize_64x64, SpriteColorFormat_256Color, gfx[6], -1, false, false, false, false, false);
				oamSet(&oamMain, oamStart+8, 0, 0, 0, 6, SpriteSize_64x64, SpriteColorFormat_256Color, gfx[7], -1, false, false, false, false, false);

				vramSetBankF(VRAM_F_LCD);
				dmaCopy(imgPal[0], &VRAM_F_EXT_SPR_PALETTE[6], 512);
				vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
			}
			break;
	}

	currAnim = mode;
	currFrame = 0;
}

void WTCE::update()
{
	if (spr_testimonyIndicator)
	{
		indicatorTicks++;
		int limit = (oamMain.oamMemory[125].isHidden) ? 20 : 100;

		if (indicatorTicks > limit)
		{
			oamSetHidden(&oamMain, 125, !oamMain.oamMemory[125].isHidden);
			indicatorTicks = 0;
		}
	}

	if (!gfxInUse) return;

	if (wtceAnims.count(currAnim) && wtceAnims[currAnim].count(currFrame))
	{
		for (u32 i=0; i<wtceAnims[currAnim][currFrame].size(); i++)
		{
			keyFrame& frame = wtceAnims[currAnim][currFrame][i];
			TickLerp tickLerp;
			switch(frame.type)
			{
				case KEYFRAME_MOVE:
					tickLerp.ticks = 0;
					tickLerp.affineIndex = -1;
					tickLerp.oamIndex = frame.move.oamIndex;
					tickLerp.spriteSize = frame.move.spriteSize;
					tickLerp.tileSize = frame.move.tileSize;
					tickLerp.maxTicks = frame.move.tickLength;
					tickLerp.from.x = inttof32(frame.move.from.x);
					tickLerp.from.y = inttof32(frame.move.from.y);
					tickLerp.to.x = inttof32(frame.move.to.x);
					tickLerp.to.y = inttof32(frame.move.to.y);
					tickLerp.lerpFunc = frame.move.lerpFunc;
					allLerps.push_back(tickLerp);
					break;

				case KEYFRAME_SCALE:
					tickLerp.ticks = 0;
					tickLerp.affineIndex = frame.scale.affineIndex;
					tickLerp.oamIndex = frame.scale.oamIndex;
					tickLerp.spriteSize = frame.scale.spriteSize;
					tickLerp.maxTicks = frame.scale.tickLength;
					tickLerp.from.x = inttof32(frame.scale.from);
					tickLerp.to.x = inttof32(frame.scale.to);
					tickLerp.lerpFunc = frame.scale.lerpFunc;
					oamSetAffineIndex(&oamMain, oamStart+tickLerp.oamIndex, tickLerp.affineIndex, true);
					allLerps.push_back(tickLerp);
					break;

				case KEYFRAME_SHAKE:
					m_pCourt->shake(3, frame.ticks);
					break;

				case KEYFRAME_FLASH:
					m_pCourt->flash(frame.ticks);
					break;

				case KEYFRAME_IMGFRAME:
					{
						int tileW = frame.imgFrame.tileSize.x;
						int tileH = frame.imgFrame.tileSize.y;
						int sprW = frame.imgFrame.spriteSize.x;
						int sprH = frame.imgFrame.spriteSize.y;
						for (int i=0; i<tileW*tileH; i++)
						{
							if (!gfx[i+frame.imgFrame.oamIndex]) break;
							u8* offset = imgGfx[frame.imgFrame.gfxIndex] + (frame.imgFrame.frame * tileW*tileH * sprW*sprH);
							dmaCopy(offset + (i*sprW*sprH), gfx[i+frame.imgFrame.oamIndex], sprW*sprH);
						}
					}
					break;

				case KEYFRAME_VISIBLE:
					for (int j=0; j<frame.visible.max; j++)
						oamSetHidden(&oamMain, oamStart + frame.visible.oamIndex + j, !frame.visible.on);
					break;

				case KEYFRAME_PALETTE:
					for (int j=0; j<frame.palette.max; j++)
						oamSetPalette(&oamMain, oamStart + frame.palette.oamIndex + j, frame.palette.paletteInd);
					break;

				case KEYFRAME_SFX:
					wav_play(sfx);
					break;

				case KEYFRAME_END:
					clear(true);
					break;
			}
		}
	}

	for (u32 i=0; i<allLerps.size(); i++)
	{
		TickLerp& tickLerp = allLerps[i];

		int lerpAmount = divf32(inttof32(tickLerp.ticks), inttof32(tickLerp.maxTicks));
		int x = f32toint(lerp(tickLerp.from.x, tickLerp.to.x, tickLerp.lerpFunc(lerpAmount)));
		int y = f32toint(lerp(tickLerp.from.y, tickLerp.to.y, tickLerp.lerpFunc(lerpAmount)));

		if (tickLerp.affineIndex > -1)
			oamRotateScale(&oamMain, tickLerp.affineIndex, 0, x, x);
		else
		{
			for (int tileY=0; tileY<tickLerp.tileSize.y; tileY++)
			{
				for (int tileX=0; tileX<tickLerp.tileSize.x; tileX++)
				{
					int xx = tileX * tickLerp.spriteSize.x;
					int yy = tileY * tickLerp.spriteSize.y;
					int i = tileY * tickLerp.tileSize.x + tileX;
					oamSetXY(&oamMain, oamStart + tickLerp.oamIndex + i, x + xx, y + yy);
				}
			}
		}

		tickLerp.ticks++;
		if (tickLerp.ticks > tickLerp.maxTicks)
		{
			if (tickLerp.affineIndex > -1 && x >= 256)
			{
				int xPos = oamMain.oamMemory[oamStart + tickLerp.oamIndex].x;
				int yPos = oamMain.oamMemory[oamStart + tickLerp.oamIndex].y;

				oamSetAffineIndex(&oamMain, oamStart + tickLerp.oamIndex, -1, false);
				oamSetXY(&oamMain, oamStart + tickLerp.oamIndex, xPos + (tickLerp.spriteSize.x>>1), yPos + (tickLerp.spriteSize.y>>1));
			}
			allLerps.erase(allLerps.begin() + i);
			i--;
		}
	}

	currFrame++;
}
