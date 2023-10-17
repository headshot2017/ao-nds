#include "ui/uicourt.h"

#include <string.h>
#include <algorithm>

#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <nds/arm9/input.h>
#include <nds/interrupts.h>

#include "mp3_shared.h"
#include "engine.h"
#include "fonts.h"
#include "global.h"
#include "mini/ini.h"
#include "ui/uiserverlist.h"
#include "ui/uidisconnected.h"
#include "ui/court/loading.h"
#include "ui/court/console.h"

const char* indToSide[6] = {
	"def",
	"pro",
	"wit",
	"hld",
	"hlp",
	"jud"
};

UIScreenCourt::UIScreenCourt() : UIScreen()
{
	court = 0;
	subScreen = 0;
	nextScreen = 0;
}

UIScreenCourt::~UIScreenCourt()
{
	if (court)
	{
		court->setVisible(false);
		delete court;
	}

	if (subScreen)
		delete subScreen;

	delete[] sndSelect;
	delete[] sndCancel;
	delete[] sndEvTap;
	delete[] sndEvPage;

	AOsocket* sock = gEngine->getSocket();
	sock->clearCallbacks();
	gEngine->setSocket(nullptr);

	bgExtPaletteDisable();
}

void UIScreenCourt::init()
{
	bgExtPaletteEnable();
	currChar = -1;
	memset(bars, 0, sizeof(bars));

	showname = gEngine->getShowname();
	oocName = gEngine->getOOCname();

	if (keysHeld() & KEY_SELECT)
		subScreen = new UICourtConsole(this);
	else
		subScreen = new UICourtLoading(this);
	subScreen->init();

	court = new Courtroom;
	court->setVisible(true);

	sndSelect = wav_load_handle("/data/ao-nds/sounds/general/sfx-selectblip2.wav", &sndSelectSize);
	sndCancel = wav_load_handle("/data/ao-nds/sounds/general/sfx-cancel.wav", &sndCancelSize);
	sndEvTap = wav_load_handle("/data/ao-nds/sounds/general/sfx-selectblip.wav", &sndEvTapSize);
	sndEvPage = wav_load_handle("/data/ao-nds/sounds/general/sfx-blink.wav", &sndEvPageSize);

	AOsocket* sock = gEngine->getSocket();
	sock->addMessageCallback("ID", onMessageID, this);
	sock->addMessageCallback("PN", onMessagePN, this);
	sock->addMessageCallback("SI", onMessageSI, this);
	sock->addMessageCallback("SC", onMessageSC, this);
	sock->addMessageCallback("SM", onMessageSM, this);
	sock->addMessageCallback("BN", onMessageBN, this);
	sock->addMessageCallback("MC", onMessageMC, this);
	sock->addMessageCallback("MS", onMessageMS, this);
	sock->addMessageCallback("CT", onMessageCT, this);
	sock->addMessageCallback("CharsCheck", onMessageCharsCheck, this);
	sock->addMessageCallback("PV", onMessagePV, this);
	sock->addMessageCallback("FA", onMessageFA, this);
	sock->addMessageCallback("ARUP", onMessageARUP, this);
	sock->addMessageCallback("HP", onMessageHP, this);
	sock->addMessageCallback("KK", onMessageKK, this);
	sock->addMessageCallback("KB", onMessageKB, this);
	sock->addMessageCallback("BD", onMessageBD, this);

	std::string hdid = "HI#NDS " + gEngine->getMacAddr() + "#%";
	gEngine->getSocket()->sendData(hdid);
}

void UIScreenCourt::updateInput()
{
	subScreen->updateInput();
}

void UIScreenCourt::update()
{
	if (nextScreen)
	{
		if (subScreen) delete subScreen;
		subScreen = nextScreen;
		subScreen->init();
		nextScreen = nullptr;
	}

	court->update();
	subScreen->update();

	if (!icSendQueue.empty())
		gEngine->getSocket()->sendData(icSendQueue.front());
}

void UIScreenCourt::changeScreen(UISubScreen* newScreen)
{
	nextScreen = newScreen;
}

void UIScreenCourt::sendIC(const std::string& msg)
{
	icSendQueue.push_back(msg);
}

void UIScreenCourt::onMessageID(void* pUserData, std::string msg)
{
	gEngine->getSocket()->sendData("ID#AO-NDS#0.0.1#%");
}

void UIScreenCourt::onMessagePN(void* pUserData, std::string msg)
{
	gEngine->getSocket()->sendData("askchaa#%");
}

void UIScreenCourt::onMessageSI(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	pSelf->charList.clear();
	pSelf->musicList.clear();

	gEngine->getSocket()->sendData("RC#%");
}

void UIScreenCourt::onMessageSC(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	std::size_t delimiterPos = msg.find("#");
	std::size_t lastPos = delimiterPos;
	delimiterPos = msg.find("#", delimiterPos+1);

	while (lastPos != std::string::npos)
	{
		std::string name = msg.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1);
		std::string showname = name;
		std::string blip = "male";
		std::string side = "wit";

		mINI::INIFile file("/data/ao-nds/characters/" + name + "/char.ini");
		mINI::INIStructure ini;

		if (file.read(ini))
		{
			showname = ini["options"]["showname"];
			if (showname.empty()) showname = name;

			blip = ini["options"]["blips"];
			if (blip.empty()) blip = ini["options"]["gender"];

			side = ini["options"]["side"];
		}

		pSelf->charList.push_back({name, showname, blip, side, false, false});

		lastPos = delimiterPos;
		delimiterPos = msg.find("#", delimiterPos+1);
	}

	pSelf->charList.pop_back(); // remove "%"

	gEngine->getSocket()->sendData("RM#%");
}

void UIScreenCourt::onMessageSM(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	bool musics_time = false;
	std::size_t delimiterPos = msg.find("#");
	std::size_t lastPos = delimiterPos;
	delimiterPos = msg.find("#", delimiterPos+1);

	while (lastPos != std::string::npos)
	{
		std::string value = msg.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1);

		// remove file extension
		std::string mp3Music = value;
		AOdecode(mp3Music);
		size_t newPos = 0;
		size_t pos = 0;
		while (newPos != std::string::npos)
		{
			pos = newPos;
			newPos = mp3Music.find(".", pos+1);
			mp3_fill_buffer();
		}
		if (pos)
		{
			mp3Music = mp3Music.substr(0, pos);
			mp3_fill_buffer();
		}

		std::string mp3Lower = mp3Music+".mp3";
		std::transform(mp3Lower.begin(), mp3Lower.end(), mp3Lower.begin(), [](char c){return std::tolower(c);});

		if (musics_time)
			pSelf->musicList.push_back({value, mp3Music, mp3Lower});
		else if (value.find(".mp3") != std::string::npos || value.find(".ogg") != std::string::npos || value.find(".opus") != std::string::npos || value.find(".wav") != std::string::npos)
		{
			musics_time = true;
			pSelf->musicList.push_back({pSelf->areaList.back().name, pSelf->areaList.back().name, pSelf->areaList.back().name});
			pSelf->areaList.pop_back();
			pSelf->musicList.push_back({value, mp3Music, mp3Lower});
		}
		else
			pSelf->areaList.push_back({value, 0, "", "", ""});

		lastPos = delimiterPos;
		delimiterPos = msg.find("#", delimiterPos+1);
	}
	pSelf->musicList.pop_back(); // remove "%"

	gEngine->getSocket()->sendData("RD#%");
}

void UIScreenCourt::onMessageBN(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	std::string bgname = argumentAt(msg, 1);
	pSelf->court->getBackground()->setBg(bgname);
}

void UIScreenCourt::onMessageMC(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	std::string trackname = argumentAt(msg, 1);
	AOdecode(trackname);

	int charID = std::stoi(argumentAt(msg, 2));

	std::string showname = argumentAt(msg, 3);
	AOdecode(showname);

	// insert to chatlog
	std::string logName;
	if (!showname.empty()) logName = showname;
	else if (charID >= 0) logName = (pSelf->charList.empty()) ? ("char " + std::to_string(charID)) : pSelf->charList[charID].name;
	else logName = "Server";

	std::string logMsg = logName+" played music "+trackname;
	separateLines(0, logMsg.c_str(), 7, pSelf->icLog);
	while (pSelf->icLog.size() > 100) pSelf->icLog.erase(pSelf->icLog.begin());

	pSelf->court->playMusic("/data/ao-nds/sounds/music/"+trackname);
}

void UIScreenCourt::onMessageMS(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	int charID = std::stoi(argumentAt(msg, 9));

	if (!pSelf->icSendQueue.empty() && charID == pSelf->currChar)
		pSelf->icSendQueue.pop_front();

	if (pSelf->charList[charID].muted)
		return;

	std::string name = argumentAt(msg,16);
	if (name.empty()) name = argumentAt(msg, 3);
	AOdecode(name);

	std::string chatmsg = argumentAt(msg,5);
	AOdecode(chatmsg);

	// insert to chatlog
	std::string logMsg = name+": "+chatmsg;
	separateLines(0, logMsg.c_str(), 7, pSelf->icLog);
	while (pSelf->icLog.size() > 100) pSelf->icLog.erase(pSelf->icLog.begin());

	if (name.size() > 12)
		name.resize(12);

	// show message in court screen
	pSelf->court->getBackground()->setBgSide(argumentAt(msg,6));
	pSelf->court->MSchat(argumentAt(msg,3), argumentAt(msg,4), argumentAt(msg,2), std::stoi(argumentAt(msg,8)), name, chatmsg, std::stoi(argumentAt(msg,15)), pSelf->charList[charID].blip);
}

void UIScreenCourt::onMessageCT(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	std::string name = argumentAt(msg, 1);
	AOdecode(name);

	std::string chatmsg = argumentAt(msg, 2);
	AOdecode(chatmsg);

	// insert to chatlog
	std::string logMsg = name+": "+chatmsg;
	separateLines(0, logMsg.c_str(), 7, pSelf->oocLog);
	while (pSelf->oocLog.size() > 100) pSelf->oocLog.erase(pSelf->oocLog.begin());
}

void UIScreenCourt::onMessageCharsCheck(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	std::size_t delimiterPos = msg.find("#");
	std::size_t lastPos = delimiterPos;
	delimiterPos = msg.find("#", delimiterPos+1);

	for (u32 i=0; i<pSelf->charList.size(); i++)
	{
		int taken = std::stoi(msg.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1));
		pSelf->charList[i].taken = (taken == -1);

		lastPos = delimiterPos;
		delimiterPos = msg.find("#", delimiterPos+1);
	}
}

void UIScreenCourt::onMessagePV(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	pSelf->currChar = std::stoi(argumentAt(msg, 3));
	pSelf->charEmotes.clear();
	for (int i=0; i<6; i++)
	{
		if (pSelf->getCurrChar().side == indToSide[i])
		{
			pSelf->icControls.side = i;
			break;
		}
	}
	if (pSelf->getCurrChar().muted)
		pSelf->getCurrChar().muted = false;

	mINI::INIFile file("/data/ao-nds/characters/" + pSelf->getCurrChar().name + "/char.ini");
	mINI::INIStructure ini;

	if (file.read(ini))
	{
		u32 total = std::stoi(ini["emotions"]["number"]);

		for (u32 i=0; i<total; i++)
		{
			std::string I = std::to_string(i+1);

			std::string name = argumentAt(ini["emotions"][I], 0);
			std::string preanim = argumentAt(ini["emotions"][I], 1);
			std::string anim = argumentAt(ini["emotions"][I], 2);
			int emoteModifier = std::stoi(argumentAt(ini["emotions"][I], 3));
			std::string deskModStr = argumentAt(ini["emotions"][I], 4);
			int deskMod = std::stoi(deskModStr.empty() ? "1" : "0");

			std::string sound = (ini["soundn"].has(I)) ? ini["soundn"][I] : "1";

			int delay = std::stoi( (ini["soundt"].has(I)) ? ini["soundt"][I] : "0" );

			pSelf->charEmotes.push_back({name, preanim, anim, emoteModifier, deskMod, sound, delay});
		}
	}
}

void UIScreenCourt::onMessageFA(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	pSelf->areaList.clear();

	std::size_t delimiterPos = msg.find("#");
	std::size_t lastPos = delimiterPos;
	delimiterPos = msg.find("#", delimiterPos+1);

	while (lastPos != std::string::npos)
	{
		std::string area = msg.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1);

		pSelf->areaList.push_back({area, 0, "", "", ""});

		lastPos = delimiterPos;
		delimiterPos = msg.find("#", delimiterPos+1);
	}

	pSelf->areaList.pop_back(); // remove "%"
}

void UIScreenCourt::onMessageARUP(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	int type = std::stoi(argumentAt(msg, 1));
	for (u32 i=0; i<pSelf->areaList.size(); i++)
	{
		std::string value = argumentAt(msg, 2+i);
		switch(type)
		{
			case 0:
				pSelf->areaList[i].players = std::stoi(value);
				break;

			case 1:
				pSelf->areaList[i].status = value;
				break;

			case 2:
				pSelf->areaList[i].cm = value;
				break;

			case 3:
				pSelf->areaList[i].lock = (value.empty()) ? "Open" : value;
				break;
		}
	}
}

void UIScreenCourt::onMessageHP(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	int bar = std::stoi(argumentAt(msg, 1))-1;
	int hp = std::stoi(argumentAt(msg, 2));

	if (bar < 0 || bar > 1 || hp < 0 || hp > 10) return;
	pSelf->bars[bar] = hp;
}

void UIScreenCourt::onMessageKK(void* pUserData, std::string msg)
{
	gEngine->changeScreen(new UIScreenDisconnected("You have been kicked", argumentAt(msg, 1), false));
}

void UIScreenCourt::onMessageKB(void* pUserData, std::string msg)
{
	gEngine->changeScreen(new UIScreenDisconnected("You have been banned", argumentAt(msg, 1), false));
}

void UIScreenCourt::onMessageBD(void* pUserData, std::string msg)
{
	gEngine->changeScreen(new UIScreenDisconnected("You are banned from this server", argumentAt(msg, 1), false));
}
