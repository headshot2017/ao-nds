#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include <unordered_map>
#include <vector>

#include "ui/uiscreen.h"
#include "sockets/aosocket.h"

struct evidenceInfo
{
	std::string name;
	std::string description;
	std::string image;
};

class Engine
{
	UIScreen* screen;
	UIScreen* nextScreen;
	AOsocket* aosocket;
	std::string macAddr;
	std::unordered_map<std::string, bool> cachedMusic;
	std::vector<std::string> cachedEvidence;
	std::unordered_map<std::string, std::string> cachedCharBlips;
	std::vector<evidenceInfo> privateEvidence;

	std::string defaultShowname;
	std::string defaultOOCname;
	bool chatlogIniswaps;
	bool chatlogShownames;

	int alpha;
	bool fading;
	bool running;
	bool wifiSwitch;

	void cacheMusic(const std::string& folder, std::string extra="");
	void cacheEvidence(const std::string& folder);
	void cacheCharBlips(const std::string& folder);
	void loadPrivateEvidence();

public:
	Engine();
	~Engine();

	UIScreen* getScreen() {return screen;}
	AOsocket* getSocket() {return aosocket;}
	const std::string& getMacAddr() {return macAddr;}
	const std::string& getShowname() {return defaultShowname;}
	const std::string& getOOCname() {return defaultOOCname;}
	const bool showChatlogIniswaps() {return chatlogIniswaps;}
	const bool showChatlogShownames() {return chatlogShownames;}
	const std::vector<std::string>& getEvidence() {return cachedEvidence;}
	const std::string getCharBlip(const std::string& charname) {return (cachedCharBlips.count(charname)) ? cachedCharBlips[charname] : "";}
	std::vector<evidenceInfo>& getPrivateEvidence() {return privateEvidence;}
	bool isFading() {return fading;}
	bool isRunning() {return running;}
	bool musicExists(const std::string& file) {return cachedMusic.count(file);}

	void changeScreen(UIScreen* next);
	void setSocket(AOsocket* sock);
	void setMacAddr(std::string addr) {macAddr = addr;}
	void setShowname(std::string val) {defaultShowname = val;}
	void setOOCname(std::string val) {defaultOOCname = val;}
	void setChatlogIniswaps(bool val) {chatlogIniswaps = val;}
	void setChatlogShownames(bool val) {chatlogShownames = val;}
	void resetWifiSwitch() {wifiSwitch = true;}

	void updateInput();
	void update();

	void savePrivateEvidence();

	void quit();
};

extern Engine* gEngine;

#endif // ENGINE_H_INCLUDED
