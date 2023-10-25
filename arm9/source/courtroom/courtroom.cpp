#include "courtroom/courtroom.h"

#include <dirent.h>
#include <stdio.h>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/input.h>
#include <nds/arm9/video.h>

#include "mp3_shared.h"
#include "colors.h"

Courtroom::Courtroom()
{
	visible = false;

	tempColor = COLOR_WHITE;
	shakeForce = 0;
	shakeTicks = 0;

	background = new Background;
	chatbox = new Chatbox;
	character = new Character;

	chatbox->setOnChatboxFinishedCallback(onChatboxFinished, this);
	character->setOnAnimFinishedCallback(onAnimFinished, this);
}

Courtroom::~Courtroom()
{
	delete background;
	delete chatbox;
	delete character;
	mp3_stop();
}

void Courtroom::setVisible(bool on)
{
	visible = on;
	background->setVisible(on);
	chatbox->setVisible(on);
	character->setVisible(on);
}

void Courtroom::MSchat(std::string charname, std::string anim, std::string preAnim, int emoteMod, std::string name, std::string msg, int color, std::string blip)
{
	if (color < 0 || color >= 6)
		color = 0;

	tempChar = charname;
	tempAnim = anim;
	tempPreAnim = preAnim;
	tempName = name;
	tempMsg = msg;
	tempColor = AOcolorToPalette[color];
	tempBlip = blip;

	if (emoteMod == 0 || !fileExists("/data/ao-nds/characters/" + tempChar + "/" + tempPreAnim + ".img.bin"))
	{
		character->setCharImage(tempChar, ((tempMsg.empty() || tempColor == COLOR_BLUE) ? "(a)" : "(b)") + tempAnim);
		chatbox->setVisible(true);
		chatbox->setName(tempName);
		chatbox->setText(tempMsg, tempColor, tempBlip);
	}
	else
	{
		chatbox->setVisible(false);
		character->setCharImage(tempChar, tempPreAnim, false);
	}
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

	if (filename == "~stop.mp3")
		return;

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

void Courtroom::onChatboxFinished(void* pUserData)
{
	Courtroom* pSelf = (Courtroom*)pUserData;
	if (pSelf->tempColor != COLOR_BLUE)
		pSelf->character->setCharImage(pSelf->tempChar, "(a)"+pSelf->tempAnim);
}

void Courtroom::onAnimFinished(void* pUserData)
{
	Courtroom* pSelf = (Courtroom*)pUserData;
	pSelf->character->setCharImage(pSelf->tempChar, ((pSelf->tempMsg.empty() || pSelf->tempColor == COLOR_BLUE) ? "(a)" : "(b)") + pSelf->tempAnim);
	pSelf->chatbox->setVisible(true);
	pSelf->chatbox->setName(pSelf->tempName);
	pSelf->chatbox->setText(pSelf->tempMsg, pSelf->tempColor, pSelf->tempBlip);
}
