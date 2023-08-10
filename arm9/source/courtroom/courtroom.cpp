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

	background = new Background;
	background->setVisible(false);

	chatbox = new Chatbox;
	chatbox->setVisible(false);
}

Courtroom::~Courtroom()
{
	delete background;
	delete chatbox;
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
	if (!f)
		return;

	mp3_play_file(f, 1, 0);
}

void Courtroom::shake(int force, int ticks)
{
	chatbox->shake(force, ticks);
	background->shake(force, ticks);
}

void Courtroom::update()
{
	chatbox->update();
	background->update();

	bgUpdate();
	oamUpdate(&oamMain);
}
