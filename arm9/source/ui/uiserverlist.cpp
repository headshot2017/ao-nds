#include "ui/uiserverlist.h"

#include <math.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <nds/arm9/input.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "mini/ini.h"

#include "engine.h"
#include "ui/uimainmenu.h"
#include "ui/uicourt.h"
#include "sockets/aowebsocket.h"
#include "sockets/aotcpsocket.h"

struct serverBtnData
{
	UIScreenServerList* pObj;
	int btnInd;
};

UIScreenServerList::~UIScreenServerList()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	dmaFillHalfWords(0, bgGetGfxPtr(subBgIndex), bgSubTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(subBgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	oamClearSprite(&oamMain, 0);
	oamClearSprite(&oamMain, 1);
	oamFreeGfx(&oamMain, spr_arrowDownGfx);

	delete btn_manageFav;
	delete btn_listToggle;
	delete btn_back;
	delete btn_connect;
	delete btn_prevPage;
	delete btn_nextPage;
	for (int i=0; i<4; i++)
	{
		delete btn_server[i];
		delete lbl_server[i];
	}
	delete lbl_pages;
	delete lbl_plswait;
	delete lbl_desc;
	delete lbl_players;
	delete lbl_playercount;

	wav_free_handle(sndSelect);
	wav_free_handle(sndCancel);

	shutdown(sockfd, 0);
	closesocket(sockfd);
}

void UIScreenServerList::init()
{
	u8* bgTiles = readFile("/data/ao-nds/ui/bg_serverDesc.img.bin", &bgTilesLen);
	u8* bgMap = readFile("/data/ao-nds/ui/bg_serverDesc.map.bin");
	u8* bgPal = readFile("/data/ao-nds/ui/bg_serverDesc.pal.bin");
	u8* bgSubTiles = readFile("/data/ao-nds/ui/bg_serverList.img.bin", &bgSubTilesLen);
	u8* bgSubMap = readFile("/data/ao-nds/ui/bg_serverList.map.bin");
	u8* bgSubPal = readFile("/data/ao-nds/ui/bg_serverList.pal.bin");

	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	subBgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(subBgIndex, 1);

	dmaCopy(bgTiles, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), 1536);
	memcpy(BG_PALETTE, bgPal, 512);

	dmaCopy(bgSubTiles, bgGetGfxPtr(subBgIndex), bgSubTilesLen);
	dmaCopy(bgSubMap, bgGetMapPtr(subBgIndex), 1536);
	memcpy(BG_PALETTE_SUB, bgSubPal, 512);

	delete[] bgTiles;
	delete[] bgMap;
	delete[] bgPal;
	delete[] bgSubTiles;
	delete[] bgSubMap;
	delete[] bgSubPal;

	u8* spr_arrowDownTiles = readFile("/data/ao-nds/ui/spr_arrowDown.img.bin");
	u8* spr_arrowDownPal = readFile("/data/ao-nds/ui/spr_arrowDown.pal.bin");

	vramSetBankF(VRAM_F_LCD);
	spr_arrowDownGfx = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
	dmaCopy(spr_arrowDownTiles, spr_arrowDownGfx, 16*16);
	memcpy(&VRAM_F_EXT_SPR_PALETTE[0], spr_arrowDownPal, 512);
	vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

	delete[] spr_arrowDownTiles;
	delete[] spr_arrowDownPal;

	oamSet(&oamMain, 0, 32, 192-20, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, spr_arrowDownGfx, -1, false, false, false, false, false);
	oamSet(&oamMain, 1, 256-16-32, 192-20, 0, 0, SpriteSize_16x16, SpriteColorFormat_256Color, spr_arrowDownGfx, -1, false, false, false, false, false);

	lbl_desc = new UILabel(&oamMain, 2, 6, 10, RGB15(31,31,31), 1, 0);
	lbl_players = new UILabel(&oamMain, lbl_desc->nextOamInd(), 2, 1, RGB15(8,8,8), 2, 0);
	lbl_playercount = new UILabel(&oamMain, lbl_players->nextOamInd(), 1, 1, RGB15(8,8,8), 2, 0);
	lbl_desc->setPos(32, 32);
	lbl_players->setText("Players:");
	lbl_players->setPos(190, 9);
	lbl_players->setVisible(false);
	lbl_playercount->setVisible(false);

	btn_manageFav = new UIButton(&oamSub, "/data/ao-nds/ui/spr_addFav", 2, 3, 1, SpriteSize_32x64, 0, 0, 80, 33, 32, 64, 1);
	btn_listToggle = new UIButton(&oamSub, "/data/ao-nds/ui/spr_favoritesPublic", btn_manageFav->nextOamInd(), 3, 1, SpriteSize_32x64, 256-80, 0, 80, 33, 32, 64, 2);
	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", btn_listToggle->nextOamInd(), 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 3);
	btn_connect = new UIButton(&oamSub, "/data/ao-nds/ui/spr_connect", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-79, 192-30, 79, 30, 32, 32, 4);
	btn_prevPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft", btn_connect->nextOamInd(), 1, 1, SpriteSize_32x16, 79+2, 192-15, 19, 14, 32, 16, 5);
	btn_nextPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight", btn_prevPage->nextOamInd(), 1, 1, SpriteSize_32x16, 256-79-19-2, 192-15, 19, 14, 32, 16, 6);
	for (int i=0; i<4; i++)
	{
		int nextOam = (i == 0) ? btn_nextPage->nextOamInd() : lbl_server[i-1]->nextOamInd();
		btn_server[i] = new UIButton(&oamSub, "/data/ao-nds/ui/spr_serverBtn", nextOam, 7, 1, SpriteSize_32x32, 128-112, 36+(i*32), 224, 26, 32, 32, 7+i);
		lbl_server[i] = new UILabel(&oamSub, btn_server[i]->nextOamInd(), 8, 1, RGB15(13, 2, 0), 11, 0);
		btn_server[i]->setPriority(1);
		btn_server[i]->setVisible(false);
	}
	lbl_pages = new UILabel(&oamSub, lbl_server[3]->nextOamInd(), 1, 1, RGB15(13, 2, 0), 12, 0);
	lbl_plswait = new UILabel(&oamSub, lbl_pages->nextOamInd(), 8, 2, RGB15(31,31,31), 13, 0);
	publicListMsg = "Getting server list...";
	lbl_plswait->setText(publicListMsg.c_str());
	lbl_plswait->setPos(128, 96-6, true);
	btn_prevPage->setVisible(false);
	btn_nextPage->setVisible(false);
	btn_manageFav->setVisible(false);

	btn_manageFav->assignKey(KEY_L);
	btn_listToggle->assignKey(KEY_R);
	btn_back->assignKey(KEY_B);
	btn_connect->assignKey(KEY_A);
	btn_prevPage->assignKey(KEY_LEFT);
	btn_nextPage->assignKey(KEY_RIGHT);

	static serverBtnData btnData[4];
	btn_manageFav->connect(onManageFavorite, this);
	btn_listToggle->connect(onToggleList, this);
	btn_back->connect(onBack, this);
	btn_connect->connect(onConnect, this);
	btn_prevPage->connect(onPrevPage, this);
	btn_nextPage->connect(onNextPage, this);
	for (int i=0; i<4; i++)
	{
		btnData[i].btnInd = i;
		btnData[i].pObj = this;
		btn_server[i]->connect(onServerClicked, &btnData[i]);
	}

	sndSelect = wav_load_handle("/data/ao-nds/sounds/general/sfx-selectblip2.wav");
	sndCancel = wav_load_handle("/data/ao-nds/sounds/general/sfx-cancel.wav");

	arrowY = 0;
	arrowYadd = 1;
	currPage = 0;
	currServer = -1;
	isFavorites = 0;
	loaded = false;

	for (int i=0; i<2; i++)
		m_servers.push_back(std::vector<serverInfo>());

	parseFavoritesList();

	// store the HTTP request for later
	const char * request_text =
		"GET /servers HTTP/1.1\r\n"
		"Host: servers.aceattorneyonline.com\r\n"
		"User-Agent: Nintendo DS\r\n\r\n";

	// Find the IP address of the server, with gethostbyname
	struct hostent* myhost = gethostbyname("servers.aceattorneyonline.com");
	// Create a TCP socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// Tell the socket to connect to the IP address we found, on port 80 (HTTP)
	struct sockaddr_in sain;
	sain.sin_family = AF_INET;
	sain.sin_port = htons(80);
	sain.sin_addr.s_addr= *( (unsigned long *)(myhost->h_addr_list[0]) );
	connect(sockfd, (struct sockaddr *)&sain, sizeof(sain));

	// set non-blocking
	u32 one = 1;
	ioctl(sockfd, FIONBIO, &one);

	// send our request
	send(sockfd, request_text, strlen(request_text), 0);
}

void UIScreenServerList::updateInput()
{
	btn_manageFav->updateInput();
	btn_listToggle->updateInput();
	btn_back->updateInput();
	btn_connect->updateInput();
	btn_prevPage->updateInput();
	btn_nextPage->updateInput();
	for (int i=0; i<4; i++) btn_server[i]->updateInput();
}

void UIScreenServerList::update()
{
	char incoming_buffer[256] = {0};
	int recvd_len = recv(sockfd, incoming_buffer, 255, 0);
	if (recvd_len)
	{
		tempData += incoming_buffer;
		if (incoming_buffer[recvd_len-2] == '}' && incoming_buffer[recvd_len-1] == ']') // got json dats
		{
			parsePublicList(tempData.substr(tempData.find("\r\n\r\n")));
			tempData.clear();
		}
	}

	arrowY += arrowYadd;
	if (arrowY <= 0 || arrowY >= 8)
		arrowYadd = -arrowYadd;

	oamSetXY(&oamMain, 0, 32, 192-20+(arrowY/2));
	oamSetXY(&oamMain, 1, 256-16-32, 192-20+(arrowY/2));

	btn_connect->setVisible(currServer != -1);
}

void UIScreenServerList::reloadPage()
{
	if (currServer != -1)
	{
		btn_server[currServer]->setFrame(0);
		btn_manageFav->setVisible(false);
		currServer = -1;
	}

	for (u32 i=0; i<4; i++)
	{
		u32 ind = currPage*4 + i;
		if (ind >= m_servers[isFavorites].size())
		{
			btn_server[i]->setVisible(false);
			lbl_server[i]->setVisible(false);
			continue;
		}

		btn_server[i]->setVisible(true);
		lbl_server[i]->setVisible(true);
		lbl_server[i]->setText(m_servers[isFavorites][ind].name.c_str());
		lbl_server[i]->setPos(128, 42+(i*32), true);
	}

	u32 maxPages = (u32)ceil(m_servers[isFavorites].size()/4.f);
	btn_prevPage->setVisible(currPage > 0);
	btn_nextPage->setVisible(m_servers[isFavorites].size() && currPage < maxPages-1);

	lbl_desc->setVisible(false);
	lbl_players->setVisible(false);
	lbl_playercount->setVisible(false);

	if (!m_servers[isFavorites].empty())
	{
		char buf[8];
		sprintf(buf, "%lu/%lu", currPage+1, maxPages);
		lbl_pages->setVisible(true);
		lbl_pages->setText(buf);
		lbl_pages->setPos(128, 192-15, true);
		lbl_plswait->setVisible(false);
	}
	else
	{
		lbl_pages->setVisible(false);
		lbl_plswait->setVisible(true);
		lbl_plswait->setText(isFavorites ? "Favorites list is empty" : publicListMsg.c_str());
		lbl_plswait->setPos(128, 96-6, publicListMsg.find("JSON parse error") == std::string::npos);
	}
}

void UIScreenServerList::saveFavorites()
{
	mINI::INIFile file("/data/ao-nds/favorite_servers.ini");
	mINI::INIStructure ini;

	for (u32 i=0; i<m_servers[1].size(); i++)
	{
		ini[std::to_string(i)]["name"] = m_servers[1][i].name;
		ini[std::to_string(i)]["desc"] = m_servers[1][i].description;
		ini[std::to_string(i)]["address"] = m_servers[1][i].ip;
		ini[std::to_string(i)]["port"] = std::to_string((m_servers[1][i].ws_port) ? m_servers[1][i].ws_port : m_servers[1][i].port);
		ini[std::to_string(i)]["protocol"] = (m_servers[1][i].ws_port) ? "ws" : "tcp";
	}

	file.generate(ini);
}

void UIScreenServerList::onManageFavorite(void* pUserData)
{
	UIScreenServerList* pSelf = (UIScreenServerList*)pUserData;
	wav_play(pSelf->sndSelect);

	u32 selectedInd = pSelf->currPage*4 + pSelf->currServer;

	if (!pSelf->isFavorites)
	{
		// add favorite
		pSelf->m_servers[1].push_back(pSelf->m_servers[0][selectedInd]);
		pSelf->btn_manageFav->setVisible(false);
	}
	else
	{
		// remove favorite
		pSelf->m_servers[1].erase(pSelf->m_servers[1].begin() + selectedInd);
	}

	pSelf->saveFavorites();
	if (pSelf->isFavorites) pSelf->reloadPage();
}

void UIScreenServerList::onToggleList(void* pUserData)
{
	UIScreenServerList* pSelf = (UIScreenServerList*)pUserData;
	wav_play(pSelf->sndSelect);

	pSelf->isFavorites = -pSelf->isFavorites+1;
	if (pSelf->isFavorites)
	{
		pSelf->btn_manageFav->setImage("/data/ao-nds/ui/spr_delete", 32, 64, 1);
		pSelf->btn_listToggle->setFrame(1);
	}
	else
	{
		pSelf->btn_manageFav->setImage("/data/ao-nds/ui/spr_addFav", 32, 64, 1);
		pSelf->btn_listToggle->setFrame(0);
	}

	pSelf->currPage = 0;
	pSelf->reloadPage();
}

void UIScreenServerList::onBack(void* pUserData)
{
	UIScreenServerList* pSelf = (UIScreenServerList*)pUserData;
	wav_play(pSelf->sndCancel);

	gEngine->changeScreen(new UIScreenMainMenu);
}

void UIScreenServerList::onConnect(void* pUserData)
{
	UIScreenServerList* pSelf = (UIScreenServerList*)pUserData;
	wav_play(pSelf->sndSelect);

	const serverInfo& server = pSelf->m_servers[pSelf->isFavorites][pSelf->currPage*4 + pSelf->currServer];
	if (server.ws_port)
	{
		AOwebSocket* sock = new AOwebSocket;
		char ip[256];
		sprintf(ip, "ws://%s:%d", server.ip.c_str(), server.ws_port);

		sock->connectIP(ip);
		gEngine->setSocket(sock);
	}
	else
	{
		AOtcpSocket* sock = new AOtcpSocket;
		sock->connectIP(server.ip, server.port);
		gEngine->setSocket(sock);
	}

	gEngine->changeScreen(new UIScreenCourt);
}

void UIScreenServerList::onPrevPage(void* pUserData)
{
	UIScreenServerList* pSelf = (UIScreenServerList*)pUserData;
	wav_play(pSelf->sndSelect);

	if (!pSelf->loaded && !pSelf->isFavorites) return;

	pSelf->currPage--;
	pSelf->reloadPage();
}

void UIScreenServerList::onNextPage(void* pUserData)
{
	UIScreenServerList* pSelf = (UIScreenServerList*)pUserData;
	wav_play(pSelf->sndSelect);

	if (!pSelf->loaded && !pSelf->isFavorites) return;

	pSelf->currPage++;
	pSelf->reloadPage();
}

void UIScreenServerList::onServerClicked(void* pUserData)
{
	serverBtnData* pData = (serverBtnData*)pUserData;
	UIScreenServerList* pSelf = pData->pObj;

	if (pSelf->currServer == pData->btnInd) // already selected
		return;

	wav_play(pSelf->sndSelect);

	// unselect server
	if (pSelf->currServer != -1)
		pSelf->btn_server[pSelf->currServer]->setFrame(0);

	// select server
	pSelf->currServer = pData->btnInd;
	pSelf->btn_server[pSelf->currServer]->setFrame(1);

	const serverInfo& server = pSelf->m_servers[pSelf->isFavorites][pSelf->currServer + pSelf->currPage*4];
	if (!pSelf->isFavorites)
	{
		// add fav img
		pSelf->btn_manageFav->setVisible(true);
		for (u32 i=0; i<pSelf->m_servers[1].size(); i++)
		{
			const serverInfo& favServer = pSelf->m_servers[1][i];
			if (favServer.ip == server.ip && (favServer.port == server.port || favServer.ws_port == server.ws_port))
			{
				// favorite server already exists
				pSelf->btn_manageFav->setVisible(false);
				break;
			}
		}
	}
	else
	{
		// delete favorite img
		pSelf->btn_manageFav->setVisible(true);
	}

	pSelf->lbl_desc->setVisible(true);
	pSelf->lbl_desc->setText(server.description.c_str());

	if (!pSelf->isFavorites)
	{
		pSelf->lbl_players->setVisible(true);
		pSelf->lbl_playercount->setVisible(true);

		char buf[8];
		sprintf(buf, "%lu", server.players);
		pSelf->lbl_playercount->setText(buf);
		pSelf->lbl_playercount->setPos(212, 20, true);
	}
}

void UIScreenServerList::parsePublicList(const std::string& data)
{
	rapidjson::Document doc;
	rapidjson::ParseResult ok = doc.Parse(data.c_str());
	if (!ok)
	{
		publicListMsg = "JSON parse error at offset " + std::to_string(ok.Offset()) + ":\n" + rapidjson::GetParseError_En(ok.Code());
		if (!isFavorites)
		{
			lbl_plswait->setText(publicListMsg.c_str());
			lbl_plswait->setPos(0, 96-6, false);
		}
		return;
	}

	m_servers[0].clear();
	u32 serverCount = doc.Size();

	for (u32 i=0; i<serverCount; i++)
	{
		const rapidjson::Value& server = doc[i];

		serverInfo info;
		info.name = server["name"].GetString();
		info.description = server["description"].GetString();
		info.ip = server["ip"].GetString();
		info.players = server["players"].GetUint();
		info.port = server["port"].GetUint();
		info.ws_port = (server.HasMember("ws_port")) ? server["ws_port"].GetUint() : 0;

		m_servers[0].push_back(info);
	}

	loaded = true;
	if (!isFavorites)
	{
		currPage = 0;
		reloadPage();
	}
}

void UIScreenServerList::parseFavoritesList()
{
	mINI::INIFile file("/data/ao-nds/favorite_servers.ini");
	mINI::INIStructure ini;

	if (!file.read(ini)) return;

	m_servers[1].clear();
	u32 serverCount = ini.size();

	for (u32 i=0; i<serverCount; i++)
	{
		serverInfo info;
		info.name = ini[std::to_string(i)]["name"];
		info.description = ini[std::to_string(i)]["desc"];
		info.ip = ini[std::to_string(i)]["address"];
		info.players = 0;
		if (ini[std::to_string(i)]["protocol"] == "tcp")
		{
			info.port = std::stoi(ini[std::to_string(i)]["port"]);
			info.ws_port = 0;
		}
		else
		{
			info.port = 0;
			info.ws_port = std::stoi(ini[std::to_string(i)]["port"]);
		}

		m_servers[1].push_back(info);
	}
}
