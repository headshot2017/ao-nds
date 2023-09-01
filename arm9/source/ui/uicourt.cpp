#include "ui/uicourt.h"

#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <nds/arm9/input.h>
#include <nds/interrupts.h>

#include "engine.h"
#include "global.h"

UIScreenCourt::UIScreenCourt() : UIScreen()
{
	court = 0;
}

UIScreenCourt::~UIScreenCourt()
{
	if (court)
	{
		court->setVisible(false);
		delete court;
	}
	bgExtPaletteDisable();
}

void UIScreenCourt::init()
{
	bgExtPaletteEnable();

	// printconsole will be removed once UI work actually begins
	consoleInit(0, consoleGetDefault()->bgLayer, BgType_Text4bpp, BgSize_T_256x256, consoleGetDefault()->mapBase, consoleGetDefault()->gfxBase, false, true);

	court = new Courtroom;
	court->setVisible(true);

	AOsocket* sock = gEngine->getSocket();
	sock->setMessageCallback("decryptor", onMessageDecryptor, this);
	sock->setMessageCallback("ID", onMessageID, this);
	sock->setMessageCallback("PN", onMessagePN, this);
	sock->setMessageCallback("SI", onMessageSI, this);
	sock->setMessageCallback("SC", onMessageSC, this);
	sock->setMessageCallback("SM", onMessageSM, this);
	sock->setMessageCallback("BN", onMessageBN, this);
	sock->setMessageCallback("MC", onMessageMC, this);
	sock->setMessageCallback("MS", onMessageMS, this);
}

void UIScreenCourt::updateInput()
{
	if (keysDown() & KEY_SELECT)
	{
		iprintf("disconnecting\n");
		gEngine->getSocket()->disconnect();
	}
}

void UIScreenCourt::update()
{
	court->update();
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
