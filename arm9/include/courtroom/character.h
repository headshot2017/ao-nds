#ifndef CHARACTER_H_INCLUDED
#define CHARACTER_H_INCLUDED

// character sprites use:
// OAM slots 50 and onward
// sprite size 32x32
// extended palette slot 2

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <nds/ndstypes.h>

#include "cfgFile.h"
#include "global.h"
#include "animStream.h"
#include "mp3_shared.h"

struct FrameInfo
{
	FrameInfo() : frameCount(0), realW(0), realH(0), frameW(0), frameH(0), offsetX(0), offsetY(0) {}

	std::vector<u32> frameDurations;
	std::vector<u16> frameIndexes;
	u32 frameCount;
	int realW;
	int realH;
	int frameW;
	int frameH;
	int offsetX;
	int offsetY;
	bool streaming;
};

class Courtroom;

class Character
{
	Courtroom* m_pCourt;

	int oamStart;
	u16* charGfx[4*3];
	bool charGfxVisible[4*3];
	int gfxInUse;

	int pair;
	u8* charData;
	animStream stream;

	std::string currCharacter;
	std::string currAnim;

	cfgFile animInfos;
	FrameInfo frameInfo;
	bool loop;

	u32 charTicks;
	u32 currFrame;

	int shakeX;
	int shakeY;
	int offsetX;
	int offsetY;
	bool flip;

	bool sfxPlayed;
	u32 sfxTicks;
	u32 sfxDelay;
	wav_handle* sfx;

	std::unordered_map<std::string, wav_handle*> cachedFrameSFX;
	std::unordered_map<int, std::string> frameSFX;
	std::unordered_set<int> frameFlash;
	std::unordered_set<int> frameShake;

	void* pUserData;
	voidCallback onAnimFinished;

	bool visible;

	void clearFrameData();

public:
	Character(Courtroom* pCourt, int start, int isPair);
	~Character();

	const std::string& getCurrCharacter() {return currCharacter;}
	const std::string& getCurrAnim() {return currAnim;}

	void setShakes(int x, int y) {shakeX = x; shakeY = y;}
	void setOffsets(int x, int y) {offsetX = x; offsetY = y;}
	void setFlip(bool on) {flip = on;}
	void setCharImage(std::string charname, std::string relativeFile, bool doLoop=true);
	void setSound(const std::string& filename, int delay);
	void setFrameSFX(const std::string& data);
	void setFrameFlash(const std::string& data);
	void setFrameShake(const std::string& data);
	void setVisible(bool on);
	void unload();

	void setOnAnimFinishedCallback(voidCallback newCB, void* userdata) {onAnimFinished = newCB; pUserData = userdata;}

	void triggerFrameData();
	void update();
};

#endif // CHARACTER_H_INCLUDED
