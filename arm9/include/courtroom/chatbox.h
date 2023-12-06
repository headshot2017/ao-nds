#ifndef CHATBOX_H_INCLUDED
#define CHATBOX_H_INCLUDED

// chatbox uses the following:
// BG slot 1, 512x256
// OAM slots 24-25 for name, sprite size 32x16
// OAM slots 26-49 for text, sprite size 32x16

#include <string>
#include <stack>
#include <vector>

#include <nds/ndstypes.h>

#include "global.h"
#include "colors.h"
#include "mp3_shared.h"

struct colorSwitchChar
{
	int color;
	char start;
	char stop;
	bool talk;
	bool show;
	bool removing;
};

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

	wav_handle* blipSnd;

	int nameWidth;
	u32 currTextInd;
	int currTextGfxInd;
	int currTextLine;
	std::string currText;
	int textX;
	int textTimer;
	int textSpeed;
	int blipTicks;
	bool center;
	std::stack<colorSwitchChar> colorStack;
	std::vector<std::string> lines;
	std::vector<int> linesHalfWidth;

	int xOffset;
	int yOffset;

	void* pUserData;
	voidCallback onChatboxFinished;

	bool visible;
	bool ignoreBlend;

	bool handleEscape();
	bool handleControlChars();
	void handleNewLine();

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
	int getColor() {return (colorStack.empty()) ? COLOR_WHITE : colorStack.top().color;}
	void setOnChatboxFinishedCallback(voidCallback newCB, void* userdata) {onChatboxFinished = newCB; pUserData = userdata;}

	void update();
};

#endif // CHATBOX_H_INCLUDED
