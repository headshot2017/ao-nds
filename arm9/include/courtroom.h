#ifndef COURTROOM_H_INCLUDED
#define COURTROOM_H_INCLUDED

#include <nds/ndstypes.h>
#include <string>
#include <unordered_map>

#include "cfgFile.h"
#include "chatbox.h"

struct BgData
{
	u8* data;
	u32 len;
};

struct LoadedBg
{
	BgData mainBg;
	BgData mainMap;
	BgData mainPal;
	BgData deskBg;
	BgData deskPal;
};

class Courtroom
{
	std::unordered_map<std::string, LoadedBg> currentBg;
	std::string currentSide;
	cfgFile deskTiles;
	int bgIndex;
	bool visible;

	u16* deskGfx[4*6];
	bool deskGfxVisible[4*6];

	Chatbox* chatbox;

	void destroyBg();

public:
	Courtroom();
	~Courtroom();

	bool setBg(const std::string& name);
	void setBgSide(const std::string& side, bool force=false);
	void setVisible(bool on);

	void stopMusic() {playMusic("");}
	void playMusic(std::string filename);

	void update();
};

#endif // COURTROOM_H_INCLUDED
