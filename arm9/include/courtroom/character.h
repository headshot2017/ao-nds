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

struct FrameInfo
{
	FrameInfo() : frameCount(0), realW(0), frameW(0), frameH(0), offsetX(0), offsetY(0) {}

	std::vector<u32> frameDurations;
	std::vector<u16> frameIndexes;
	u32 frameCount;
	int realW;
	int frameW;
	int frameH;
	int offsetX;
	int offsetY;
};

class Character
{
	u16* charGfx[8*6];
	bool charGfxVisible[8*6];
	int gfxInUse;

	u8* charData;

	std::string currCharacter;
	std::string currAnim;

	cfgFile animInfos;
	FrameInfo frameInfo;
	bool loop;

	u32 timerTicks;
	u32 currFrame;

	int shakeX;
	int shakeY;

	void* pUserData;
	voidCallback onAnimFinished;

	bool visible;

public:
	Character();
	~Character();

	const std::string& getCurrCharacter() {return currCharacter;}
	const std::string& getCurrAnim() {return currAnim;}

	void setOffsets(int x, int y) {shakeX = x; shakeY = y;}
	void setCharImage(std::string charname, std::string relativeFile, bool doLoop=true);
	void setVisible(bool on);

	void setOnAnimFinishedCallback(voidCallback newCB, void* userdata) {onAnimFinished = newCB; pUserData = userdata;}

	void update();
};

#endif // CHARACTER_H_INCLUDED
