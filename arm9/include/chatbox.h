#ifndef CHATBOX_H_INCLUDED
#define CHATBOX_H_INCLUDED

#include <nds/ndstypes.h>

enum
{
	COLOR_WHITE=1,
	COLOR_GREEN,
	COLOR_RED,
	COLOR_ORANGE,
	COLOR_BLUE,
	COLOR_YELLOW,
	COLOR_BLACK,
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

public:
	Chatbox();
	~Chatbox();

	void setVisible(bool on);
	void setName(const char* name);

	void update();
};

#endif // CHATBOX_H_INCLUDED
