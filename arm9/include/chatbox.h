#ifndef CHATBOX_H_INCLUDED
#define CHATBOX_H_INCLUDED

#include <nds/ndstypes.h>

class Chatbox
{
	int bgIndex;
	u8* bgData;
	u8* bgMap;
	u8* bgPal;

public:
	Chatbox();
	~Chatbox();

	void setVisible(bool on);
};

#endif // CHATBOX_H_INCLUDED
