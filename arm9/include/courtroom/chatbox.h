#ifndef CHATBOX_H_INCLUDED
#define CHATBOX_H_INCLUDED

// chatbox uses the following:
// BG slot 1, 256x256
// OAM slots 24-25 for name, sprite size 32x16
// OAM slots 26-49 for text, sprite size 32x16

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

	int nameWidth;
	u32 currTextInd;
	int currTextGfxInd;
	std::string currText;
	int textX;
	int textTicks;
	int textSpeed;
	int textColor;
	int blipTicks;

	int shakeForce;
	int shakeTicks;

public:
	Chatbox();
	~Chatbox();

	void setVisible(bool on);
	void setName(const char* name);
	void setText(std::string text, int color, std::string blip="male");

	void shake(int force, int ticks);

	void update();
};

#endif // CHATBOX_H_INCLUDED
