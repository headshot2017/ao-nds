#include "courtroom/background.h"

#include <dirent.h>
#include <stdio.h>

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
	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_512x256, 12, 2);
	bgSetPriority(bgIndex, 3);
	bgHide(bgIndex);
	visible = false;

	for (int i=0; i<4*6; i++)
	{
		int x = (i%4) * 64;
		int y = (i/4) * 32;

		deskGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_64x32, SpriteColorFormat_256Color);
		deskGfxVisible[i] = false;
		oamSet(&oamMain, i, x, y, 2, 1, SpriteSize_64x32, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
	}
}

Background::~Background()
{
	for (int i=0; i<4*6; i++)
	{
		oamSet(&oamMain, i, 0, 0, 0, 0, SpriteSize_64x32, SpriteColorFormat_256Color, 0, 0, false, true, false, false, false);
		oamFreeGfx(&oamMain, deskGfx[i]);
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

	if (currentSide.empty()) currentSide = "def";
	setBgSide(currentSide, true);

	return true;
}

void Background::setBgSide(const std::string& side, bool force)
{
	if (currentBg.empty() || !sideToBg.count(side) || (!force && side == currentSide))
		return;

	u32 bgGfxLen, bgMapLen, bgPalLen, deskPalLen;
	u8* bgGfx = readFile(currentBg + "/" + sideToBg[side] + ".img.bin", &bgGfxLen);
	u8* bgMap = readFile(currentBg + "/" + sideToBg[side] + ".map.bin", &bgMapLen);
	u8* bgPal = readFile(currentBg + "/" + sideToBg[side] + ".pal.bin", &bgPalLen);

	// copy main background
	dmaCopy(bgGfx, bgGetGfxPtr(bgIndex), bgGfxLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), bgMapLen);

	vramSetBankE(VRAM_E_LCD);
	dmaCopy(bgPal, &VRAM_E_EXT_PALETTE[bgIndex][0], bgPalLen);
	vramSetBankE(VRAM_E_BG_EXT_PALETTE);

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
	if (!currentSide.empty())
	{
		readDeskTiles(deskTiles.get(sideToDesk[currentSide]), &horTiles, &verTiles);

		// make the old ones blank first
		for (int y=0; y<verTiles; y++)
		{
			int yScreen = 6-verTiles+y;

			for (int x=0; x<horTiles; x++)
			{
				mp3_fill_buffer();

				int iScreen = yScreen*4+x;

				deskGfxVisible[iScreen] = false;
			}
		}
	}

	if (deskGfxImg)
	{
		readDeskTiles(deskTiles.get(sideToDesk[side]), &horTiles, &verTiles);

		vramSetBankF(VRAM_F_LCD);
		dmaCopy(deskPal, &VRAM_F_EXT_SPR_PALETTE[1], deskPalLen); // copy palette
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

		for (int y=0; y<verTiles; y++)
		{
			int yScreen = 6-verTiles+y;

			for (int x=0; x<horTiles; x++)
			{
				mp3_fill_buffer();

				int iScreen = yScreen*4+x;
				int i = y*4+x;

				// copy specific 64x32 tile from image data
				u8* offset = deskGfxImg + i * 64*32;
				dmaCopy(offset, deskGfx[iScreen], 64*32);
				deskGfxVisible[iScreen] = true;
			}
		}

		delete[] deskGfxImg;
		delete[] deskPal;
	}

	currentSide = side;
	mp3_fill_buffer();
}

void Background::setVisible(bool on)
{
	visible = on;
	(on) ? bgShow(bgIndex) : bgHide(bgIndex);
}

void Background::update()
{
	bgSetScroll(bgIndex, -xOffset, -yOffset);

	for (int i=0; i<4*6; i++)
	{
		int x = (i%4) * 64;
		int y = (i/4) * 32;

		oamSet(&oamMain, i, x+xOffset, y+yOffset, 2, 1, SpriteSize_64x32, SpriteColorFormat_256Color, deskGfx[i], -1, false, !deskGfxVisible[i] || !visible, false, false, false);
	}
}
