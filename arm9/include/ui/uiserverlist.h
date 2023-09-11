#ifndef UISERVERLIST_H_INCLUDED
#define UISERVERLIST_H_INCLUDED

#include <string>

#include <nds/ndstypes.h>

#include "uiscreen.h"
#include "button.h"
#include "label.h"

struct serverInfo
{
	std::string ip;
	u16 port;
	u16 ws_port;
	u32 players;
	std::string name;
	std::string description;
};

class UIScreenServerList : public UIScreen
{
	int bgIndex;
	int subBgIndex;

	u16* spr_arrowDownGfx;
	UIButton* btn_manageFav;
	UIButton* btn_listToggle;
	UIButton* btn_back;
	UIButton* btn_connect;
	UIButton* btn_prevPage;
	UIButton* btn_nextPage;
	UIButton* btn_server[4];
	UILabel* lbl_server[4];
	UILabel* lbl_pages;
	UILabel* lbl_plswait;
	UILabel* lbl_desc;
	UILabel* lbl_players;
	UILabel* lbl_playercount;

	u32* sndSelect;
	u32 sndSelectSize;
	u32* sndCancel;
	u32 sndCancelSize;

	int sockfd;
	std::string tempData;
	serverInfo* m_servers[2];
	u32 m_serverCount[2];
	std::string publicListMsg;

	int arrowY;
	int arrowYadd;
	int currServer;
	u32 currPage;
	int isFavorites;
	bool loaded;

public:
	UIScreenServerList() : UIScreen() {}
	~UIScreenServerList();

	void init();
	void updateInput();
	void update();
	void reloadPage();
	void saveFavorites();

	static void onManageFavorite(void* pUserData);
	static void onToggleList(void* pUserData);
	static void onBack(void* pUserData);
	static void onConnect(void* pUserData);
	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onServerClicked(void* pUserData);

	void parsePublicList(const std::string& data);
	void parseFavoritesList();
};

#endif // UISERVERLIST_H_INCLUDED