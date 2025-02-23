#include "ui/uicourt.h"

#include <nds/arm9/background.h>
#include <nds/dma.h>

UISubScreen::~UISubScreen()
{
	if (bgPal) mem_free(bgPal);

	if (bgTilesLen) dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);
}

void UISubScreen::loadBg(std::string filename, bool deletePal)
{
	u8* bgTiles = readFile(filename+".img.bin", &bgTilesLen);
	u8* bgMap = readFile(filename+".map.bin");
	bgPal = readFile(filename+".pal.bin");

	dmaCopy(bgTiles, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), 1536);
	dmaCopy(bgPal, BG_PALETTE_SUB, 512);

	mem_free(bgTiles);
	mem_free(bgMap);
	if (deletePal)
	{
		mem_free(bgPal);
		bgPal = 0;
	}
}
