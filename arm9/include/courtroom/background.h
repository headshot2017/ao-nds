#ifndef BACKGROUND_H_INCLUDED
#define BACKGROUND_H_INCLUDED

// courtroom background
// main BG uses slot 0, 256x256
// desk BG uses OAM slots 0-23

#include <nds/ndstypes.h>
#include <string>
#include <unordered_map>

#include "cfgFile.h"

struct BgData
{
	u8* data;
	u32 len;
};

struct LoadedBg
{
	BgData mainBg;
	BgData mainMap;
	BgData mainPal;
	BgData deskBg;
	BgData deskPal;
};

class Background
{
	std::unordered_map<std::string, LoadedBg> currentBg;
	std::string currentSide;
	cfgFile deskTiles;
	int bgIndex;
	bool visible;

	u16* deskGfx[4*6];
	bool deskGfxVisible[4*6];

	int xOffset;
	int yOffset;

	void destroyBg();

public:
	Background();
	~Background();

	void setOffsets(int x, int y) {xOffset = x; yOffset = y;}
	bool setBg(const std::string& name);
	void setBgSide(const std::string& side, bool force=false);
	void setVisible(bool on);

	void update();
};

#endif // BACKGROUND_H_INCLUDED
