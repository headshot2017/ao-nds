#include "courtroom/evidence.h"

#include <nds/dma.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>

#include "global.h"

#define EVIDENCE_HIDDEN_SCALE 3328
#define EVIDENCE_SHOWN_SCALE 256
#define EVIDENCE_ADD_SCALE 192

Evidence::Evidence()
{
	visible = false;
	offsetX = 0;
	offsetY = 0;
	right = false;
	scale = EVIDENCE_HIDDEN_SCALE;
	scaleAdd = 0;
	nextEvidenceRight = false;

	spriteGfx = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_256Color);
	sndEvShow = wav_load_handle("/data/ao-nds/sounds/general/sfx-shooop.wav");

	dmaFillHalfWords(0, spriteGfx, 64*64);
	oamRotateScale(&oamMain, 0, 0, scale, scale);
	oamSet(&oamMain, 126, 16, 16, 1, 4, SpriteSize_64x64, SpriteColorFormat_256Color, spriteGfx, 0, false, false, false, false, false);
}

Evidence::~Evidence()
{
	oamFreeGfx(&oamMain, spriteGfx);
	oamClearSprite(&oamMain, 126);
	wav_free_handle(sndEvShow);
}

void Evidence::setVisible(bool on)
{
	visible = on;
}

void Evidence::hideEvidence()
{
	if (scale != EVIDENCE_HIDDEN_SCALE)
		scaleAdd = EVIDENCE_ADD_SCALE;
}

void Evidence::showEvidence(const std::string& image, bool rightSide)
{
	if (scale != EVIDENCE_HIDDEN_SCALE)
	{
		hideEvidence();
		nextEvidence = image;
		nextEvidenceRight = rightSide;
		return;
	}

	right = rightSide;

	u8* gfx = readFile("/data/ao-nds/evidence/large/" + image + ".img.bin");
	u8* pal = readFile("/data/ao-nds/evidence/large/" + image + ".pal.bin");
	if (!gfx || !pal)
	{
		if (gfx) delete[] gfx;
		if (pal) delete[] pal;
		if (image != "Empty") showEvidence("Empty", rightSide);
		return;
	}

	dmaCopy(gfx, spriteGfx, 64*64);

	vramSetBankF(VRAM_F_LCD);
	dmaCopy(pal, &VRAM_F_EXT_SPR_PALETTE[4], 512);
	vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

	delete[] gfx;
	delete[] pal;

	scaleAdd = -EVIDENCE_ADD_SCALE;
}

void Evidence::update()
{
	oamSetXY(&oamMain, 126, (right ? 256-64-16 : 16) + offsetX, 16+offsetY);
	if (!scaleAdd) return;

	if (scale == EVIDENCE_HIDDEN_SCALE || scale == EVIDENCE_SHOWN_SCALE) // starting
		wav_play(sndEvShow);

	scale += scaleAdd;
	if (scale <= EVIDENCE_SHOWN_SCALE)
	{
		scale = EVIDENCE_SHOWN_SCALE;
		scaleAdd = 0;
	}
	else if (scale >= EVIDENCE_HIDDEN_SCALE)
	{
		scale = EVIDENCE_HIDDEN_SCALE;
		scaleAdd = 0;
		if (!nextEvidence.empty())
		{
			showEvidence(nextEvidence, nextEvidenceRight);
			nextEvidence.clear();
			return;
		}
		dmaFillHalfWords(0, spriteGfx, 64*64);
	}

	oamRotateScale(&oamMain, 0, 0, scale, scale);
}
