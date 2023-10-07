#ifndef UICOURT_H_INCLUDED
#define UICOURT_H_INCLUDED

#include <string>
#include <vector>

#include "uiscreen.h"
#include "courtroom/courtroom.h"

struct charInfo
{
	std::string name;
	std::string showname;
	std::string blip;
	std::string side;
	bool taken;
};

struct musicInfo
{
	std::string name;
	std::string nameDecoded;
	std::string nameLower;
};

struct areaInfo
{
	std::string name;
	int players;
	std::string status;
	std::string cm;
	std::string lock;
};

class UIScreenCourt;
class UISubScreen
{
public:
	UISubScreen(UIScreenCourt* courtUI) : pCourtUI(courtUI) {}
	virtual ~UISubScreen() {}

	virtual void init() {}
	virtual void updateInput() {}
	virtual void update() {}

protected:
	UIScreenCourt* pCourtUI;
};

class UIScreenCourt : public UIScreen
{
	Courtroom* court;
	UISubScreen* subScreen;
	UISubScreen* nextScreen;

	std::vector<charInfo> charList;
	std::vector<musicInfo> musicList;
	std::vector<areaInfo> areaList;

	std::vector<std::string> icLog;
	std::vector<std::string> oocLog;

	int currChar;

public:
	UIScreenCourt();
	~UIScreenCourt();

	void init();
	void updateInput();
	void update();
	void changeScreen(UISubScreen* newScreen);

	const std::vector<charInfo>& getCharList() {return charList;}
	const std::vector<musicInfo>& getMusicList() {return musicList;}
	const std::vector<areaInfo>& getAreaList() {return areaList;}
	const std::vector<std::string>& getICLog() {return icLog;}
	const std::vector<std::string>& getOOCLog() {return oocLog;}
	int getCurrCharID() {return currChar;}
	const charInfo& getCurrChar() {return charList[currChar];}

	static void onMessageID(void* pUserData, std::string msg);
	static void onMessagePN(void* pUserData, std::string msg);
	static void onMessageSI(void* pUserData, std::string msg);
	static void onMessageSC(void* pUserData, std::string msg);
	static void onMessageSM(void* pUserData, std::string msg);
	static void onMessageBN(void* pUserData, std::string msg);
	static void onMessageMC(void* pUserData, std::string msg);
	static void onMessageMS(void* pUserData, std::string msg);
	static void onMessageCT(void* pUserData, std::string msg);
	static void onMessageCharsCheck(void* pUserData, std::string msg);
	static void onMessagePV(void* pUserData, std::string msg);
	static void onMessageARUP(void* pUserData, std::string msg);
	static void onMessageKK(void* pUserData, std::string msg);
	static void onMessageKB(void* pUserData, std::string msg);
	static void onMessageBD(void* pUserData, std::string msg);

	u32* sndSelect;
	u32* sndCancel;
	u32* sndEvTap;
	u32* sndEvPage;
	u32 sndSelectSize;
	u32 sndCancelSize;
	u32 sndEvTapSize;
	u32 sndEvPageSize;

	std::string showname;
	std::string oocName;
};

#endif // UICOURT_H_INCLUDED
