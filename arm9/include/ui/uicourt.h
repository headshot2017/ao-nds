#ifndef UICOURT_H_INCLUDED
#define UICOURT_H_INCLUDED

#include <string>
#include <vector>
#include <deque>

#include "uiscreen.h"
#include "courtroom/courtroom.h"

extern const char* indToSide[6];

struct charInfo
{
	std::string name;
	std::string showname;
	std::string blip;
	std::string side;
	bool taken;
	bool muted;
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

struct emoteInfo
{
	std::string name;
	std::string preanim;
	std::string anim;
	int emoteModifier;
	int deskMod;

	std::string sound;
	int delay;
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
	std::vector<emoteInfo> charEmotes;

	std::deque<std::string> icSendQueue;

public:
	UIScreenCourt();
	~UIScreenCourt();

	void init();
	void updateInput();
	void update();
	void changeScreen(UISubScreen* newScreen);

	void sendIC(const std::string& msg);

	std::vector<charInfo>& getCharList() {return charList;}
	const std::vector<musicInfo>& getMusicList() {return musicList;}
	const std::vector<areaInfo>& getAreaList() {return areaList;}
	const std::vector<std::string>& getICLog() {return icLog;}
	const std::vector<std::string>& getOOCLog() {return oocLog;}
	int getCurrCharID() {return currChar;}
	charInfo& getCurrChar() {return charList[currChar];}
	const std::vector<emoteInfo>& getCharEmotes() {return charEmotes;}

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
	static void onMessageFA(void* pUserData, std::string msg);
	static void onMessageARUP(void* pUserData, std::string msg);
	static void onMessageHP(void* pUserData, std::string msg);
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

	int bars[2];

	struct ICControls
	{
		ICControls() : additive(0), preanim(0), immediate(0), flip(0), shake(0), flash(0), evidence(-1), side(0), color(0), pairID(-1), offset(0) {}
		int additive;
		int preanim;
		int immediate;
		int flip;
		int shake;
		int flash;

		int evidence;
		int side;
		int color;

		int pairID;
		int offset;
	} icControls;
};

#endif // UICOURT_H_INCLUDED
