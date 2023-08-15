#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED

// character sprites use:
// OAM slots 50 and onward
// sprite size 32x32
// extended palette slot 2

#include <string>
#include <vector>

#include <nds/ndstypes.h>

#include "cfgFile.h"

class Character
{
	u16* charGfx[8*6];
	bool charGfxVisible[8*6];
	int gfxInUse;

	u8* charData;
	u32 charSize;

	std::string currCharacter;
	cfgFile animInfos;
	std::vector<u32> frameDurations;
	int frameW;
	int frameH;

	u32 timerTicks;
	int currFrame;

	bool visible;

public:
	Character();
	~Character();

	void setCharImage(std::string charname, std::string relativeFile);
	void setVisible(bool on);

	void update();
};

#endif // CHARACTER_H_INCLUDED
