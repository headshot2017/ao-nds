#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include <nds/ndstypes.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/input.h>

#include "global.h"

class UIButton
{
	u16** spriteGfx;
	int spriteGfxCount;
	SpriteSize spriteSize;
	OamState* oam;
	int oamStart;
	touchPosition touchPos;

	int x;
	int y;
	int w;
	int h;

	voidCallback callback;
	void* pUserData;

public:
	UIButton(OamState* chosenOam, u8* data, u8* palData, int oamStartInd, int gfxCount, SpriteSize sprSize, int xPos, int yPos, int width, int height, int palSlot);
	~UIButton();

	void setPos(int xPos, int yPos);
	void connect(voidCallback cb, void* p) {callback = cb; pUserData = p;}
	int nextOamInd() {return oamStart+spriteGfxCount;}

	void updateInput();
};

#endif // BUTTON_H_INCLUDED
