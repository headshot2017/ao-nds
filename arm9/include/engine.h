#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include "ui/uiscreen.h"
#include "sockets/aosocket.h"

class Engine
{
	UIScreen* screen;
	UIScreen* nextScreen;
	AOsocket* aosocket;
	std::string macAddr;

public:
	Engine();
	~Engine();

	UIScreen* getScreen() {return screen;}
	AOsocket* getSocket() {return aosocket;}
	const std::string& getMacAddr() {return macAddr;}

	void changeScreen(UIScreen* next) {nextScreen = next;}
	void setSocket(AOsocket* sock) {aosocket = sock;}
	void setMacAddr(std::string addr) {macAddr = addr;}

	void updateInput();
	void update();
};

extern Engine* gEngine;

#endif // ENGINE_H_INCLUDED
