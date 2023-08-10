#ifndef COURTROOM_H_INCLUDED
#define COURTROOM_H_INCLUDED

#include <nds/ndstypes.h>
#include <string>

#include "courtroom/background.h"
#include "courtroom/chatbox.h"

class Courtroom
{
	bool visible;

	Background* background;
	Chatbox* chatbox;

public:
	Courtroom();
	~Courtroom();

	Chatbox* getChatbox() {return chatbox;}
	Background* getBackground() {return background;}

	void setVisible(bool on);

	void stopMusic() {playMusic("");}
	void playMusic(std::string filename);

	void update();
};

#endif // COURTROOM_H_INCLUDED
