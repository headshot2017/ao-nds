#ifndef COURTROOM_H_INCLUDED
#define COURTROOM_H_INCLUDED

#include <nds/ndstypes.h>
#include <string>

#include "courtroom/background.h"
#include "courtroom/chatbox.h"
#include "courtroom/character.h"

struct MSchatStruct
{
	bool deskMod;
	std::string preanim;
	std::string charname;
	std::string emote;
	std::string chatmsg;
	std::string sfx;
	int emoteMod;
	int charID;
	int sfxDelay;
	int shoutMod;
	int evidence;
	bool flip;
	int realization;
	int textColor;
	std::string showname;
	int otherCharID;
	std::string otherName;
	std::string otherEmote;
	std::string selfOffset;
	int otherOffset;
	int otherFlip;
	bool noInterrupt;
	int sfxLoop;
	int shake;
	bool additive;
	std::string blip;
};

class Courtroom
{
	bool visible;

	int shakeForce;
	int shakeTicks;
	int flashTicks;
	bool onPreAnim;

	u32* sndRealization;
	u32 sndRealizationSize;

	std::string tempChar;
	std::string tempAnim;
	std::string tempPreAnim;
	std::string tempName;
	std::string tempMsg;
	int tempColor;
	std::string tempBlip;
	int tempFlash;
	int tempID;
	int tempLastID;
	bool tempImmediate;
	bool tempAdditive;

	Background* background;
	Chatbox* chatbox;
	Character* character;

public:
	Courtroom();
	~Courtroom();

	Background* getBackground() {return background;}
	Chatbox* getChatbox() {return chatbox;}
	Character* getCharacter() {return character;}

	void setVisible(bool on);
	void MSchat(const MSchatStruct& data);

	void stopMusic() {playMusic("");}
	void playMusic(std::string filename);

	void shake(int force, int ticks);

	void update();

public:
	static void onChatboxFinished(void* pUserData);
	static void onAnimFinished(void* pUserData);
};

#endif // COURTROOM_H_INCLUDED
