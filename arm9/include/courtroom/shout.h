#ifndef SHOUT_H_INCLUDED
#define SHOUT_H_INCLUDED

// shout overlay (objection, hold it, take that, etc)
// BG uses slot 2, 512x256

#include <nds/ndstypes.h>
#include <string>

#include "global.h"
#include "mp3_shared.h"

class Courtroom;

class Shout
{
	int bgIndex;
	u32 bgMapLen;
	bool visible;

	Courtroom* m_pCourt;

	int xOffset;
	int yOffset;
	int ticks;

	wav_handle* sndShout;

	void* pUserData;
	voidCallback onShoutFinished;

public:
	Shout(Courtroom* pCourt);
	~Shout();


	void setOffsets(int x, int y) {xOffset = x; yOffset = y;}
	void setShout(const std::string& name, int shoutMod, const std::string& custom="");
	void setVisible(bool on);
	void freeSound();
	void cancelShout();

	void setOnShoutFinishedCallback(voidCallback newCB, void* userdata) {onShoutFinished = newCB; pUserData = userdata;}

	void update();
};

#endif // SHOUT_H_INCLUDED
