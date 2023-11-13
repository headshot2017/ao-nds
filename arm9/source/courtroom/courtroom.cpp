#include "courtroom/courtroom.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/input.h>
#include <nds/arm9/video.h>

#include "colors.h"

Courtroom::Courtroom()
{
	visible = false;
	onPreAnim = false;

	shakeForce = 0;
	shakeTicks = 0;
	flashTicks = 0;

	sndRealization = wav_load_handle("/data/ao-nds/sounds/general/sfx-realization.wav");

	background = new Background;
	chatbox = new Chatbox(this);
	character = new Character(this);
	shout = new Shout(this);
	evidence = new Evidence;

	chatbox->setOnChatboxFinishedCallback(onChatboxFinished, this);
	shout->setOnShoutFinishedCallback(onShoutFinished, this);
}

Courtroom::~Courtroom()
{
	delete background;
	delete chatbox;
	delete character;
	delete shout;
	delete evidence;

	wav_free_handle(sndRealization);

	mp3_stop();
}

void Courtroom::setVisible(bool on)
{
	visible = on;
	background->setVisible(on);
	chatbox->setVisible(on);
	character->setVisible(on);
	shout->setVisible(on);
	evidence->setVisible(on);
}

void Courtroom::setTalkingAnim(bool on)
{
	if (onPreAnim) return;

	std::string prefix = (on) ? "(b)" : "(a)";
	character->setCharImage(currIC.charname, prefix + currIC.emote);
}

void Courtroom::MSchat(const MSchatStruct& data)
{
	currIC = data;

	if (currIC.textColor < 0 || currIC.textColor >= 6)
		currIC.textColor = 0;

	AOdecode(currIC.charname);
	AOdecode(currIC.showname);
	AOdecode(currIC.selfOffset);
	AOdecode(currIC.otherOffset);
	if (currIC.blip.empty()) currIC.blip = "male";

	handleChat();
}

void Courtroom::handleChat()
{
	if (currIC.shoutMod)
	{
		character->setOnAnimFinishedCallback(0, 0);
		shout->setShout(currIC.charname, currIC.shoutMod, currIC.customShout);
		return;
	}
	else
		shout->cancelShout();

	if (currIC.evidence.empty())
		evidence->hideEvidence();
	else
		evidence->showEvidence(currIC.evidence, currIC.side == "def" || currIC.side == "hlp");

	std::string chatname = (currIC.showname.empty()) ? currIC.charname : currIC.showname;
	int color = AOcolorToPalette[currIC.textColor];

	if (currIC.shake)
		shake(5, 35);

	std::string offsetStr = currIC.selfOffset;

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
	character->setFlip(currIC.flip);
	character->setOnAnimFinishedCallback(onAnimFinished, this);
	background->setBgSide(currIC.side);

	if (currIC.emoteMod == 1 || currIC.emoteMod >= 5)
		character->setSound("/data/ao-nds/sounds/general/" + currIC.sfx + ".wav", currIC.sfxDelay);

	if (currIC.emoteMod == 0 || !fileExists("/data/ao-nds/characters/" + currIC.charname + "/" + currIC.preanim + ".img.bin"))
	{
		onPreAnim = false;

		bool isPng = (currIC.preanim != currIC.emote && fileExists("/data/ao-nds/characters/" + currIC.charname + "/" + currIC.emote + ".img.bin"));
		bool playIdle = (currIC.chatmsg.empty() || color == COLOR_BLUE);
		std::string prefix;
		if (!isPng) prefix = (playIdle ? "(a)" : "(b)");

		character->setCharImage(currIC.charname, prefix + currIC.emote);
		if (!isPng)
		{
			int index = (playIdle) ? 2 : 1;
			if (!currIC.frameSFX.empty()) character->setFrameSFX(argumentAt(currIC.frameSFX, index, '^'));
			if (!currIC.frameShake.empty()) character->setFrameShake(argumentAt(currIC.frameShake, index, '^'));
			if (!currIC.frameFlash.empty()) character->setFrameFlash(argumentAt(currIC.frameFlash, index, '^'));
		}
		chatbox->setVisible(true);
		chatbox->setName(chatname);

		if (currIC.additive && currIC.charID == lastIC.charID)
			chatbox->additiveText(currIC.chatmsg, color);
		else
			chatbox->setText(currIC.chatmsg, color, currIC.blip);
		lastIC = currIC;

		if (currIC.realization)
		{
			flash(3);
			wav_play(sndRealization);
		}
	}
	else
	{
		// play pre-animation
		onPreAnim = true;

		if (!currIC.noInterrupt)
			chatbox->setVisible(false);
		else
		{
			chatbox->setVisible(true);
			chatbox->setName(chatname);

			if (currIC.additive && currIC.charID == lastIC.charID)
				chatbox->additiveText(currIC.chatmsg, color);
			else
				chatbox->setText(currIC.chatmsg, color, currIC.blip);
			lastIC = currIC;
		}
		character->setCharImage(currIC.charname, currIC.preanim, false);
		if (!currIC.frameSFX.empty()) character->setFrameSFX(argumentAt(currIC.frameSFX, 0, '^'));
		if (!currIC.frameShake.empty()) character->setFrameShake(argumentAt(currIC.frameShake, 0, '^'));
		if (!currIC.frameFlash.empty()) character->setFrameFlash(argumentAt(currIC.frameFlash, 0, '^'));
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
	mp3_play(filename.c_str(), 1, 0);
}

void Courtroom::shake(int force, int ticks)
{
	shakeForce = force;
	shakeTicks = ticks;
}

void Courtroom::flash(int ticks)
{
	flashTicks = ticks;
	chatbox->setIgnoreBlend(true);
}

void Courtroom::update()
{
	int xOffset = (shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0;
	int yOffset = (shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0;
	if (shakeTicks > 0) shakeTicks--;

	// chatbox and shouts get their own offsets
	character->setShakes(xOffset, yOffset);
	background->setOffsets(xOffset, yOffset);
	evidence->setOffsets(xOffset, yOffset);
	chatbox->setOffsets(
		(shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0,
		(shakeTicks > 0) ? -shakeForce + rand()%(shakeForce*2) : 0
	);

	character->update();
	chatbox->update();
	background->update();
	shout->update();
	evidence->update();

	if (flashTicks)
	{
		flashTicks--;
		if (!flashTicks)
		{
			REG_BLDCNT = BLEND_NONE;
			REG_BLDY = 0;
			chatbox->setIgnoreBlend(false);
		}
		else
		{
			REG_BLDCNT = BLEND_FADE_WHITE | BLEND_SRC_BACKDROP | BLEND_SRC_BG0 | BLEND_SRC_BG1 | BLEND_SRC_BG2 | BLEND_SRC_BG3 | BLEND_SRC_SPRITE;
			REG_BLDY = 16;
		}
	}
}

void Courtroom::onChatboxFinished(void* pUserData)
{
	Courtroom* pSelf = (Courtroom*)pUserData;

	int color = AOcolorToPalette[pSelf->currIC.textColor];
	if (color != COLOR_BLUE && (!pSelf->currIC.noInterrupt || !pSelf->onPreAnim))
	{
		bool isPng = (pSelf->currIC.preanim != pSelf->currIC.emote && fileExists("/data/ao-nds/characters/" + pSelf->currIC.charname + "/" + pSelf->currIC.emote + ".img.bin"));
		std::string prefix;
		if (!isPng) prefix = "(a)";

		pSelf->character->setCharImage(pSelf->currIC.charname, prefix + pSelf->currIC.emote);
		if (!isPng)
		{
			if (!pSelf->currIC.frameSFX.empty()) pSelf->character->setFrameSFX(argumentAt(pSelf->currIC.frameSFX, 2, '^'));
			if (!pSelf->currIC.frameShake.empty()) pSelf->character->setFrameShake(argumentAt(pSelf->currIC.frameShake, 2, '^'));
			if (!pSelf->currIC.frameFlash.empty()) pSelf->character->setFrameFlash(argumentAt(pSelf->currIC.frameFlash, 2, '^'));
		}
	}
}

void Courtroom::onAnimFinished(void* pUserData)
{
	Courtroom* pSelf = (Courtroom*)pUserData;

	int color = AOcolorToPalette[pSelf->currIC.textColor];
	bool useIdleAnim =
		pSelf->currIC.chatmsg.empty() ||
		color == COLOR_BLUE || pSelf->chatbox->getColor() == COLOR_BLUE ||
		(pSelf->currIC.noInterrupt && pSelf->chatbox->isFinished());

	pSelf->onPreAnim = false;

	bool isPng = (pSelf->currIC.preanim != pSelf->currIC.emote && fileExists("/data/ao-nds/characters/" + pSelf->currIC.charname + "/" + pSelf->currIC.emote + ".img.bin"));
	std::string prefix;
	if (!isPng) prefix = ((useIdleAnim) ? "(a)" : "(b)");

	pSelf->character->setCharImage(pSelf->currIC.charname, prefix + pSelf->currIC.emote);
	if (!isPng)
	{
		int index = (useIdleAnim) ? 2 : 1;
		if (!pSelf->currIC.frameSFX.empty()) pSelf->character->setFrameSFX(argumentAt(pSelf->currIC.frameSFX, index, '^'));
		if (!pSelf->currIC.frameShake.empty()) pSelf->character->setFrameShake(argumentAt(pSelf->currIC.frameShake, index, '^'));
		if (!pSelf->currIC.frameFlash.empty()) pSelf->character->setFrameFlash(argumentAt(pSelf->currIC.frameFlash, index, '^'));
	}

	if (useIdleAnim) pSelf->character->setOnAnimFinishedCallback(0, 0);

	if (!pSelf->currIC.noInterrupt)
	{
		std::string chatname = (pSelf->currIC.showname.empty()) ? pSelf->currIC.charname : pSelf->currIC.showname;
		pSelf->chatbox->setVisible(true);
		pSelf->chatbox->setName(chatname);

		if (pSelf->currIC.additive && pSelf->currIC.charID == pSelf->lastIC.charID)
			pSelf->chatbox->additiveText(pSelf->currIC.chatmsg, color);
		else
			pSelf->chatbox->setText(pSelf->currIC.chatmsg, color, pSelf->currIC.blip);
		pSelf->lastIC = pSelf->currIC;
	}

	if (pSelf->currIC.realization)
	{
		pSelf->flash(3);
		wav_play(pSelf->sndRealization);
	}
}

void Courtroom::onShoutFinished(void* pUserData)
{
	Courtroom *pSelf = (Courtroom*)pUserData;

	pSelf->currIC.shoutMod = 0;
	if (pSelf->currIC.emoteMod == 2 || pSelf->currIC.emoteMod == 6)
		pSelf->currIC.emoteMod--;
	pSelf->handleChat();
}
