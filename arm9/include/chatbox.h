#ifndef CHATBOX_H_INCLUDED
#define CHATBOX_H_INCLUDED

#include <string>

#include <nds/ndstypes.h>

#include "mp3_shared.h"

enum
{
	COLOR_WHITE=1,
	COLOR_GREEN=3,
	COLOR_RED=5,
	COLOR_ORANGE=7,
	COLOR_BLUE=9,
	COLOR_YELLOW=11,
	COLOR_BLACK=13,
	COLOR_MAX
};

class Chatbox
{
	int bgIndex;
	u8* bgData;
	u8* bgMap;
	u8* bgPal;

	u8* textCanvas;
	u16* nameGfx[2];
	u16* textGfx[8*3]; // 8 rows of sprite text, 3 lines

	u32* blipSnd;
	u32 blipSize;

	u32 currTextInd;
	int currTextGfxInd;
	std::string currText;
	int textX;
	int textTicks;
	int textSpeed;
	int textColor;

public:
	Chatbox();
	~Chatbox();

	void setVisible(bool on);
	void setName(const char* name);
	void setText(const char* text, int color);

	void update();
};

#endif // CHATBOX_H_INCLUDED
