#ifndef BACKGROUND_H_INCLUDED
#define BACKGROUND_H_INCLUDED

// courtroom background
// main BG uses slot 0, 512x256
// desk BG uses OAM slots 0-23

#include <nds/ndstypes.h>
#include <string>
#include <unordered_map>

#include "cfgFile.h"

struct FullCourtInfo
{
	struct CourtSideInfo
	{
		int origin;
		std::unordered_map<std::string, int> slideMS;
	};

	int parts;
	std::unordered_map<std::string, CourtSideInfo> sideInfo;

	u16* courtPalette;
	u32 paletteLen;
	u16** courtGfx;

	bool lastState;
	int camOffset;
};

class Background
{
	std::string currentBg;
	std::string currentSide;
	u32 currBgGfxLen;
	FullCourtInfo fullCourt;
	cfgFile deskTiles;
	int bgIndex;
	bool visible;


	bool zooming;
	int zoomScroll;
	int zoomScrollAdd;

	u16* deskGfx[4*6];
	bool deskGfxVisible[4*6];
	int currVertTiles;

	int xOffset;
	int yOffset;

	void loadBgPosition(int camOffset);
	void cleanFullCourt();

public:
	Background();
	~Background();

	bool isZoom() {return zooming;}

	void setOffsets(int x, int y) {xOffset = x; yOffset = y;}
	bool setBg(const std::string& name);
	void setBgSide(const std::string& side, bool showDesk, bool force=false);
	void setZoom(bool scrollLeft, bool force=false);
	void setVisible(bool on);

	void update();
};

#endif // BACKGROUND_H_INCLUDED
