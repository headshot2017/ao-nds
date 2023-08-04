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

public:
	Chatbox();
	~Chatbox();

	void setVisible(bool on);

	void setName(const char* name);
};

#endif // CHATBOX_H_INCLUDED
