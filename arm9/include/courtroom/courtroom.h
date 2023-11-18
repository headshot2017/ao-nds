#ifndef COURTROOM_H_INCLUDED
#define COURTROOM_H_INCLUDED

#include <nds/ndstypes.h>
#include <string>

#include "courtroom/background.h"
#include "courtroom/chatbox.h"
#include "courtroom/character.h"
#include "courtroom/shout.h"
#include "courtroom/evidence.h"
#include "mp3_shared.h"

struct MSchatStruct
{
	MSchatStruct() :
		deskMod(false),
		emoteMod(0),
		charID(-1),
		sfxDelay(0),
		shoutMod(0),
		evidence(),
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
	std::string evidence;
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
	std::string frameShake;
	std::string frameFlash;
	std::string frameSFX;
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

	wav_handle* sndRealization;
	wav_handle* sndEvShow;

	MSchatStruct lastIC;
	MSchatStruct currIC;

	Background* background;
	Chatbox* chatbox;
	Character* character[2];
	Shout* shout;
	Evidence* evidence;

	void handleChat();

public:
	Courtroom();
	~Courtroom();

	Background* getBackground() {return background;}
	Chatbox* getChatbox() {return chatbox;}
	Character* getCharacter(int pair) {return character[pair];}
	Shout* getShout() {return shout;}
	Evidence* getEvidence() {return evidence;}

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
