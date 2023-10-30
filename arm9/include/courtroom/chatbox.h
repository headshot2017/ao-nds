#ifndef CHATBOX_H_INCLUDED
#define CHATBOX_H_INCLUDED

// chatbox uses the following:
// BG slot 1, 512x256
// OAM slots 24-25 for name, sprite size 32x16
// OAM slots 26-49 for text, sprite size 32x16

#include <string>

#include <nds/ndstypes.h>

#include "global.h"

class Courtroom;

class Chatbox
{
	int bgIndex;
	u32 mapLen;
	u8* bgMap;
	u8* bgPal;

	Courtroom* m_pCourt;

	u8* textCanvas;
	u16* nameGfx[2];
	u16* textGfx[8*3]; // 8 rows of sprite text, 3 lines

	// arrow info: X between 243-246, Y=174. reverse arrowXadd when reaching a corner
	u16* spr_arrowGfx;
	int arrowX;
	int arrowXadd;
	int arrowTicks;

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

	int xOffset;
	int yOffset;

	void* pUserData;
	voidCallback onChatboxFinished;

	bool visible;
	bool ignoreBlend;

public:
	Chatbox(Courtroom* pCourt);
	~Chatbox();

	void setOffsets(int x, int y) {xOffset = x; yOffset = y;}
	void setIgnoreBlend(bool on) {ignoreBlend = on;}
	void setVisible(bool on);
	void setName(std::string name);
	void setText(std::string text, int color, std::string blip="male");
	void additiveText(std::string text, int color);

	bool isFinished() {return currTextInd >= currText.size();}
	void setOnChatboxFinishedCallback(voidCallback newCB, void* userdata) {onChatboxFinished = newCB; pUserData = userdata;}

	void update();
};

#endif // CHATBOX_H_INCLUDED
