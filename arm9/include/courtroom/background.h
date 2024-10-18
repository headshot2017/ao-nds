#ifndef BACKGROUND_H_INCLUDED
#define BACKGROUND_H_INCLUDED

// courtroom background
// main BG uses slot 0, 512x256
// desk BG uses OAM slots 0-23

#include <nds/ndstypes.h>
#include <string>
#include <unordered_map>

#include "cfgFile.h"
#include "global.h"

struct FullCourtInfo
{
	struct CourtSideInfo
	{
		int origin;
		std::unordered_map<std::string, int> slideMS;
	};

	int parts;
	std::unordered_map<std::string, CourtSideInfo> sideInfo;

	int bgIndex;
	u16* courtPalette;
	u32 paletteLen;
	u16** courtGfx;

	bool lastState;
	int camOffset;
	u32 camTimer;
	u32 camTimerMax;
	int camStart;
	int camEnd;

	void* pUserData;
	voidCallback onScrollFinished;

	////////////////////////////////////////////////////////////////
	FullCourtInfo() :
		parts(0),
		bgIndex(-1),
		courtPalette(0),
		courtGfx(0),
		lastState(false),
		camOffset(0),
		camTimer(0),
		camTimerMax(0),
		camStart(0),
		camEnd(0),
		pUserData(0),
		onScrollFinished(0) {}

	void loadPosition(int index, int offset);
	void startScroll(int index, const std::string& sideBefore, const std::string& sideAfter);
	void clean();
	void update();
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

public:
	Background();
	~Background();

	bool isZoom() {return zooming;}
	bool isFullCourt() {return fullCourt.parts;}

	void setOffsets(int x, int y) {xOffset = x; yOffset = y;}
	bool setBg(const std::string& name);
	bool setBgSide(const std::string& side, bool showDesk, bool pan, bool force=false);
	void setZoom(bool scrollLeft, bool force=false);
	void setVisible(bool on);
	void setOnScrollFinishedCallback(voidCallback newCB, void* userdata);

	void update();
};

#endif // BACKGROUND_H_INCLUDED
