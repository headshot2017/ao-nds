#include "courtroom/background.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>

#include "mp3_shared.h"
#include "global.h"

std::unordered_map<std::string, std::string> sideToBg = {
    {"def", "defenseempty"},
    {"pro", "prosecutorempty"},
    {"wit", "witnessempty"},
    {"jud", "judgestand"},
    {"hld", "helperstand"},
    {"hlp", "prohelperstand"}
};

std::unordered_map<std::string, std::string> sideToDesk = {
    {"def", "defensedesk"},
    {"pro", "prosecutiondesk"},
    {"wit", "stand"},
    {"jud", "judgedesk"}
};

void readDeskTiles(const std::string& value, int* horizontal, int* vertical)
{
    std::size_t delimiterPos = value.find(",");
    if (delimiterPos == std::string::npos)
    {
        *horizontal = 0;
        *vertical = 0;
        return;
    }

    *horizontal = std::stoi(value.substr(0, delimiterPos));
    *vertical = std::stoi(value.substr(delimiterPos + 1));
}


Background::Background()
{
	//bgIndex = bgInit(3, BgType_ExRotation, BgSize_ER_256x256, 1, 2);
	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_512x256, 1, 1);
	bgSetPriority(bgIndex, 3);
	bgHide(bgIndex);
	visible = false;
	currBgGfxLen = 0;

	zooming = false;
	zoomScroll = 0;
	zoomScrollAdd = 0;

	currVertTiles = 0;

	for (int i=0; i<4*6; i++)
	{
		int x = (i%4) * 64;
		int y = (i/4) * 32;

		//deskGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_64x32, SpriteColorFormat_256Color);
		deskGfx[i] = 0;
		deskGfxVisible[i] = false;
		oamSet(&oamMain, i, x, y, 2, 1, SpriteSize_64x32, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
	}
}

Background::~Background()
{
	if (currBgGfxLen) dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), currBgGfxLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);

	for (int i=0; i<4*6; i++)
	{
		oamSet(&oamMain, i, 0, 0, 0, 0, SpriteSize_64x32, SpriteColorFormat_256Color, 0, 0, false, true, false, false, false);
		if (deskGfx[i]) oamFreeGfx(&oamMain, deskGfx[i]);
	}
	bgHide(bgIndex);
}

bool Background::setBg(const std::string& name)
{
	std::string bgPath = "/data/ao-nds/background/" + name;
	DIR* dir = opendir(bgPath.c_str());
	if (!dir) bgPath = "/data/ao-nds/background/gs4";
	else closedir(dir);

	currentBg = bgPath;

	if (!deskTiles.load(bgPath + "/desk_tiles.cfg"))
		return false;

	std::string newSide = currentSide;
	if (newSide.empty()) newSide = "def";
	setBgSide(newSide, true);

	return true;
}

void Background::setBgSide(const std::string& side, bool force)
{
	if (currentBg.empty() || !sideToBg.count(side) || (!force && !zooming && side == currentSide))
		return;

	if (zooming)
	{
		currentSide = "";
		bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_512x256, 1, 1);
		bgSetPriority(bgIndex, 3);
	}

	zooming = false;

	u32 bgGfxLen, bgMapLen, bgPalLen, deskPalLen;
	u8* bgGfx = readFile(currentBg + "/" + sideToBg[side] + ".img.bin", &bgGfxLen);
	u8* bgMap = readFile(currentBg + "/" + sideToBg[side] + ".map.bin", &bgMapLen);
	u8* bgPal = readFile(currentBg + "/" + sideToBg[side] + ".pal.bin", &bgPalLen);
	currBgGfxLen = bgGfxLen;

	// copy main background
	dmaCopy(bgGfx, bgGetGfxPtr(bgIndex), bgGfxLen);
	memcpy(bgGetMapPtr(bgIndex), bgMap, bgMapLen);

	vramSetBankE(VRAM_E_LCD);
	memcpy(&VRAM_E_EXT_PALETTE[bgIndex][0], bgPal, bgPalLen);
	vramSetBankE(VRAM_E_BG_EXT_PALETTE);

	BG_PALETTE[0] = ((u16*)bgPal)[0];

	mp3_fill_buffer();

	delete[] bgGfx;
	delete[] bgMap;
	delete[] bgPal;
	mp3_fill_buffer();

	u8* deskGfxImg = 0;
	u8* deskPal = 0;
	if (sideToDesk.count(side))
	{
		deskGfxImg = readFile(currentBg + "/" + sideToDesk[side] + ".img.bin");
		deskPal = readFile(currentBg + "/" + sideToDesk[side] + ".pal.bin", &deskPalLen);
	}

	// handle desk sprite

	int horTiles, verTiles;
	int horTilesOld, verTilesOld;

	readDeskTiles(deskTiles.get(sideToDesk[currentSide]), &horTilesOld, &verTilesOld);
	readDeskTiles(deskTiles.get(sideToDesk[side]), &horTiles, &verTiles);

	int gfxInUseOld = horTilesOld * verTilesOld;
	int gfxInUse = horTiles * verTiles;
	int maxHorTiles = std::max(horTilesOld, horTiles);
	int maxVerTiles = std::max(verTilesOld, verTiles);
	int minGfx = std::min(gfxInUseOld, gfxInUse);

	if (deskPal)
	{
		vramSetBankF(VRAM_F_LCD);
		memcpy(&VRAM_F_EXT_SPR_PALETTE[1], deskPal, deskPalLen); // copy palette
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
		delete[] deskPal;
	}

	for (int y=0; y<maxVerTiles; y++)
	{
		for (int x=0; x<maxHorTiles; x++)
		{
			mp3_fill_buffer();

			int i = y*4+x;

			if (i >= minGfx)
			{
				if (!deskGfx[i] && deskGfxImg)
				{
					deskGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_64x32, SpriteColorFormat_256Color);
				}
				else
				{
					if (deskGfx[i])
					{
						oamFreeGfx(&oamMain, deskGfx[i]);
						oamSetHidden(&oamMain, i, true);
						deskGfx[i] = 0;
					}
					continue;
				}
			}

			// copy specific 64x32 tile from image data
			u8* offset = deskGfxImg + i * 64*32;
			dmaCopy(offset, deskGfx[i], 64*32);
			deskGfxVisible[i] = true;

			oamSet(&oamMain, i, x*64+xOffset, y*32+yOffset + 192 - (verTiles*32), 2, 1, SpriteSize_64x32, SpriteColorFormat_256Color, deskGfx[i], -1, false, !deskGfx[i] || !visible, false, false, false);
		}
	}
	oamUpdate(&oamMain);
	delete[] deskGfxImg;

	currentSide = side;
	currVertTiles = verTiles;
	mp3_fill_buffer();
}

void Background::setZoom(bool scrollLeft, bool force)
{
	int newScrollAdd = (scrollLeft) ? 12 : -12;
	if (!force && zooming && newScrollAdd == zoomScrollAdd)
		return;

	if (!zooming)
	{
		bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 1, 1);
		bgSetPriority(bgIndex, 3);
	}

	zooming = true;
	zoomScroll = 0;
	zoomScrollAdd = newScrollAdd;

	u32 bgGfxLen, bgMapLen, bgPalLen;
	u8* bgGfx = readFile("/data/ao-nds/misc/speedlines.img.bin", &bgGfxLen);
	u8* bgMap = readFile("/data/ao-nds/misc/speedlines.map.bin", &bgMapLen);
	u8* bgPal = readFile("/data/ao-nds/misc/speedlines.pal.bin", &bgPalLen);
	currBgGfxLen = bgGfxLen;

	// copy main background
	dmaCopy(bgGfx, bgGetGfxPtr(bgIndex), bgGfxLen);
	memcpy(bgGetMapPtr(bgIndex), bgMap, bgMapLen);

	vramSetBankE(VRAM_E_LCD);
	memcpy(&VRAM_E_EXT_PALETTE[bgIndex][0], bgPal, bgPalLen);
	vramSetBankE(VRAM_E_BG_EXT_PALETTE);

	BG_PALETTE[0] = ((u16*)bgPal)[0];

	// hide the desk tiles
	int horTiles, verTiles;
	readDeskTiles(deskTiles.get(sideToDesk[currentSide]), &horTiles, &verTiles);

	for (int i=0; i<horTiles*verTiles; i++)
	{
		if (!deskGfx[i]) continue;
		mp3_fill_buffer();

		oamFreeGfx(&oamMain, deskGfx[i]);
		oamSetHidden(&oamMain, i, true);
		deskGfx[i] = 0;
	}

	currentSide = "";
	oamUpdate(&oamMain);
}

void Background::setVisible(bool on)
{
	visible = on;
	(on) ? bgShow(bgIndex) : bgHide(bgIndex);
}

void Background::update()
{
	if (zooming)
	{
		zoomScroll += zoomScrollAdd;
		bgSetScroll(bgIndex, -xOffset+zoomScroll, -yOffset);
	}
	else
		bgSetScroll(bgIndex, -xOffset, -yOffset);

	for (int i=0; i<4*6; i++)
	{
		int x = (i%4) * 64;
		int y = (i/4) * 32;

		oamSet(&oamMain, i, x+xOffset, y+yOffset + 192 - (currVertTiles*32), 2, 1, SpriteSize_64x32, SpriteColorFormat_256Color, deskGfx[i], -1, false, !deskGfx[i] || !visible, false, false, false);
	}
}
