#include "ui/uicourt.h"

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
}

void UIScreenCourt::init()
{
	court = new Courtroom;
	court->setVisible(true);

	AOsocket* sock = gEngine->getSocket();
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

}

void UIScreenCourt::update()
{
	court->update();
}

void UIScreenCourt::onMessageID(void* pUserData, std::string msg)
{
	gEngine->getSocket()->send("ID#ndsAO#0.0.1#%");
}

void UIScreenCourt::onMessagePN(void* pUserData, std::string msg)
{
	gEngine->getSocket()->send("askchaa#%");
}

void UIScreenCourt::onMessageSI(void* pUserData, std::string msg)
{
	gEngine->getSocket()->send("RC#%");
}

void UIScreenCourt::onMessageSC(void* pUserData, std::string msg)
{
	gEngine->getSocket()->send("RM#%");
}

void UIScreenCourt::onMessageSM(void* pUserData, std::string msg)
{
	gEngine->getSocket()->send("RD#%");
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
	if (name.size() > 10)
		name.resize(10);

	pSelf->court->getBackground()->setBgSide(argumentAt(msg,6));
	pSelf->court->MSchat(argumentAt(msg,3), argumentAt(msg,4), argumentAt(msg,2), std::stoi(argumentAt(msg,8)), name, argumentAt(msg,5), std::stoi(argumentAt(msg,15)), "male");
}
