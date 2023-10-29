#ifndef SHOUT_H_INCLUDED
#define SHOUT_H_INCLUDED

// shout overlay (objection, hold it, take that, etc)
// BG uses slot 2, 512x256

#include <nds/ndstypes.h>
#include <string>

#include "cfgFile.h"

class Shout
{
	int bgIndex;
	bool visible;

	int xOffset;
	int yOffset;

	u32* sndShout;
	u32 sndShoutSize;

public:
	Shout();
	~Shout();

	void setOffsets(int x, int y) {xOffset = x; yOffset = y;}
	void setShout(const std::string& name);
	void setVisible(bool on);
	void freeSound();

	void update();
};

#endif // SHOUT_H_INCLUDED
