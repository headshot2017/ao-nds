#include "ui/uicourt.h"

#include <string.h>

#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <nds/arm9/input.h>
#include <nds/interrupts.h>

#include "mp3_shared.h"
#include "engine.h"
#include "global.h"
#include "ui/uiserverlist.h"
#include "ui/court/loading.h"

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

	subScreen = new UICourtLoading(this);
	subScreen->init();

	court = new Courtroom;
	court->setVisible(true);

	sndSelect = wav_load_handle("/data/ao-nds/sounds/general/sfx-selectblip2.wav", &sndSelectSize);
	sndCancel = wav_load_handle("/data/ao-nds/sounds/general/sfx-cancel.wav", &sndCancelSize);
	sndEvTap = wav_load_handle("/data/ao-nds/sounds/general/sfx-selectblip.wav", &sndEvTapSize);
	sndEvPage = wav_load_handle("/data/ao-nds/sounds/general/sfx-blink.wav", &sndEvPageSize);

	AOsocket* sock = gEngine->getSocket();
	sock->addMessageCallback("decryptor", onMessageDecryptor, this);
	sock->addMessageCallback("ID", onMessageID, this);
	sock->addMessageCallback("PN", onMessagePN, this);
	sock->addMessageCallback("SI", onMessageSI, this);
	sock->addMessageCallback("SC", onMessageSC, this);
	sock->addMessageCallback("SM", onMessageSM, this);
	sock->addMessageCallback("BN", onMessageBN, this);
	sock->addMessageCallback("MC", onMessageMC, this);
	sock->addMessageCallback("MS", onMessageMS, this);
	sock->addMessageCallback("CharsCheck", onMessageCharsCheck, this);
	sock->addMessageCallback("PV", onMessagePV, this);
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
}

void UIScreenCourt::changeScreen(UISubScreen* newScreen)
{
	nextScreen = newScreen;
}

void UIScreenCourt::onMessageDecryptor(void* pUserData, std::string msg)
{
	std::string hdid = "HI#NDS " + gEngine->getMacAddr() + "#%";
	gEngine->getSocket()->sendData(hdid);
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
		pSelf->charList.push_back({msg.substr((lastPos == 0) ? lastPos : lastPos+1, delimiterPos-lastPos-1), false});

		lastPos = delimiterPos;
		delimiterPos = msg.find("#", delimiterPos+1);
	}

	pSelf->charList.pop_back(); // remove "%"

	gEngine->getSocket()->sendData("RM#%");
}

void UIScreenCourt::onMessageSM(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	fillArguments(pSelf->musicList, msg, 1);
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
	pSelf->court->playMusic("/data/ao-nds/sounds/music/"+trackname);
}

void UIScreenCourt::onMessageMS(void* pUserData, std::string msg)
{
	UIScreenCourt* pSelf = (UIScreenCourt*)pUserData;

	std::string name = argumentAt(msg,16);
	if (name.empty())
		name = argumentAt(msg, 3);
	if (name.size() > 12)
		name.resize(12);

	pSelf->court->getBackground()->setBgSide(argumentAt(msg,6));
	pSelf->court->MSchat(argumentAt(msg,3), argumentAt(msg,4), argumentAt(msg,2), std::stoi(argumentAt(msg,8)), name, argumentAt(msg,5), std::stoi(argumentAt(msg,15)), "male");
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
}
