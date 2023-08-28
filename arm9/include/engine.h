#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include "ui/uiscreen.h"
#include "sockets/aosocket.h"

class Engine
{
	UIScreen* screen;
	UIScreen* nextScreen;
	AOsocket* aosocket;

public:
	Engine();
	~Engine();

	UIScreen* getScreen() {return screen;}
	AOsocket* getSocket() {return aosocket;}

	void changeScreen(UIScreen* next) {nextScreen = next;}
	void setSocket(AOsocket* sock) {aosocket = sock;}

	void updateInput();
	void update();
};

extern Engine* gEngine;

#endif // ENGINE_H_INCLUDED
