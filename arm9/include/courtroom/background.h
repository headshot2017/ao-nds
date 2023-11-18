#ifndef BACKGROUND_H_INCLUDED
#define BACKGROUND_H_INCLUDED

// courtroom background
// main BG uses slot 0, 512x256
// desk BG uses OAM slots 0-23

#include <nds/ndstypes.h>
#include <string>
#include <unordered_map>

#include "cfgFile.h"

class Background
{
	std::string currentBg;
	std::string currentSide;
	u32 currBgGfxLen;
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

public:
	Background();
	~Background();

	bool isZoom() {return zooming;}

	void setOffsets(int x, int y) {xOffset = x; yOffset = y;}
	bool setBg(const std::string& name);
	void setBgSide(const std::string& side, bool force=false);
	void setZoom(bool scrollLeft, bool force=false);
	void setVisible(bool on);

	void update();
};

#endif // BACKGROUND_H_INCLUDED
