#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include <unordered_map>

#include "ui/uiscreen.h"
#include "sockets/aosocket.h"

class Engine
{
	UIScreen* screen;
	UIScreen* nextScreen;
	AOsocket* aosocket;
	std::string macAddr;
	std::unordered_map<std::string, bool> cachedMusic;

	int alpha;
	bool fading;
	bool running;

	void cacheMusic(const std::string& folder, std::string extra="");

public:
	Engine();
	~Engine();

	UIScreen* getScreen() {return screen;}
	AOsocket* getSocket() {return aosocket;}
	const std::string& getMacAddr() {return macAddr;}
	bool isFading() {return fading;}
	bool isRunning() {return running;}
	bool musicExists(const std::string& file) {return cachedMusic.count(file);}

	void changeScreen(UIScreen* next);
	void setSocket(AOsocket* sock);
	void setMacAddr(std::string addr) {macAddr = addr;}

	void updateInput();
	void update();

	void quit();
};

extern Engine* gEngine;

#endif // ENGINE_H_INCLUDED
