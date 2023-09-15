#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

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
	touchPosition touchPos;

	int x;
	int y;
	int w;
	int h;
	int sprW;
	int sprH;
	bool visible;
	bool pressing;

	cbInfo callbacks[2];

public:
	enum
	{
		PRESSED,
		RELEASED
	};

	UIButton(OamState* chosenOam, u8* data, u8* palData, int oamStartInd, int horTiles, int vertTiles, SpriteSize sprSize, int xPos, int yPos, int width, int height, int sprWidth, int sprHeight, int palSlot);
	~UIButton();

	void setImage(u8* data, u8* palData, int sprWidth, int sprHeight, int palSlot);
	void setVisible(bool on);
	void setPos(int xPos, int yPos);
	void setPriority(int pr);
	void forceRelease();
	void connect(voidCallback cb, void* p, int cbType=PRESSED) {callbacks[cbType].cb = cb; callbacks[cbType].pUserData = p;}

	int getX() {return x;}
	int getY() {return y;}
	int getW() {return w;}
	int getH() {return h;}
	bool isHeld() {return pressing;}
	int nextOamInd() {return oamStart + (spriteHorTiles*spriteVertTiles);}

	void updateInput();
};

#endif // BUTTON_H_INCLUDED
