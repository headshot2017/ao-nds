#include "ui/uicourt.h"

#include <string.h>
#include <algorithm>
#include <array>

#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <nds/arm9/input.h>
#include <nds/interrupts.h>

#include "libadx.h"
#include "utf8.h"
#include "fonts.h"
#include "global.h"
#include "engine.h"
#include "content.h"
#include "settings.h"
#include "mini/ini.h"
#include "ui/uiserverlist.h"
#include "ui/uidisconnected.h"
#include "ui/court/loading.h"
#include "ui/court/charselect.h"
#include "ui/court/console.h"
#include "ui/court/message.h"
#include "mem.h"

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

	wav_free_handle(sndSelect);
	wav_free_handle(sndCancel);
	wav_free_handle(sndEvTap);
	wav_free_handle(sndEvPage);
	wav_free_handle(sndCrtRcrd);

	AOsocket* sock = gEngine->getSocket();
	sock->clearCallbacks();
	gEngine->setSocket(nullptr);

	bgExtPaletteDisable();
}

void UIScreenCourt::init()
{
	bgExtPaletteEnable();
	currChar = -1;
	sendTicks = 0;
	receiveTicks = 0;
	loggedIn = false;
	guard = false;
	memset(bars, 0, sizeof(bars));

	showname = Settings::defaultShowname;
	oocName = Settings::defaultOOCname;

	isConsole = !!(keysHeld() & KEY_SELECT);
	if (isConsole)
		subScreen = new UICourtConsole(this);
	else
		subScreen = new UICourtLoading(this);
	subScreen->init();

	court = new Courtroom;
	court->setVisible(true);

	sndSelect = wav_load_handle("/data/ao-nds/sounds/general/sfx-selectblip2.wav");
	sndCancel = wav_load_handle("/data/ao-nds/sounds/general/sfx-cancel.wav");
	sndEvTap = wav_load_handle("/data/ao-nds/sounds/general/sfx-selectblip.wav");
	sndEvPage = wav_load_handle("/data/ao-nds/sounds/general/sfx-blink.wav");
	sndEvShow = court->getEvidence()->sndEvShow;
	sndCrtRcrd = wav_load_handle("/data/ao-nds/sounds/general/sfx-scroll.wav");

	AOsocket* sock = gEngine->getSocket();
	sock->addMessageCallback("ID", onMessageID, this);
	sock->addMessageCallback("PN", onMessagePN, this);
	sock->addMessageCallback("FL", onMessageFL, this);
	sock->addMessageCallback("SI", onMessageSI, this);
	sock->addMessageCallback("SC", onMessageSC, this);
	sock->addMessageCallback("SM", onMessageSM, this);
	sock->addMessageCallback("BN", onMessageBN, this);
	sock->addMessageCallback("MC", onMessageMC, this);
	sock->addMessageCallback("MS", onMessageMS, this);
	sock->addMessageCallback("CT", onMessageCT, this);
	sock->addMessageCallback("RT", onMessageRT, this);
	sock->addMessageCallback("CharsCheck", onMessageCharsCheck, this);
	sock->addMessageCallback("PV", onMessagePV, this);
	sock->addMessageCallback("DONE", onMessageDONE, this);
	sock->addMessageCallback("FA", onMessageFA, this);
	sock->addMessageCallback("ARUP", onMessageARUP, this);
	sock->addMessageCallback("LE", onMessageLE, this);
	sock->addMessageCallback("HP", onMessageHP, this);
	sock->addMessageCallback("KK", onMessageKK, this);
	sock->addMessageCallback("KB", onMessageKB, this);
	sock->addMessageCallback("BD", onMessageBD, this);
	sock->addMessageCallback("BB", onMessageBB, this);
	sock->addMessageCallback("AUTH", onMessageAUTH, this);
	sock->addMessageCallback("ZZ", onMessageZZ, this);
	sock->addMessageCallback("PR", onMessagePR, this);
	sock->addMessageCallback("PU", onMessagePU, this);

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

	if (!gEngine->getSocket()->isConnected() && !gEngine->isFading())
	{
		gEngine->changeScreen(new UIScreenDisconnected("Disconnected", "Connection to server lost.", false));
	}

	if (receiveTicks <= 0)
	{
		if (!icReceiveQueue.empty())
		{
			court->MSchat(icReceiveQueue.front());
			icReceiveQueue.pop_front();
			receiveTicks = 5;
		}
		else if (!rtReceiveQueue.empty())
		{
			court->getWTCE()->play(rtReceiveQueue.front());
			rtReceiveQueue.pop_front();
			receiveTicks = 5;
		}
	}
	else
		receiveTicks--;

	court->update();
	subScreen->update();

	if (!icSendQueue.empty())
	{
		sendTicks--;
		if (sendTicks <= 0)
		{
			gEngine->getSocket()->sendData(icSendQueue.front());
			sendTicks = 10;
		}
	}
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
	gEngine->getSocket()->sendData("ID#AO-NDS#2.11.0#%");
}

void UIScreenCourt::onMessagePN(void* pUserData, std::string msg)
{
	gEngine->getSocket()->sendData("askchaa#%");
}

void UIScreenCourt::onMessageFL(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	u32 features = totalArguments(msg)-1;
	for (u32 i=0; i<features; i++)
		pSelf->featureList.insert(argumentAt(msg, 1+i));
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

		mINI::INIFile file(Content::getFile("characters/" + name + "/char.ini"));
		mINI::INIStructure ini;

		if (file.read(ini))
		{
			showname = ini["options"]["showname"];
			if (showname.empty()) showname = name;

			blip = ini["options"]["blips"];
			if (blip.empty()) blip = ini["options"]["gender"];

			side = ini["options"]["side"];
		}

		pSelf->charList.push_back({name, utf8::utf8to16(showname), blip, side, false, false});

		lastPos = delimiterPos;
		delimiterPos = msg.find("#", delimiterPos+1);
	}

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
		if (value.empty() || value.at(0) == '%') break;

		std::string adxMusic = value;
		AOdecode(adxMusic);

		// remove file extension
		size_t newPos = 0;
		size_t pos = 0;
		while (newPos != std::string::npos)
		{
			pos = newPos;
			newPos = adxMusic.find(".", pos+1);
			adx_update();
		}
		if (pos)
		{
			adxMusic = adxMusic.substr(0, pos);
			adx_update();
		}

		std::string adxLower = adxMusic+".adx";
		std::transform(adxLower.begin(), adxLower.end(), adxLower.begin(), [](char c){return std::tolower(c);});

		// remove category
		newPos = 0;
		pos = 0;
		while (newPos != std::string::npos)
		{
			pos = newPos;
			newPos = adxMusic.find("/", pos+1);
			adx_update();
		}
		if (pos)
		{
			adxMusic = adxMusic.substr(pos+1);
			adx_update();
		}

		if (musics_time)
			pSelf->musicList.push_back({value, utf8::utf8to16(adxMusic), adxLower});
		else if (value.find(".mp3") != std::string::npos || value.find(".ogg") != std::string::npos || value.find(".opus") != std::string::npos || value.find(".wav") != std::string::npos)
		{
			musics_time = true;
			pSelf->musicList.push_back({pSelf->areaList.back().name, utf8::utf8to16(pSelf->areaList.back().name), pSelf->areaList.back().name});
			pSelf->areaList.pop_back();
			pSelf->musicList.push_back({value, utf8::utf8to16(adxMusic), adxLower});
		}
		else
			pSelf->areaList.push_back({value, 0, "", "", ""});

		lastPos = delimiterPos;
		delimiterPos = msg.find("#", delimiterPos+1);
	}

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

	std::u16string logMsg = utf8::utf8to16(logName+" played music "+trackname);
	separateLines(0, logMsg, 7, false, pSelf->icLog);

	auto pos = trackname.find_last_of('.');
	if (pos != std::string::npos)
		trackname = trackname.substr(0, pos) + ".adx";

	pSelf->court->playMusic(Content::getFile("sounds/music/" + trackname));
}

void UIScreenCourt::onMessageMS(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	int charID = std::stoi(argumentAt(msg, 9));

	if (!pSelf->icSendQueue.empty() && charID == pSelf->currChar)
		pSelf->icSendQueue.pop_front();

	if (pSelf->charList.empty() || pSelf->charList[charID].muted)
		return;

	u32 argc = totalArguments(msg);

	std::string charname = argumentAt(msg, 3);
	std::string showname = argumentAt(msg, 16);
	AOdecode(charname);
	AOdecode(showname);

	std::u16string chatmsg = utf8::utf8to16(argumentAt(msg,5));
	AOdecode(chatmsg);

	std::string shoutModStr = argumentAt(msg, 11);
	AOdecode(shoutModStr);

	int shoutMod = 0;
	std::string customShout = "custom";
	if (shoutModStr.find("&") != std::string::npos)
	{
		shoutMod = std::stoi(argumentAt(shoutModStr, 0, '&'));
		customShout = "custom_objections/" + argumentAt(shoutModStr, 1, '&');

		// remove file extension
		auto pos = customShout.find_last_of('.');
		if (pos != std::string::npos)
			customShout = customShout.substr(0, pos);
	}
	else
		shoutMod = std::stoi(shoutModStr);

	// insert to chatlog
	std::string name = pSelf->charList[charID].name;
	if (Settings::chatlogIniswaps && name != charname)
		name += " (" + charname + ")";
	if (Settings::chatlogShownames && !showname.empty())
		name += " [" + showname + "]";

	std::u16string logMsg = utf8::utf8to16(name+": ") + chatmsg;
	separateLines(0, logMsg, 7, false, pSelf->icLog);


	std::string lowerCharname = charname;
	std::transform(lowerCharname.begin(), lowerCharname.end(), lowerCharname.begin(), [](char c){return std::tolower(c);});

	int evidence = std::stoi(argumentAt(msg, 12))-1;
	std::string evidenceImage;
	if (evidence >= 0) evidenceImage = pSelf->evidenceList[evidence].image;

	// show message in court screen
	MSchatStruct data;
	data.deskMod = argumentAt(msg, 1) != "0";
	data.preanim = argumentAt(msg, 2);
	data.charname = charname;
	data.emote = argumentAt(msg, 4);
	data.chatmsg = chatmsg;
	data.side = argumentAt(msg, 6);
	data.sfx = argumentAt(msg, 7);
	data.emoteMod = std::stoi(argumentAt(msg, 8));
	data.charID = charID;
	data.sfxDelay = std::stoi(argumentAt(msg, 10));
	data.shoutMod = shoutMod;
	data.customShout = customShout;
	data.evidence = evidenceImage;
	data.flip = argumentAt(msg, 13) == "1";
	data.realization = std::stoi(argumentAt(msg, 14));
	data.textColor = std::stoi(argumentAt(msg, 15));
	data.showname = utf8::utf8to16(showname);
	data.otherCharID = std::stoi(argumentAt(msg, 17));
	data.otherName = argumentAt(msg, 18);
	data.otherEmote = argumentAt(msg, 19);
	data.selfOffset = argumentAt(msg, 20);
	data.otherOffset = argumentAt(msg, 21);
	data.otherFlip = std::stoi(argumentAt(msg, 22));
	data.noInterrupt = argumentAt(msg, 23) == "1";
	data.sfxLoop = std::stoi(argumentAt(msg, 24));
	data.shake = std::stoi(argumentAt(msg, 25));
	data.frameShake = argumentAt(msg, 26);
	data.frameFlash = argumentAt(msg, 27);
	data.frameSFX = argumentAt(msg, 28);
	data.additive = argumentAt(msg, 29) == "1";
	data.blip = (argc >= 32) ? argumentAt(msg, 31) : Content::getCharBlip(lowerCharname);
	data.panCourt = (argc >= 33 && argumentAt(msg, 32) == "1");
	data.chatbox = Content::getCharChatbox(lowerCharname);
	if (data.blip.empty()) data.blip = pSelf->charList[charID].blip;
	if (data.chatbox.empty()) data.chatbox = Settings::defaultChatbox;

	pSelf->icReceiveQueue.push_back(data);
}

void UIScreenCourt::onMessageCT(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	std::string name = argumentAt(msg, 1);
	AOdecode(name);

	std::string chatmsg = argumentAt(msg, 2);
	AOdecode(chatmsg);

	bool isServer = (argumentAt(msg, 3) == "1");

	// insert to chatlog
	std::u16string logMsg = utf8::utf8to16(name+": "+chatmsg);
	separateLines(0, logMsg, 7, false, pSelf->oocLog);

	if (isServer)
	{
		// UGLY!!! there should be an argument in the network msg to tell if it's an error or not
		const char* messageList[] = {
			"Blankposting",
			"disabled in",
			"forbidden in",
			"You are blocked",
			"a muted area",
			"custom emotes",
			"a repeat",
			"is too long",
			"client(s) with ipid",
		};

		for (u32 i=0; i<sizeof(messageList) / sizeof(const char*); i++)
		{
			if (chatmsg.find(messageList[i]) != std::string::npos)
			{
				pSelf->icSendQueue.clear();
				pSelf->changeScreen(new UICourtMessage(pSelf, chatmsg));
				break;
			}
		}

		if (chatmsg.find("Logged in as") != std::string::npos)
		{
			pSelf->loggedIn = true;
			std::u16string logMsg = u"You can now enable Guard Mode in the main menu. This will allow you to receive modcalls.";
			separateLines(0, logMsg, 7, false, pSelf->oocLog);
		}
		else if (chatmsg.find("no longer a mod") != std::string::npos)
			pSelf->loggedIn = false;
	}
}

void UIScreenCourt::onMessageRT(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	pSelf->rtReceiveQueue.push_back(msg);
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
	pSelf->charShouts.clear();

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

	mINI::INIFile file(Content::getFile("characters/" + pSelf->getCurrChar().name + "/char.ini"));
	mINI::INIStructure ini;

	if (file.read(ini))
	{
		// load emotes
		u32 total = std::stoi(ini["emotions"]["number"]);

		for (u32 i=0; i<total; i++)
		{
			std::string I = std::to_string(i+1);

			std::string preanim = argumentAt(ini["emotions"][I], 1);
			std::string anim = argumentAt(ini["emotions"][I], 2);
			std::string emoteModStr = argumentAt(ini["emotions"][I], 3);
			int emoteModifier = (!emoteModStr.empty()) ? std::stoi(emoteModStr) : 0;
			std::string deskModStr = argumentAt(ini["emotions"][I], 4);
			int deskMod = 1;
			if (totalArguments(ini["emotions"][I]) >= 5)
			{
				std::string deskModStr = argumentAt(ini["emotions"][I], 4);
				deskMod = (!deskModStr.empty()) ? std::stoi(deskModStr) : 1;
			}

			std::string sound = (ini["soundn"].has(I)) ? ini["soundn"][I] : "1";

			int delay = 0;
			if (ini["soundt"].has(I))
			{
				std::string delayStr = ini["soundT"][I];
				delay = (!delayStr.empty()) ? std::stoi(delayStr) : 0;
			}

			// set up frameSFX, frameshake, frameflash
			/// franziska-damage|1=sfx-stab^(b)franziska-mad^(a)franziska-mad|12=sfx-deskslam|16=sfx-deskslam|4=sfx-deskslam^
			/// franziska-damage|1=1|2=1|3=1^(b)franziska-mad^(a)franziska-mad^
			/// franziska-damage^(b)franziska-mad^(a)franziska-mad^
			std::array<std::string, 3> frameEmotes = {preanim, "(b)"+anim, "(a)"+anim};
			std::array<std::string, 3> frameStrings = {"", "", ""};
			std::array<std::string, 3> types = {"_FrameSFX", "_FrameScreenshake", "_FrameRealization"};

			for (int j=0; j<3; j++) // frameStrings[j], types[j]
			{
				for (int k=0; k<3; k++) // frameEmotes[k]
				{
					frameStrings[j] += frameEmotes[k];
					std::string section = frameEmotes[k] + types[j];
					if (ini.has(section))
					{
						for (auto const& frame : ini[section])
							frameStrings[j] += "|" + frame.first + "=" + frame.second;
					}
					frameStrings[j] += '^';
				}
			}

			pSelf->charEmotes.push_back({preanim, anim, frameStrings[0], frameStrings[1], frameStrings[2], emoteModifier, deskMod, sound, delay});
		}

		// load custom objections
		if (ini.has("shouts"))
		{
			for (auto const& shoutKey : ini["shouts"])
			{
				std::string filename = argumentAt(shoutKey.first, 0, '_');
				if (filename == "custom") filename.clear();
				std::string type = argumentAt(shoutKey.first, 1, '_');
				if (type != "name") continue;

				pSelf->charShouts.push_back({filename, shoutKey.second});
			}
		}
	}
}

void UIScreenCourt::onMessageDONE(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;
	if (!pSelf->isConsole)
		pSelf->changeScreen(new UICourtCharSelect(pSelf));
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

		if (!area.empty()) pSelf->areaList.push_back({area, 0, "", "", ""});

		lastPos = delimiterPos;
		delimiterPos = msg.find("#", delimiterPos+1);
	}
}

void UIScreenCourt::onMessageARUP(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	int type = std::stoi(argumentAt(msg, 1));
	for (u32 i=0; i<pSelf->areaList.size(); i++)
	{
		std::string value = argumentAt(msg, 2+i);
		if (value.empty() || value.at(0) == '%') break;

		switch (type)
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

void UIScreenCourt::onMessageLE(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	pSelf->evidenceList.clear();

	std::size_t delimiterPos = msg.find("#");
	std::size_t lastPos = delimiterPos;
	delimiterPos = msg.find("#", delimiterPos+1);

	while (lastPos != std::string::npos)
	{
		std::string info = msg.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1);
		if (info.empty() || info.at(0) == '%') break;

		std::u16string name = utf8::utf8to16(argumentAt(info, 0, '&'));
		std::u16string desc = utf8::utf8to16(argumentAt(info, 1, '&'));
		std::string image = argumentAt(info, 2, '&');
		AOdecode(name);
		AOdecode(desc);
		AOdecode(image);

		// remove file extension from image
		size_t newExtPos = 0;
		size_t extPos = 0;
		while (newExtPos != std::string::npos)
		{
			extPos = newExtPos;
			newExtPos = image.find(".", extPos+1);
			adx_update();
		}
		if (extPos)
		{
			image = image.substr(0, extPos);
			adx_update();
		}

		pSelf->evidenceList.push_back({name, desc, image});

		lastPos = delimiterPos;
		delimiterPos = msg.find("#", delimiterPos+1);
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

void UIScreenCourt::onMessageBB(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;
	pSelf->changeScreen(new UICourtMessage(pSelf, argumentAt(msg, 1)));
}

void UIScreenCourt::onMessageAUTH(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	std::string auth = argumentAt(msg, 1);
	pSelf->loggedIn = (auth == "1");
	if (pSelf->loggedIn)
	{
		std::u16string logMsg = u"You can now enable Guard Mode in the main menu. This will allow you to receive modcalls.";
		separateLines(0, logMsg, 7, false, pSelf->oocLog);
	}
}

void UIScreenCourt::onMessageZZ(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	if (!pSelf->guard) return;

	std::u16string modcall = utf8::utf8to16(argumentAt(msg, 1));
	separateLines(0, modcall, 7, false, pSelf->oocLog);

	pSelf->changeScreen(new UICourtMessage(pSelf, modcall));
}

void UIScreenCourt::onMessagePR(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	int id = std::stoi(argumentAt(msg, 1));
	int action = std::stoi(argumentAt(msg, 2));

	switch (action)
	{
		case 0: // Add
			pSelf->playerList[id] = {};
			pSelf->playerList[id].area = -1;
			pSelf->playerListIDs.push_back(id);
			break;

		case 1: // Remove
			{
				const auto& iter = std::find(pSelf->playerListIDs.begin(), pSelf->playerListIDs.end(), id);
				if (pSelf->playerList.count(id))
					pSelf->playerList.erase(id);
				if (iter != pSelf->playerListIDs.end())
					pSelf->playerListIDs.erase(iter);
			}
			break;
	}
}

void UIScreenCourt::onMessagePU(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	int id = std::stoi(argumentAt(msg, 1));
	int dataType = std::stoi(argumentAt(msg, 2));
	std::string data = argumentAt(msg, 3);

	if (!pSelf->playerList.count(id)) return;

	switch (dataType)
	{
		case 0:
			pSelf->playerList[id].oocName = data;
			break;

		case 1:
			pSelf->playerList[id].character = data;
			break;

		case 2:
			pSelf->playerList[id].showname = data;
			break;

		case 3:
			pSelf->playerList[id].area = std::stoi(data);
			break;
	}
}
