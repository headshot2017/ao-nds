#include "courtroom/background.h"

#include <dirent.h>
#include <stdio.h>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/math.h>
#include <nds/timers.h>

#include "courtroom/courtroom.h"
#include "mini/ini.h"
#include "mp3_shared.h"
#include "global.h"
#include "content.h"

//the speed of the timer when using ClockDivider_1024
#define TIMER_SPEED div32(BUS_CLOCK,1024)

static std::unordered_map<std::string, std::string> sideToBg = {
    {"def", "defenseempty"},
    {"pro", "prosecutorempty"},
    {"wit", "witnessempty"},
    {"jud", "judgestand"},
    {"hld", "helperstand"},
    {"hlp", "prohelperstand"},
    {"jur", "jurystand"},
    {"sea", "seancestand"},
};

static std::unordered_map<std::string, std::string> sideToDesk = {
    {"def", "defensedesk"},
    {"pro", "prosecutiondesk"},
    {"wit", "stand"},
    {"jud", "judgedesk"},
    {"jur", "jurydesk"},
};

static void readDeskTiles(const std::string& value, int* horizontal, int* vertical)
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

// https://github.com/warrenm/AHEasing/blob/82d85f7183e207bf4405d427a723a8a85406ddf4/AHEasing/easing.c#L81
static int easeInOutCubic(int t)
{
	if (t < 2048)
		return mulf32(mulf32(mulf32(inttof32(4), t), t), t);
	else
	{
		int f = (mulf32(inttof32(2), t) - inttof32(2));
		return mulf32(mulf32(mulf32(2048, f), f), f) + inttof32(1);
	}
}


void FullCourtInfo::loadPosition(int index, int offset)
{
	if (index < 0) return;

	camOffset = offset;
	lastState = true;

	u16* gfxPtr = bgGetGfxPtr(index);
	u16* mapPtr = bgGetMapPtr(index);
	for (int y=0; y<24; y++)
	{
		for (int x=0; x<33; x++)
		{
			int xx = (x+camOffset/8);
			int mapOffset = (32*32 * ((xx%64)/32)) + (y * 32 + (xx%32));
			int courtGfxInd = xx/33;
			int gfxOffset = (y*264*4) + ((xx%33)*8*4);

			mapPtr[mapOffset] = (y * 33 + (xx%33));
			dmaCopy(courtGfx[courtGfxInd]+gfxOffset, gfxPtr+gfxOffset, 64);
		}
	}
}

void FullCourtInfo::startScroll(int index, const std::string& sideBefore, const std::string& sideAfter)
{
	bgIndex = index;
	camStart = camOffset;
	camEnd = sideInfo[sideAfter].origin - 128;
	camTimer = 0;
	camTimerMax = sideInfo[sideBefore].slideMS[sideAfter];

	// load new desk tiles
	readDeskTiles(m_pCourt->getBackground()->deskTiles.get(sideToDesk[sideAfter]), &newDeskHorTiles, &newDeskVertTiles);

	if (sideToDesk.count(sideAfter))
	{
		if (newDeskGfx) delete[] newDeskGfx;
		newDeskGfx = 0;
		if (newDeskPal) delete[] newDeskPal;
		newDeskPal = 0;

		std::string& currentBg = m_pCourt->getBackground()->currentBg;
		newDeskGfx = readFile(currentBg + "/" + sideToDesk[sideAfter] + ".img.bin");
		newDeskPal = readFile(currentBg + "/" + sideToDesk[sideAfter] + ".pal.bin");
	}

	// load target character
	MSchatStruct* currIC = m_pCourt->getIC();
	std::string offsetStr = currIC->selfOffset;
	int offsetX = 0;
	int offsetY = 0;

	if (offsetStr.find("&"))
	{
		offsetX = std::stoi(argumentAt(offsetStr, 0, '&'));
		offsetY = std::stoi(argumentAt(offsetStr, 1, '&'));
	}
	else
		offsetX = std::stoi(offsetStr);

	m_pCourt->getCharacter(1)->setOffsets(offsetX/100.f*256, offsetY/100.f*192);
	m_pCourt->getCharacter(1)->setFlip(currIC->flip);

	bool isAnim = Content::exists("characters/" + currIC->charname + "/(a)" + currIC->emote + ".img.bin");
	std::string prefix;
	if (isAnim) prefix = "(a)";

	m_pCourt->getCharacter(1)->setCharImage(currIC->charname, prefix + currIC->emote);
	m_pCourt->getCharacter(1)->setVisible(false);
	oamUpdate(&oamMain);

	lastState = true;
	active = true;
	halfway = false;

	timerStop(3);
	timerStart(3, ClockDivider_1024, 0, NULL);
}

void FullCourtInfo::clean()
{
	if (courtPalette) delete[] courtPalette;
	courtPalette = 0;

	for (int i=0; i<parts; i++)
	{
		if (!courtGfx[i]) continue;
		delete[] courtGfx[i];
	}

	if (courtGfx) delete[] courtGfx;
	courtGfx = 0;

	if (newDeskGfx) delete[] newDeskGfx;
	newDeskGfx = 0;
	if (newDeskPal) delete[] newDeskPal;
	newDeskPal = 0;

	camOffset = 0;
	parts = 0;
	lastState = false;
	halfway = false;
	active = false;
}

void FullCourtInfo::update()
{
	if (!parts || bgIndex < 0 || !active || !(TIMER_CR(3) & TIMER_ENABLE))
		return;

	if (camTimer == camTimerMax)
	{
		active = false;
		timerStop(3);
		m_pCourt->getCharacter(0)->setCamOffset(0);
		m_pCourt->getCharacter(1)->setCamOffset(0);
		m_pCourt->getBackground()->setDeskOffset(0);

		if (newDeskGfx) delete[] newDeskGfx;
		newDeskGfx = 0;
		if (newDeskPal) delete[] newDeskPal;
		newDeskPal = 0;

		if (onScrollFinished)
			onScrollFinished(pUserData);
		return;
	}

	u32 elapsed = timerElapsed(3);
	u32 ms = f32toint(mulf32(divf32(inttof32(elapsed), inttof32(TIMER_SPEED)), inttof32(1000)));
	camTimer = (camTimer+ms < camTimerMax) ? camTimer+ms : camTimerMax;

	int d = easeInOutCubic( divf32(inttof32(camTimer), inttof32(camTimerMax)) );
	camOffset = camStart + f32toint(mulf32(inttof32(camEnd - camStart), d));
	loadPosition(bgIndex, camOffset);

	if (camTimer >= camTimerMax>>1 && !halfway)
	{
		halfway = true;
		m_pCourt->getBackground()->setDesk(newDeskGfx, newDeskPal, newDeskHorTiles, newDeskVertTiles);
	}

	int x0 = camStart - camOffset;
	int x1 = camEnd - camOffset;
	m_pCourt->getCharacter(0)->setVisible(x0+256 > 0 && x0 < 256);
	m_pCourt->getCharacter(1)->setVisible(x1+256 > 0 && x1 < 256);
	m_pCourt->getCharacter(0)->setCamOffset(x0);
	m_pCourt->getCharacter(1)->setCamOffset(x1);

	int deskX = (!halfway) ? x0 : x1;
	m_pCourt->getBackground()->setDeskVisible(deskX+256 > 0 && deskX < 256);
	m_pCourt->getBackground()->setDeskOffset(deskX);
}

////////////////////////////////////////////////////////////////////////////////////////////////

Background::Background(Courtroom* court)
{
	//bgIndex = bgInit(3, BgType_ExRotation, BgSize_ER_256x256, 1, 2);
	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_512x256, 0, 1);
	bgSetPriority(bgIndex, 3);
	bgHide(bgIndex);
	visible = false;
	deskVisible = true;
	currBgGfxLen = 0;

	zooming = false;
	zoomScroll = 0;
	zoomScrollAdd = 0;

	currVertTiles = 0;
	deskCamOffset = 0;

	for (int i=0; i<4*6; i++)
	{
		int x = (i%4) * 64;
		int y = (i/4) * 32;

		deskGfx[i] = 0;
		oamSet(&oamMain, i, x, y, 2, 1, SpriteSize_64x32, SpriteColorFormat_256Color, 0, -1, false, true, false, false, false);
	}

	m_pCourt = court;
	fullCourt.m_pCourt = court;
}

Background::~Background()
{
	if (currBgGfxLen) dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), currBgGfxLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);

	fullCourt.clean();

	for (int i=0; i<4*6; i++)
	{
		oamSet(&oamMain, i, 0, 0, 0, 0, SpriteSize_64x32, SpriteColorFormat_256Color, 0, 0, false, true, false, false, false);
		if (deskGfx[i]) oamFreeGfx(&oamMain, deskGfx[i]);
	}
	bgHide(bgIndex);
}

bool Background::setBg(const std::string& name)
{
	std::string bgPath;
	if (!Content::exists("background/"+name, bgPath))
		bgPath = "/data/ao-nds/background/gs4";

	if (!deskTiles.load(bgPath + "/desk_tiles.cfg"))
		return false;

	currentBg = bgPath;

	fullCourt.clean();

	mINI::INIFile file(bgPath + "/design.ini");
	mINI::INIStructure ini;
	if (file.read(ini))
	{
		// all court gfx's share the same palette
		// if loading this fails, use normal background
		fullCourt.courtPalette = (u16*)readFile(bgPath + "/court0.pal.bin", &fullCourt.paletteLen);
		if (fullCourt.courtPalette)
		{
			// if the original image was of a higher resolution (e.g. 5184x768), the origins must be scaled down
			int originalScale = std::stoi(ini["nds"]["original_scale"]);
			fullCourt.parts = std::stoi(ini["nds"]["total_parts"]);

			// load ALL the gfx parts
			fullCourt.courtGfx = new u16*[fullCourt.parts];
			for (int i=0; i<fullCourt.parts; i++)
			{
				std::string filename = "/court" + std::to_string(i) + ".img.bin";
				fullCourt.courtGfx[i] = (u16*)readFile(bgPath + filename);
			}

			fullCourt.sideInfo.clear();
			fullCourt.sideInfo["def"] = {};
			fullCourt.sideInfo["wit"] = {};
			fullCourt.sideInfo["pro"] = {};

			fullCourt.sideInfo["def"].origin =         std::stoi(ini["court:def"]["origin"]) / originalScale;
			fullCourt.sideInfo["def"].slideMS["wit"] = std::stoi(ini["court:def"]["slide_ms_wit"]);
			fullCourt.sideInfo["def"].slideMS["pro"] = std::stoi(ini["court:def"]["slide_ms_pro"]);

			fullCourt.sideInfo["wit"].origin =         std::stoi(ini["court:wit"]["origin"]) / originalScale;
			fullCourt.sideInfo["wit"].slideMS["def"] = std::stoi(ini["court:wit"]["slide_ms_def"]);
			fullCourt.sideInfo["wit"].slideMS["pro"] = std::stoi(ini["court:wit"]["slide_ms_pro"]);

			fullCourt.sideInfo["pro"].origin =         std::stoi(ini["court:pro"]["origin"]) / originalScale;
			fullCourt.sideInfo["pro"].slideMS["def"] = std::stoi(ini["court:pro"]["slide_ms_def"]);
			fullCourt.sideInfo["pro"].slideMS["wit"] = std::stoi(ini["court:pro"]["slide_ms_wit"]);
		}
	}

	std::string newSide = currentSide;
	if (newSide.empty()) newSide = "def";
	setBgSide(newSide, true, false, true);

	return true;
}

void Background::setBgSide(const std::string& side, bool showDesk, bool pan, bool force)
{
	if (currentBg.empty() || !sideToBg.count(side) || (!force && !zooming && side == currentSide))
		return;

	if (zooming)
	{
		currentSide = "";
		bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_512x256, 0, 1);
		bgSetPriority(bgIndex, 3);
	}

	if (fullCourt.parts && fullCourt.sideInfo.count(side))
	{
		if (zooming || !fullCourt.lastState)
		{
			vramSetBankE(VRAM_E_LCD);
			dmaCopy(fullCourt.courtPalette, &VRAM_E_EXT_PALETTE[bgIndex][0], fullCourt.paletteLen);
			vramSetBankE(VRAM_E_BG_EXT_PALETTE);
			BG_PALETTE[0] = fullCourt.courtPalette[0];
		}

		if (pan)
		{
			if (!fullCourt.lastState)
				fullCourt.loadPosition(bgIndex, fullCourt.sideInfo[side].origin - 128);
			else if (fullCourt.sideInfo.count(currentSide))
				fullCourt.startScroll(bgIndex, currentSide, side);
		}
		else
			fullCourt.loadPosition(bgIndex, fullCourt.sideInfo[side].origin - 128);
	}
	else
	{
		fullCourt.camOffset = 0;
		fullCourt.lastState = false;

		u32 bgGfxLen, bgMapLen, bgPalLen;
		u8* bgGfx = readFile(currentBg + "/" + sideToBg[side] + ".img.bin", &bgGfxLen);
		u8* bgMap = readFile(currentBg + "/" + sideToBg[side] + ".map.bin", &bgMapLen);
		u8* bgPal = readFile(currentBg + "/" + sideToBg[side] + ".pal.bin", &bgPalLen);

		if (bgGfx && bgMap && bgPal)
		{
			currBgGfxLen = bgGfxLen;

			// copy main background
			dmaCopy(bgGfx, bgGetGfxPtr(bgIndex), bgGfxLen);
			dmaCopy(bgMap, bgGetMapPtr(bgIndex), bgMapLen);

			vramSetBankE(VRAM_E_LCD);
			dmaCopy(bgPal, &VRAM_E_EXT_PALETTE[bgIndex][0], bgPalLen);
			vramSetBankE(VRAM_E_BG_EXT_PALETTE);

			BG_PALETTE[0] = ((u16*)bgPal)[0];

			mp3_fill_buffer();
		}

		if (bgGfx) delete[] bgGfx;
		if (bgMap) delete[] bgMap;
		if (bgPal) delete[] bgPal;
		mp3_fill_buffer();
	}

	zooming = false;

	// handle desk sprite
	if (!fullCourt.active)
	{
		if (showDesk)
		{
			int horTiles, verTiles;
			readDeskTiles(deskTiles.get(sideToDesk[side]), &horTiles, &verTiles);

			u8* deskGfxImg = 0;
			u8* deskPal = 0;
			if (sideToDesk.count(side))
			{
				deskGfxImg = readFile(currentBg + "/" + sideToDesk[side] + ".img.bin");
				deskPal = readFile(currentBg + "/" + sideToDesk[side] + ".pal.bin");
			}

			setDesk(deskGfxImg, deskPal, horTiles, verTiles);
		}
		else
			cleanDesk();
	}

	currentSide = side;
	mp3_fill_buffer();
}

void Background::cleanDesk()
{
	for (int i=0; i<4*6; i++)
	{
		if (deskGfx[i]) oamFreeGfx(&oamMain, deskGfx[i]);
		oamSetHidden(&oamMain, i, true);
		deskGfx[i] = 0;
	}
}

void Background::setDesk(u8* gfx, u8* pal, int horTiles, int verTiles)
{
	cleanDesk();

	if (pal)
	{
		vramSetBankF(VRAM_F_LCD);
		dmaCopy(pal, &VRAM_F_EXT_SPR_PALETTE[1], 512); // copy palette
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);
	}

	if (gfx)
	{
		for (int y=0; y<verTiles; y++)
		{
			for (int x=0; x<horTiles; x++)
			{
				mp3_fill_buffer();

				int i = y*4+x;

				deskGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_64x32, SpriteColorFormat_256Color);
				if (!deskGfx[i])
				{
					oamSetHidden(&oamMain, i, true);
					continue;
				}

				// copy specific 64x32 tile from image data
				u8* offset = gfx + i * 64*32;
				dmaCopy(offset, deskGfx[i], 64*32);

				oamSet(&oamMain, i, x*64+xOffset, y*32+yOffset + 192 - (verTiles*32), 2, 1, SpriteSize_64x32, SpriteColorFormat_256Color, deskGfx[i], -1, false, !deskGfx[i] || !visible, false, false, false);
			}
		}
	}

	oamUpdate(&oamMain);
	currVertTiles = verTiles;
}

void Background::setZoom(bool scrollLeft, bool force)
{
	int newScrollAdd = (scrollLeft) ? 12 : -12;
	if (!force && zooming && newScrollAdd == zoomScrollAdd)
		return;

	if (!zooming)
	{
		bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
		bgSetPriority(bgIndex, 3);
	}

	fullCourt.lastState = false;
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
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), bgMapLen);

	vramSetBankE(VRAM_E_LCD);
	dmaCopy(bgPal, &VRAM_E_EXT_PALETTE[bgIndex][0], bgPalLen);
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

void Background::setDeskVisible(bool on)
{
	deskVisible = on;
	for (int i=0; i<4*6; i++)
		oamSetHidden(&oamMain, i, !on);
}

void Background::setDeskOffset(int camOffset)
{
	deskCamOffset = camOffset;
}

void Background::setOnScrollFinishedCallback(voidCallback newCB, void* userdata)
{
	fullCourt.onScrollFinished = newCB;
	fullCourt.pUserData = userdata;
}

void Background::update()
{
	if (zooming)
	{
		zoomScroll += zoomScrollAdd;
		bgSetScroll(bgIndex, -xOffset+zoomScroll, -yOffset);
	}
	else
	{
		fullCourt.update();
		bgSetScroll(bgIndex, -xOffset+fullCourt.camOffset, -yOffset);
	}

	for (int i=0; i<4*6; i++)
	{
		int x = (i%4) * 64;
		int y = (i/4) * 32;

		oamSet(&oamMain, i, x+xOffset+deskCamOffset, y+yOffset + 192 - (currVertTiles*32), 2, 1, SpriteSize_64x32, SpriteColorFormat_256Color, deskGfx[i], -1, false, !deskGfx[i] || !deskVisible, false, false, false);
	}
}
