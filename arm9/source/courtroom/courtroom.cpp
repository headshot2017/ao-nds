#include "courtroom/courtroom.h"

#include <dirent.h>
#include <stdio.h>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/input.h>
#include <nds/arm9/video.h>

#include "mp3_shared.h"

Courtroom::Courtroom()
{
	visible = false;

	shakeForce = 0;
	shakeTicks = 0;

	background = new Background;
	chatbox = new Chatbox;
	character = new Character;
}

Courtroom::~Courtroom()
{
	delete background;
	delete chatbox;
	delete character;
}

void Courtroom::setVisible(bool on)
{
	visible = on;
	background->setVisible(on);
	chatbox->setVisible(on);
}

void Courtroom::playMusic(std::string filename)
{
	mp3_stop();

	// replace file extension
	auto pos = filename.find_last_of('.');
	if (pos != std::string::npos)
		filename = filename.substr(0, pos) + ".mp3";
	else
		filename += ".mp3";

	iprintf("%s\n", filename.c_str());
	FILE* f = fopen(filename.c_str(), "rb");
	if (f!=NULL)
		mp3_play_file(f, 1, 0);
}

void Courtroom::shake(int force, int ticks)
{
	shakeForce = force;
	shakeTicks = ticks;
}

void Courtroom::update()
{
	int xOffset = (shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0;
	int yOffset = (shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0;
	if (shakeTicks > 0) shakeTicks--;

	// chatbox gets its' own offsets
	character->setOffsets(xOffset, yOffset);
	background->setOffsets(xOffset, yOffset);
	chatbox->setOffsets(
		(shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0,
		(shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0
	);

	character->update();
	chatbox->update();
	background->update();
}
