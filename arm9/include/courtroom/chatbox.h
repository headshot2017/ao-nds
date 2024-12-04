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

struct ChatboxInfo
{
	int height;
	int nameX;
	int nameY;
	int bodyY;
	int lineSep;
	int arrowY;
};

class Courtroom;

class Chatbox
{
	int bgIndex;
	u32 mapLen;
	u8* bgMap;
	u8* bgPal;
	std::string currTheme;

	Courtroom* m_pCourt;

	ChatboxInfo info;
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
	std::u16string currText;
	int textX;
	int textTimer;
	int textSpeed;
	int blipTicks;
	bool center;
	std::stack<colorSwitchChar> colorStack;
	std::vector<std::u16string> lines;
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
	void setTheme(const std::string& name);
	void setName(std::u16string name);
	void setText(std::u16string text, int color, std::string blip="male");
	void additiveText(std::u16string text, int color);

	bool isFinished() {return currTextInd >= currText.size();}
	int getColor() {return (colorStack.empty()) ? COLOR_WHITE : colorStack.top().color;}
	void setOnChatboxFinishedCallback(voidCallback newCB, void* userdata) {onChatboxFinished = newCB; pUserData = userdata;}

	void update();

	void updateBodyPosition();
};

#endif // CHATBOX_H_INCLUDED
