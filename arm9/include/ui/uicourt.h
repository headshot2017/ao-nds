#ifndef UICOURT_H_INCLUDED
#define UICOURT_H_INCLUDED

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>

#include "uiscreen.h"
#include "courtroom/courtroom.h"
#include "settings.h"
#include "wav_nds.h"
#include "mem.h"

extern const char* indToSide[6];

struct charInfo
{
	std::string name;
	std::u16string showname;
	std::string blip;
	std::string side;
	bool taken;
	bool muted;
};

struct musicInfo
{
	std::string name;
	std::u16string nameDecoded;
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
	std::string preanim;
	std::string anim;
	std::string frameSFX;
	std::string frameShake;
	std::string frameFlash;
	int emoteModifier;
	int deskMod;

	std::string sound;
	int delay;
};

struct shoutInfo
{
	std::string filename;
	std::string displayname;
};

struct playerInfo
{
	std::string oocName;
	std::string character;
	std::string showname;
	int area;
};

class UIScreenCourt;
class UISubScreen
{
public:
	UISubScreen(UIScreenCourt* courtUI) : pCourtUI(courtUI), bgIndex(-1), bgTilesLen(0), bgPal(0) {}
	virtual ~UISubScreen();

	virtual void init() {}
	virtual void updateInput() {}
	virtual void update() {}

	void loadBg(std::string filename, bool deletePal=false);

protected:
	UIScreenCourt* pCourtUI;

	int bgIndex;
	u32 bgTilesLen;
	u8* bgPal;
};

class UIScreenCourt : public UIScreen
{
	Courtroom* court;
	UISubScreen* subScreen;
	UISubScreen* nextScreen;

	std::vector<charInfo> charList;
	std::vector<musicInfo> musicList;
	std::vector<areaInfo> areaList;
	std::vector<evidenceInfo> evidenceList;
	std::unordered_set<std::string> featureList;
	std::unordered_map<u32, playerInfo> playerList;
	std::vector<u32> fullPlayerListIDs;
	std::vector<u32> playerListIDs;

	std::vector<std::u16string> icLog;
	std::vector<std::u16string> oocLog;

	int currChar;
	std::vector<emoteInfo> charEmotes;
	std::vector<shoutInfo> charShouts;

	int sendTicks;
	std::deque<std::string> icSendQueue;

	int receiveTicks;
	std::deque<MSchatStruct> icReceiveQueue;
	std::deque<std::string> rtReceiveQueue;

	bool loggedIn;
	bool isConsole;
	bool firstConnect;
	u32 clientID;

	void onAreaChanged();
	void updatePlayer(u32 id);

public:
	UIScreenCourt();
	~UIScreenCourt();
	int ID() {return 0;}

	Courtroom* getCourtroom() {return court;}

	void init();
	void updateInput();
	void update();
	void changeScreen(UISubScreen* newScreen);

	void sendIC(const std::string& msg);

	std::vector<charInfo>& getCharList() {return charList;}
	const std::vector<musicInfo>& getMusicList() {return musicList;}
	const std::vector<areaInfo>& getAreaList() {return areaList;}
	std::vector<evidenceInfo>& getEvidenceList(bool priv) {return (priv) ? Settings::privateEvidence : evidenceList;}
	bool getFeature(const char* feature) {return featureList.count(feature);}
	std::unordered_map<u32, playerInfo>& getPlayerList() {return playerList;}
	const std::vector<u32>& getFullPlayerListIDs() {return fullPlayerListIDs;}
	const std::vector<u32>& getPlayerListIDs() {return loggedIn ? fullPlayerListIDs : playerListIDs;}
	u32 getClientID() {return clientID;}

	const std::vector<std::u16string>& getICLog() {return icLog;}
	const std::vector<std::u16string>& getOOCLog() {return oocLog;}
	int getCurrCharID() {return currChar;}
	charInfo& getCurrChar() {return charList[currChar];}
	const std::vector<emoteInfo>& getCharEmotes() {return charEmotes;}
	const std::vector<shoutInfo>& getCharShouts() {return charShouts;}
	bool isMod() {return loggedIn;}

	static void onMessageDecryptor(void* pUserData, std::string msg);
	static void onMessageID(void* pUserData, std::string msg);
	static void onMessagePN(void* pUserData, std::string msg);
	static void onMessageFL(void* pUserData, std::string msg);
	static void onMessageSI(void* pUserData, std::string msg);
	static void onMessageSC(void* pUserData, std::string msg);
	static void onMessageSM(void* pUserData, std::string msg);
	static void onMessageBN(void* pUserData, std::string msg);
	static void onMessageMC(void* pUserData, std::string msg);
	static void onMessageMS(void* pUserData, std::string msg);
	static void onMessageCT(void* pUserData, std::string msg);
	static void onMessageRT(void* pUserData, std::string msg);
	static void onMessageCharsCheck(void* pUserData, std::string msg);
	static void onMessagePV(void* pUserData, std::string msg);
	static void onMessageDONE(void* pUserData, std::string msg);
	static void onMessageFA(void* pUserData, std::string msg);
	static void onMessageARUP(void* pUserData, std::string msg);
	static void onMessageLE(void* pUserData, std::string msg);
	static void onMessageHP(void* pUserData, std::string msg);
	static void onMessageKK(void* pUserData, std::string msg);
	static void onMessageKB(void* pUserData, std::string msg);
	static void onMessageBD(void* pUserData, std::string msg);
	static void onMessageBB(void* pUserData, std::string msg);
	static void onMessageAUTH(void* pUserData, std::string msg);
	static void onMessageZZ(void* pUserData, std::string msg);
	static void onMessagePR(void* pUserData, std::string msg);
	static void onMessagePU(void* pUserData, std::string msg);

	wav_handle* sndSelect;
	wav_handle* sndCancel;
	wav_handle* sndEvTap;
	wav_handle* sndEvPage;
	wav_handle* sndEvShow;
	wav_handle* sndCrtRcrd;

	std::u16string showname;
	std::u16string oocName;

	bool guard;
	int bars[2];

	struct ICControls
	{
		ICControls() : additive(0), preanim(0), immediate(0), flip(0), shake(0), flash(0), slide(0), evidence(-1), side(0), color(0), pairID(-1), xOffset(0), yOffset(0) {}
		int additive;
		int preanim;
		int immediate;
		int flip;
		int shake;
		int flash;
		int slide;

		int evidence;
		int side;
		int color;

		int pairID;
		int xOffset;
		int yOffset;
	} icControls;
};

#endif // UICOURT_H_INCLUDED
