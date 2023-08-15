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

	void stopMusic() {playMusic("");}
	void playMusic(std::string filename);

	void shake(int force, int ticks);

	void update();
};

#endif // COURTROOM_H_INCLUDED
