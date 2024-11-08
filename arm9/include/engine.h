#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ui/uiscreen.h"
#include "sockets/aosocket.h"

struct evidenceInfo
{
	std::u16string name;
	std::u16string description;
	std::string image;
};

struct evidenceCacheInfo
{
	std::string name;
	std::string subdir;
};

class Engine
{
	UIScreen* screen;
	UIScreen* nextScreen;
	AOsocket* aosocket;
	std::string macAddr;
	std::unordered_set<std::string> cachedMusic;
	std::vector<evidenceCacheInfo> cachedEvidence;
	std::unordered_map<std::string, std::string> cachedCharBlips;
	std::vector<evidenceInfo> privateEvidence;

	std::u16string defaultShowname;
	std::u16string defaultOOCname;
	bool chatlogIniswaps;
	bool chatlogShownames;

	int alpha;
	bool fading;
	bool running;
	bool wifiSwitch;

	void cacheMusic(const std::string& folder, std::string extra="");
	void cacheEvidence(const std::string& folder, std::string extra="");
	void cacheCharBlips(const std::string& folder);
	void loadPrivateEvidence();

public:
	Engine();
	~Engine();

	UIScreen* getScreen() {return screen;}
	AOsocket* getSocket() {return aosocket;}
	const std::string& getMacAddr() {return macAddr;}
	const std::u16string& getShowname() {return defaultShowname;}
	const std::u16string& getOOCname() {return defaultOOCname;}
	bool showChatlogIniswaps() {return chatlogIniswaps;}
	bool showChatlogShownames() {return chatlogShownames;}
	const std::vector<evidenceCacheInfo>& getEvidence() {return cachedEvidence;}
	const std::string getCharBlip(const std::string& charname) {return (cachedCharBlips.count(charname)) ? cachedCharBlips[charname] : "";}
	std::vector<evidenceInfo>& getPrivateEvidence() {return privateEvidence;}
	bool isFading() {return fading;}
	bool isRunning() {return running;}
	bool musicExists(const std::string& file) {return cachedMusic.count(file);}

	void changeScreen(UIScreen* next);
	void setSocket(AOsocket* sock);
	void setMacAddr(std::string addr) {macAddr = addr;}
	void setShowname(std::u16string val) {defaultShowname = val;}
	void setOOCname(std::u16string val) {defaultOOCname = val;}
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
