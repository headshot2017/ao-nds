#include "courtroom/background.h"

#include <dirent.h>
#include <stdio.h>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>

#include "mp3_shared.h"
#include "global.h"

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
	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 11, 2);
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

	shake(0, 0);
}

Background::~Background()
{
	for (int i=0; i<4*6; i++)
	{
		oamSet(&oamMain, i, 0, 0, 0, 0, SpriteSize_64x32, SpriteColorFormat_256Color, 0, 0, false, true, false, false, false);
		oamFreeGfx(&oamMain, deskGfx[i]);
	}
	bgHide(bgIndex);
	destroyBg();
}

void Background::destroyBg()
{
	if (!currentBg.empty())
	{
		delete[] currentBg["def"].mainBg.data;
		delete[] currentBg["pro"].mainBg.data;
		delete[] currentBg["wit"].mainBg.data;
		delete[] currentBg["hld"].mainBg.data;
		delete[] currentBg["hlp"].mainBg.data;
		delete[] currentBg["jud"].mainBg.data;
		mp3_fill_buffer();

		delete[] currentBg["def"].mainMap.data;
		delete[] currentBg["pro"].mainMap.data;
		delete[] currentBg["wit"].mainMap.data;
		delete[] currentBg["hld"].mainMap.data;
		delete[] currentBg["hlp"].mainMap.data;
		delete[] currentBg["jud"].mainMap.data;
		mp3_fill_buffer();

		delete[] currentBg["def"].mainPal.data;
		delete[] currentBg["pro"].mainPal.data;
		delete[] currentBg["wit"].mainPal.data;
		delete[] currentBg["hld"].mainPal.data;
		delete[] currentBg["hlp"].mainPal.data;
		delete[] currentBg["jud"].mainPal.data;
		mp3_fill_buffer();

		if (currentBg["def"].deskBg.data)
		{
			delete[] currentBg["def"].deskBg.data;
			delete[] currentBg["def"].deskPal.data;
		}
		if (currentBg["pro"].deskBg.data)
		{
			delete[] currentBg["pro"].deskBg.data;
			delete[] currentBg["pro"].deskPal.data;
		}
		if (currentBg["wit"].deskBg.data)
		{
			delete[] currentBg["wit"].deskBg.data;
			delete[] currentBg["wit"].deskPal.data;
		}
		if (currentBg["jud"].deskBg.data)
		{
			delete[] currentBg["jud"].deskBg.data;
			delete[] currentBg["jud"].deskPal.data;
		}

		mp3_fill_buffer();
	}
}

bool Background::setBg(const std::string& name)
{
	std::string bgPath = "/data/ao-nds/background/";
	bgPath += name;
	iprintf(bgPath.c_str());
	DIR* dir = opendir(bgPath.c_str());
	if (!dir) return false;
	closedir(dir);

	if (!deskTiles.load(bgPath + "/desk_tiles.cfg"))
		return false;

	destroyBg();

	// main bg data
	currentBg["def"].mainBg.data = readFile(bgPath + "/defenseempty.img.bin",      &currentBg["def"].mainBg.len);
	currentBg["pro"].mainBg.data = readFile(bgPath + "/prosecutorempty.img.bin",   &currentBg["pro"].mainBg.len);
	currentBg["wit"].mainBg.data = readFile(bgPath + "/witnessempty.img.bin",      &currentBg["wit"].mainBg.len);
	currentBg["hld"].mainBg.data = readFile(bgPath + "/helperstand.img.bin",       &currentBg["hld"].mainBg.len);
	currentBg["hlp"].mainBg.data = readFile(bgPath + "/prohelperstand.img.bin",    &currentBg["hlp"].mainBg.len);
	currentBg["jud"].mainBg.data = readFile(bgPath + "/judgestand.img.bin",        &currentBg["jud"].mainBg.len);

	currentBg["def"].mainMap.data = readFile(bgPath + "/defenseempty.map.bin",      &currentBg["def"].mainMap.len);
	currentBg["pro"].mainMap.data = readFile(bgPath + "/prosecutorempty.map.bin",   &currentBg["pro"].mainMap.len);
	currentBg["wit"].mainMap.data = readFile(bgPath + "/witnessempty.map.bin",      &currentBg["wit"].mainMap.len);
	currentBg["hld"].mainMap.data = readFile(bgPath + "/helperstand.map.bin",       &currentBg["hld"].mainMap.len);
	currentBg["hlp"].mainMap.data = readFile(bgPath + "/prohelperstand.map.bin",    &currentBg["hlp"].mainMap.len);
	currentBg["jud"].mainMap.data = readFile(bgPath + "/judgestand.map.bin",        &currentBg["jud"].mainMap.len);

	currentBg["def"].mainPal.data = readFile(bgPath + "/defenseempty.pal.bin",      &currentBg["def"].mainPal.len);
	currentBg["pro"].mainPal.data = readFile(bgPath + "/prosecutorempty.pal.bin",   &currentBg["pro"].mainPal.len);
	currentBg["wit"].mainPal.data = readFile(bgPath + "/witnessempty.pal.bin",      &currentBg["wit"].mainPal.len);
	currentBg["hld"].mainPal.data = readFile(bgPath + "/helperstand.pal.bin",       &currentBg["hld"].mainPal.len);
	currentBg["hlp"].mainPal.data = readFile(bgPath + "/prohelperstand.pal.bin",    &currentBg["hlp"].mainPal.len);
	currentBg["jud"].mainPal.data = readFile(bgPath + "/judgestand.pal.bin",        &currentBg["jud"].mainPal.len);

	// desk bg data
	currentBg["def"].deskBg.data = readFile(bgPath + "/defensedesk.img.bin",       &currentBg["def"].deskBg.len);
	currentBg["pro"].deskBg.data = readFile(bgPath + "/prosecutiondesk.img.bin",   &currentBg["pro"].deskBg.len);
	currentBg["wit"].deskBg.data = readFile(bgPath + "/stand.img.bin",             &currentBg["wit"].deskBg.len);
	currentBg["jud"].deskBg.data = readFile(bgPath + "/judgedesk.img.bin",         &currentBg["jud"].deskBg.len);
	currentBg["hld"].deskBg.data = 0;
	currentBg["hlp"].deskBg.data = 0;

	currentBg["def"].deskPal.data = readFile(bgPath + "/defensedesk.pal.bin",      &currentBg["def"].deskPal.len);
	currentBg["pro"].deskPal.data = readFile(bgPath + "/prosecutiondesk.pal.bin",  &currentBg["pro"].deskPal.len);
	currentBg["wit"].deskPal.data = readFile(bgPath + "/stand.pal.bin",            &currentBg["wit"].deskPal.len);
	currentBg["jud"].deskPal.data = readFile(bgPath + "/judgedesk.pal.bin",        &currentBg["jud"].deskPal.len);
	currentBg["hld"].deskPal.data = 0;
	currentBg["hlp"].deskPal.data = 0;

	currentSide.clear();
	setBgSide("def", true);

	return true;
}

void Background::setBgSide(const std::string& side, bool force)
{
	if (!currentBg.count(side) || (!force && side == currentSide))
		return;

	// copy main background
	dmaCopy(currentBg[side].mainBg.data, bgGetGfxPtr(bgIndex), currentBg[side].mainBg.len);
	dmaCopy(currentBg[side].mainMap.data, bgGetMapPtr(bgIndex), currentBg[side].mainMap.len);

	vramSetBankE(VRAM_E_LCD);
	dmaCopy(currentBg[side].mainPal.data, &VRAM_E_EXT_PALETTE[bgIndex][0], currentBg[side].mainPal.len);
	vramSetBankE(VRAM_E_BG_EXT_PALETTE);

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
				int iScreen = yScreen*4+x;

				deskGfxVisible[iScreen] = false;
			}
		}
	}

	if (currentBg[side].deskBg.data)
	{
		readDeskTiles(deskTiles.get(sideToDesk[side]), &horTiles, &verTiles);

		vramSetBankF(VRAM_F_LCD);
		dmaCopy(currentBg[side].deskPal.data, &VRAM_F_EXT_SPR_PALETTE[1], currentBg[side].deskPal.len); // copy palette
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

		for (int y=0; y<verTiles; y++)
		{
			int yScreen = 6-verTiles+y;

			for (int x=0; x<horTiles; x++)
			{
				int iScreen = yScreen*4+x;
				int i = y*4+x;

				// copy specific 64x32 tile from image data
				u8* offset = currentBg[side].deskBg.data + i * 64*32;
				dmaCopy(offset, deskGfx[iScreen], 64*32);
				deskGfxVisible[iScreen] = true;
			}
		}
	}

	currentSide = side;
	mp3_fill_buffer();
}

void Background::setVisible(bool on)
{
	visible = on;
	(on) ? bgShow(bgIndex) : bgHide(bgIndex);
}

void Background::shake(int force, int ticks)
{
	shakeForce = force;
	shakeTicks = ticks;
}

void Background::update()
{
	int xShake = 0;
	int yShake = 0;

	if (shakeTicks > 0)
	{
		shakeTicks--;
		xShake = -shakeForce + rand()%(shakeForce*2);
		yShake = -shakeForce + rand()%(shakeForce*2);
	}

	bgSetScroll(bgIndex, -xShake, -yShake);

	for (int i=0; i<4*6; i++)
	{
		int x = (i%4) * 64;
		int y = (i/4) * 32;

		oamSet(&oamMain, i, x+xShake, y+yShake, 2, 1, SpriteSize_64x32, SpriteColorFormat_256Color, deskGfx[i], -1, false, !deskGfxVisible[i] || !visible, false, false, false);
	}
}
