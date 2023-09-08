#include "ui/uicourt.h"

#include <string.h>

#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <nds/arm9/input.h>
#include <nds/interrupts.h>

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

	AOsocket* sock = gEngine->getSocket();
	sock->clearCallbacks();
	gEngine->setSocket(nullptr);

	bgExtPaletteDisable();
}

void UIScreenCourt::init()
{
	bgExtPaletteEnable();

	subScreen = new UICourtLoading(this);
	subScreen->init();

	court = new Courtroom;
	court->setVisible(true);

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
	std::string hdid = "HI#NDS " + gEngine->getMacAddr() + "#%\n";
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
	gEngine->getSocket()->sendData("RC#%");
}

void UIScreenCourt::onMessageSC(void* pUserData, std::string msg)
{
	gEngine->getSocket()->sendData("RM#%");
}

void UIScreenCourt::onMessageSM(void* pUserData, std::string msg)
{
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
