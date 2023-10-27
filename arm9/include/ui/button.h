#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include <string>

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/input.h>

#include "global.h"

class UIButton
{
	u16** spriteGfx;
	int spriteHorTiles;
	int spriteVertTiles;
	SpriteSize spriteSize;
	OamState* oam;
	int oamStart;
	int paletteSlot;
	touchPosition touchPos;

	int x;
	int y;
	int w;
	int h;
	int sprW;
	int sprH;
	bool visible;
	bool pressing;
	bool hflip;
	bool vflip;
	u32 assignedKey;

	u8* currData;
	u8* currPal;

	cbInfo callbacks[2];

public:
	enum
	{
		PRESSED,
		RELEASED
	};

	UIButton(OamState* chosenOam, std::string file, int oamStartInd, int horTiles, int vertTiles, SpriteSize sprSize, int xPos, int yPos, int width, int height, int sprWidth, int sprHeight, int palSlot);
	~UIButton();

	void setImage(std::string file, int sprWidth, int sprHeight, int palSlot);
	void setFrame(int frame);
	void setVisible(bool on);
	void setPos(int xPos, int yPos);
	void setPriority(int pr);
	void setFlip(bool h, bool v);
	void forceRelease();
	void darken();
	void restorePalette();
	void unloadRAM(bool deletePal=true);

	void connect(voidCallback cb, void* p, int cbType=PRESSED) {callbacks[cbType].cb = cb; callbacks[cbType].pUserData = p;}
	void assignKey(u32 k) {assignedKey = k;}

	int getX() {return x;}
	int getY() {return y;}
	int getW() {return w;}
	int getH() {return h;}
	bool isHeld() {return pressing;}
	int nextOamInd() {return oamStart + (spriteHorTiles*spriteVertTiles);}

	void updateInput();
};

#endif // BUTTON_H_INCLUDED
