#ifndef COURTROOM_H_INCLUDED
#define COURTROOM_H_INCLUDED

#include <nds/ndstypes.h>
#include <string>

#include "courtroom/background.h"
#include "courtroom/chatbox.h"
#include "courtroom/character.h"

class Courtroom
{
	bool visible;

	int shakeForce;
	int shakeTicks;

	std::string tempChar;
	std::string tempAnim;
	std::string tempPreAnim;
	std::string tempName;
	std::string tempMsg;
	int tempColor;
	std::string tempBlip;

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
	void MSchat(std::string charname, std::string anim, std::string preAnim, int emoteMod, std::string name, std::string msg, int color, std::string blip);

	void stopMusic() {playMusic("");}
	void playMusic(std::string filename);

	void shake(int force, int ticks);

	void update();

public:
	static void onChatboxFinished(void* pUserData);
	static void onAnimFinished(void* pUserData);
};

#endif // COURTROOM_H_INCLUDED
