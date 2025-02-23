#include "ui/selectcross.h"

#include <nds/dma.h>

#include "libadx.h"
#include "global.h"
#include "mem.h"

UISelectCross::UISelectCross(OamState* chosenOam, int oamStartInd, int palSlot)
{
	oam = chosenOam;
	oamStart = oamStartInd;

	u8* tiles = readFile("/data/ao-nds/ui/spr_buttonCorner.img.bin");
	u8* pal = readFile("/data/ao-nds/ui/spr_buttonCorner.pal.bin");
	spriteGfx = oamAllocateGfx(oam, SpriteSize_16x16, SpriteColorFormat_256Color);
	dmaCopy(tiles, spriteGfx, 16*16);
	adx_update();

	for (int i=0; i<4; i++)
	{
		// order: no flip, hflip, vflip, both flips
		oamSet(oam, oamStart+i, 0, 0, 0, palSlot, SpriteSize_16x16, SpriteColorFormat_256Color, spriteGfx, -1, false, false, (i&1), (i&2), false);
	}

	// copy palette to ext palette vram slot
	if (oam == &oamMain)
	{
		vramSetBankF(VRAM_F_LCD);
		dmaCopy(pal, &VRAM_F_EXT_SPR_PALETTE[palSlot], 512);
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	}
	else
	{
		vramSetBankI(VRAM_I_LCD);
		dmaCopy(pal, &VRAM_I_EXT_SPR_PALETTE[palSlot], 512);
		vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
	}
	adx_update();

	mem_free(tiles);
	mem_free(pal);
	adx_update();

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
		adx_update();
		oamSetHidden(oam, oamStart+i, !on);
	}
}

void UISelectCross::setPriority(int pr)
{
	for (int i=0; i<4; i++)
	{
		adx_update();
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
