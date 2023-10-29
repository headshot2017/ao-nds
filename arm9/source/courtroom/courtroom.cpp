#include "courtroom/courtroom.h"

#include <dirent.h>
#include <stdio.h>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/input.h>
#include <nds/arm9/video.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "colors.h"

Courtroom::Courtroom()
{
	visible = false;
	onPreAnim = false;

	tempColor = COLOR_WHITE;
	shakeForce = 0;
	shakeTicks = 0;
	flashTicks = 0;
	tempID = -1;
	tempLastID = -1;

	sndRealization = wav_load_handle("/data/ao-nds/sounds/general/sfx-realization.wav", &sndRealizationSize);

	background = new Background;
	chatbox = new Chatbox(this);
	character = new Character;
	shout = new Shout;

	chatbox->setOnChatboxFinishedCallback(onChatboxFinished, this);
}

Courtroom::~Courtroom()
{
	delete background;
	delete chatbox;
	delete character;
	delete shout;

	delete[] sndRealization;

	mp3_stop();
}

void Courtroom::setVisible(bool on)
{
	visible = on;
	background->setVisible(on);
	chatbox->setVisible(on);
	character->setVisible(on);
}

void Courtroom::MSchat(const MSchatStruct& data)
{
	int color = data.textColor;
	if (color < 0 || color >= 6)
		color = 0;

	tempChar = data.charname;
	tempAnim = data.emote;
	tempPreAnim = data.preanim;
	tempName = (data.showname.empty()) ? data.charname : data.showname;
	tempMsg = data.chatmsg;
	tempColor = AOcolorToPalette[color];
	tempBlip = data.blip;
	tempFlash = data.realization;
	tempImmediate = data.noInterrupt;
	tempID = data.charID;
	tempAdditive = data.additive;
	if (tempBlip.empty()) tempBlip = "male";

	AOdecode(tempName);
	AOdecode(tempMsg);

	if (data.shake)
		shake(5, 35);

	std::string offsetStr = data.selfOffset;
	AOdecode(offsetStr);

	int offsetX = 0;
	int offsetY = 0;

	if (offsetStr.find("&"))
	{
		offsetX = std::stoi(argumentAt(offsetStr, 0, '&'));
		offsetY = std::stoi(argumentAt(offsetStr, 1, '&'));
	}
	else
		offsetX = std::stoi(offsetStr);

	character->setOffsets(offsetX/100.f*256, offsetY/100.f*192);
	character->setFlip(data.flip);
	character->setOnAnimFinishedCallback(onAnimFinished, this);

	if (data.emoteMod == 0 || !fileExists("/data/ao-nds/characters/" + tempChar + "/" + tempPreAnim + ".img.bin"))
	{
		onPreAnim = false;

		character->setCharImage(tempChar, ((tempMsg.empty() || tempColor == COLOR_BLUE) ? "(a)" : "(b)") + tempAnim);
		chatbox->setVisible(true);
		chatbox->setName(tempName);

		if (tempAdditive && tempID == tempLastID)
			chatbox->additiveText(tempMsg, tempColor);
		else
			chatbox->setText(tempMsg, tempColor, tempBlip);
		tempLastID = tempID;

		if (tempFlash)
		{
			flash(5);
			soundPlaySample(sndRealization, SoundFormat_16Bit, sndRealizationSize, 32000, 127, 64, false, 0);
		}
	}
	else
	{
		// play pre-animation
		onPreAnim = true;

		if (!tempImmediate)
			chatbox->setVisible(false);
		else
		{
			chatbox->setVisible(true);
			chatbox->setName(tempName);

			if (tempAdditive && tempID == tempLastID)
				chatbox->additiveText(tempMsg, tempColor);
			else
				chatbox->setText(tempMsg, tempColor, tempBlip);
			tempLastID = tempID;
		}
		character->setCharImage(tempChar, tempPreAnim, false);
	}
}

void Courtroom::playMusic(std::string filename)
{
	mp3_stop();

	if (filename.find("~stop") != std::string::npos)
		return; // don't play ~stop.mp3

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

void Courtroom::flash(int ticks)
{
	flashTicks = ticks;
}

void Courtroom::update()
{
	int xOffset = (shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0;
	int yOffset = (shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0;
	if (shakeTicks > 0) shakeTicks--;

	// chatbox gets its' own offsets
	character->setShakes(xOffset, yOffset);
	background->setOffsets(xOffset, yOffset);
	chatbox->setOffsets(
		(shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0,
		(shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0
	);

	character->update();
	chatbox->update();
	background->update();
	shout->update();

	if (flashTicks)
	{
		flashTicks--;
		REG_BLDCNT = BLEND_FADE_WHITE | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
		REG_BLDY = 16;
	}
}

void Courtroom::onChatboxFinished(void* pUserData)
{
	Courtroom* pSelf = (Courtroom*)pUserData;
	if (pSelf->tempColor != COLOR_BLUE && (!pSelf->tempImmediate || !pSelf->onPreAnim))
		pSelf->character->setCharImage(pSelf->tempChar, "(a)"+pSelf->tempAnim);
}

void Courtroom::onAnimFinished(void* pUserData)
{
	Courtroom* pSelf = (Courtroom*)pUserData;

	bool useIdleAnim =
		pSelf->tempMsg.empty() ||
		pSelf->tempColor == COLOR_BLUE ||
		(pSelf->tempImmediate && pSelf->chatbox->isFinished());

	pSelf->onPreAnim = false;

	pSelf->character->setCharImage(pSelf->tempChar, ((useIdleAnim) ? "(a)" : "(b)") + pSelf->tempAnim);
	if (useIdleAnim) pSelf->character->setOnAnimFinishedCallback(0, 0);

	if (!pSelf->tempImmediate)
	{
		pSelf->chatbox->setVisible(true);
		pSelf->chatbox->setName(pSelf->tempName);

		if (pSelf->tempAdditive && pSelf->tempID == pSelf->tempLastID)
			pSelf->chatbox->additiveText(pSelf->tempMsg, pSelf->tempColor);
		else
			pSelf->chatbox->setText(pSelf->tempMsg, pSelf->tempColor, pSelf->tempBlip);
		pSelf->tempLastID = pSelf->tempID;
	}

	if (pSelf->tempFlash)
	{
		pSelf->flash(5);
		soundPlaySample(pSelf->sndRealization, SoundFormat_16Bit, pSelf->sndRealizationSize, 32000, 127, 64, false, 0);
	}
}
