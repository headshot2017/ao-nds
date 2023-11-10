#ifndef COURTROOM_H_INCLUDED
#define COURTROOM_H_INCLUDED

#include <nds/ndstypes.h>
#include <string>

#include "courtroom/background.h"
#include "courtroom/chatbox.h"
#include "courtroom/character.h"
#include "courtroom/shout.h"

struct MSchatStruct
{
	MSchatStruct() :
		deskMod(false),
		emoteMod(0),
		charID(-1),
		sfxDelay(0),
		shoutMod(0),
		evidence(0),
		flip(false),
		realization(0),
		textColor(0),
		otherCharID(-1),
		otherFlip(0),
		noInterrupt(false),
		sfxLoop(0),
		shake(0),
		additive(false) {}

	bool deskMod;
	std::string preanim;
	std::string charname;
	std::string emote;
	std::string chatmsg;
	std::string side;
	std::string sfx;
	int emoteMod;
	int charID;
	int sfxDelay;
	int shoutMod;
	std::string customShout;
	int evidence;
	bool flip;
	int realization;
	int textColor;
	std::string showname;
	int otherCharID;
	std::string otherName;
	std::string otherEmote;
	std::string selfOffset;
	std::string otherOffset;
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

	MSchatStruct lastIC;
	MSchatStruct currIC;

	Background* background;
	Chatbox* chatbox;
	Character* character;
	Shout* shout;

	void handleChat();

public:
	Courtroom();
	~Courtroom();

	Background* getBackground() {return background;}
	Chatbox* getChatbox() {return chatbox;}
	Character* getCharacter() {return character;}
	Shout* getShout() {return shout;}

	void setVisible(bool on);
	void setTalkingAnim(bool on);
	void MSchat(const MSchatStruct& data);

	void stopMusic() {playMusic("");}
	void playMusic(std::string filename);

	void shake(int force, int ticks);
	void flash(int ticks);

	void update();

public:
	static void onChatboxFinished(void* pUserData);
	static void onAnimFinished(void* pUserData);
	static void onShoutFinished(void* pUserData);
};

#endif // COURTROOM_H_INCLUDED
