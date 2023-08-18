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
#include "global.h"

class Character
{
	u16* charGfx[8*6];
	bool charGfxVisible[8*6];
	int gfxInUse;

	u8* charData;

	std::string currCharacter;
	std::string currAnim;

	cfgFile animInfos;
	std::vector<u32> frameDurations;
	int realW;
	int frameW;
	int frameH;
	bool loop;

	u32 timerTicks;
	u32 currFrame;

	int xOffset;
	int yOffset;

	void* pUserData;
	voidCallback onAnimFinished;

	bool visible;

public:
	Character();
	~Character();

	const std::string& getCurrCharacter() {return currCharacter;}
	const std::string& getCurrAnim() {return currAnim;}

	void setOffsets(int x, int y) {xOffset = x; yOffset = y;}
	void setCharImage(std::string charname, std::string relativeFile, bool doLoop=true);
	void setVisible(bool on);

	void setOnAnimFinishedCallback(voidCallback newCB, void* userdata) {onAnimFinished = newCB; pUserData = userdata;}

	void update();
};

#endif // CHARACTER_H_INCLUDED
