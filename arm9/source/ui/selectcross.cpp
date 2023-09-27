#include "ui/selectcross.h"

#include <nds/dma.h>

#include "mp3_shared.h"
#include "spr_buttonCorner.h"

UISelectCross::UISelectCross(OamState* chosenOam, int oamStartInd, int palSlot)
{
	oam = chosenOam;
	oamStart = oamStartInd;

	spriteGfx = oamAllocateGfx(oam, SpriteSize_16x16, SpriteColorFormat_256Color);
	dmaCopy((u8*)spr_buttonCornerTiles, spriteGfx, 16*16);
	mp3_fill_buffer();

	for (int i=0; i<4; i++)
	{
		// order: no flip, hflip, vflip, both flips
		oamSet(oam, oamStart+i, 0, 0, 0, palSlot, SpriteSize_16x16, SpriteColorFormat_256Color, spriteGfx, -1, false, false, (i&1), (i&2), false);
	}

	// copy palette to ext palette vram slot
	if (oam == &oamMain)
	{
		vramSetBankF(VRAM_F_LCD);
		dmaCopy((u8*)spr_buttonCornerPal, &VRAM_F_EXT_SPR_PALETTE[palSlot], spr_buttonCornerPalLen);
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	}
	else
	{
		vramSetBankI(VRAM_I_LCD);
		dmaCopy((u8*)spr_buttonCornerPal, &VRAM_I_EXT_SPR_PALETTE[palSlot], spr_buttonCornerPalLen);
		vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	}
	mp3_fill_buffer();

	selectedBtn = 0;
	visible = false;
}

UISelectCross::~UISelectCross()
{
	for (int i=0; i<4; i++)
		oamClearSprite(oam, oamStart+i);
	oamFreeGfx(oam, spriteGfx);
}

void UISelectCross::setVisible(bool on)
{
	visible = on;
	for (int i=0; i<4; i++)
	{
		mp3_fill_buffer();
		oamSetHidden(oam, oamStart+i, !on);
	}
}

void UISelectCross::setPriority(int pr)
{
	for (int i=0; i<4; i++)
	{
		mp3_fill_buffer();
		oamSetPriority(oam, oamStart+i, pr);
	}
}

void UISelectCross::selectButton(UIButton* btn, int offset)
{
	selectedBtn = btn;

	setVisible(true);
	oamSetXY(oam, oamStart+0, btn->getX()-offset, btn->getY()-offset);
	oamSetXY(oam, oamStart+1, btn->getX()+btn->getW()-16+offset, btn->getY()-offset);
	oamSetXY(oam, oamStart+2, btn->getX()-offset, btn->getY()+btn->getH()-16+offset);
	oamSetXY(oam, oamStart+3, btn->getX()+btn->getW()-16+offset, btn->getY()+btn->getH()-16+offset);
}
