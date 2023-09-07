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
#include <nds/arm9/sound.h>
#include "rapidjson/document.h"
#include "mini/ini.h"

#include "mp3_shared.h"
#include "engine.h"
#include "ui/uimainmenu.h"
#include "ui/uicourt.h"
#include "sockets/aowebsocket.h"
#include "sockets/aotcpsocket.h"
#include "bg_serverList.h"
#include "bg_serverDesc.h"
#include "spr_arrowDown.h"
#include "spr_addFav.h"
#include "spr_delete.h"
#include "spr_favorites.h"
#include "spr_public.h"
#include "spr_back.h"
#include "spr_connect.h"
#include "spr_pageLeft.h"
#include "spr_pageRight.h"
#include "spr_serverUnselected.h"
#include "spr_serverSelected.h"

struct serverBtnData
{
	UIScreenServerList* pObj;
	int btnInd;
};

UIScreenServerList::~UIScreenServerList()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bg_serverDescTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), bg_serverDescMapLen);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	dmaFillHalfWords(0, bgGetGfxPtr(subBgIndex), bg_serverListTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(subBgIndex), bg_serverListMapLen);
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

	delete[] sndSelect;
	delete[] sndCancel;

	for (int i=0; i<2; i++)
		if (m_servers[i]) delete[] m_servers[i];

	shutdown(sockfd, 0); // good practice to shutdown the socket.
	closesocket(sockfd); // remove the socket.
}

void UIScreenServerList::init()
{
	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	subBgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(subBgIndex, 1);

	dmaCopy(bg_serverDescTiles, bgGetGfxPtr(bgIndex), bg_serverDescTilesLen);
	dmaCopy(bg_serverDescMap, bgGetMapPtr(bgIndex), bg_serverDescMapLen);
	dmaCopy(bg_serverDescPal, BG_PALETTE, bg_serverDescPalLen);

	dmaCopy(bg_serverListTiles, bgGetGfxPtr(subBgIndex), bg_serverListTilesLen);
	dmaCopy(bg_serverListMap, bgGetMapPtr(subBgIndex), bg_serverListMapLen);
	dmaCopy(bg_serverListPal, BG_PALETTE_SUB, bg_serverListPalLen);

	vramSetBankF(VRAM_F_LCD);
	spr_arrowDownGfx = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
	dmaCopy(spr_arrowDownTiles, spr_arrowDownGfx, spr_arrowDownTilesLen);
	dmaCopy(spr_arrowDownPal, &VRAM_F_EXT_SPR_PALETTE[0], spr_arrowDownPalLen);
	vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

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

	btn_manageFav = new UIButton(&oamSub, (u8*)spr_addFavTiles, (u8*)spr_addFavPal, 2, 3, SpriteSize_32x64, 0, 0, 80, 33, 32, 64, 1);
	btn_listToggle = new UIButton(&oamSub, (u8*)spr_favoritesTiles, (u8*)spr_favoritesPal, btn_manageFav->nextOamInd(), 3, SpriteSize_32x64, 256-80, 0, 80, 33, 32, 64, 2);
	btn_back = new UIButton(&oamSub, (u8*)spr_backTiles, (u8*)spr_backPal, btn_listToggle->nextOamInd(), 3, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 3);
	btn_connect = new UIButton(&oamSub, (u8*)spr_connectTiles, (u8*)spr_connectPal, btn_back->nextOamInd(), 3, SpriteSize_32x32, 256-79, 192-30, 79, 30, 32, 32, 4);
	btn_prevPage = new UIButton(&oamSub, (u8*)spr_pageLeftTiles, (u8*)spr_pageLeftPal, btn_connect->nextOamInd(), 1, SpriteSize_32x16, 79+2, 192-15, 19, 14, 32, 16, 5);
	btn_nextPage = new UIButton(&oamSub, (u8*)spr_pageRightTiles, (u8*)spr_pageRightPal, btn_prevPage->nextOamInd(), 1, SpriteSize_32x16, 256-79-19-2, 192-15, 19, 14, 32, 16, 6);
	for (int i=0; i<4; i++)
	{
		int nextOam = (i == 0) ? btn_nextPage->nextOamInd() : lbl_server[i-1]->nextOamInd();
		btn_server[i] = new UIButton(&oamSub, (u8*)spr_serverUnselectedTiles, (u8*)spr_serverUnselectedPal, nextOam, 7, SpriteSize_32x32, 128-112, 36+(i*32), 224, 26, 32, 32, 7+i);
		lbl_server[i] = new UILabel(&oamSub, btn_server[i]->nextOamInd(), 8, 1, RGB15(13, 2, 0), 11, 0);
		btn_server[i]->setPriority(1);
		btn_server[i]->setVisible(false);
	}
	lbl_pages = new UILabel(&oamSub, lbl_server[3]->nextOamInd(), 1, 1, RGB15(13, 2, 0), 12, 0);
	lbl_plswait = new UILabel(&oamSub, lbl_pages->nextOamInd(), 8, 1, RGB15(31,31,31), 13, 0);
	publicListMsg = "Getting server list...";
	lbl_plswait->setText(publicListMsg.c_str());
	lbl_plswait->setPos(128, 96-6, true);
	btn_prevPage->setVisible(false);
	btn_nextPage->setVisible(false);
	btn_manageFav->setVisible(false);

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

	sndSelect = wav_load_handle("/data/ao-nds/sounds/general/sfx-selectblip2.wav", &sndSelectSize);
	sndCancel = wav_load_handle("/data/ao-nds/sounds/general/sfx-cancel.wav", &sndCancelSize);

	arrowY = 0;
	arrowYadd = 1;
	currPage = 0;
	currServer = -1;
	isFavorites = 0;
	loaded = false;

	for (int i=0; i<2; i++)
	{
		m_servers[i] = 0;
		m_serverCount[i] = 0;
	}

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
	char incoming_buffer[256];
	int recvd_len = recv(sockfd, incoming_buffer, 255, 0);
	if (recvd_len)
	{
		incoming_buffer[recvd_len] = 0; // null-terminate
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
		btn_server[currServer]->setImage((u8*)spr_serverUnselectedTiles, (u8*)spr_serverUnselectedPal, 32, 32, 7+currServer);
		btn_manageFav->setVisible(false);
		currServer = -1;
	}

	for (u32 i=0; i<4; i++)
	{
		u32 ind = currPage*4 + i;
		if (ind >= m_serverCount[isFavorites])
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

	u32 maxPages = (u32)ceil(m_serverCount[isFavorites]/4.f);
	btn_prevPage->setVisible(currPage > 0);
	btn_nextPage->setVisible(m_serverCount[isFavorites] && currPage < maxPages-1);

	lbl_desc->setVisible(false);
	lbl_players->setVisible(false);
	lbl_playercount->setVisible(false);

	if (m_serverCount[isFavorites])
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
		lbl_plswait->setPos(128, 96-6, true);
	}
}

void UIScreenServerList::saveFavorites()
{
	mINI::INIFile file("/data/ao-nds/favorite_servers.ini");
	mINI::INIStructure ini;

	for (u32 i=0; i<m_serverCount[1]; i++)
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
	soundPlaySample(pSelf->sndSelect, SoundFormat_16Bit, pSelf->sndSelectSize, 32000, 127, 64, false, 0);

	serverInfo* newList;

	if (!pSelf->isFavorites)
	{
		// add favorite
		newList = new serverInfo[pSelf->m_serverCount[1]+1];

		for (u32 i=0; i<pSelf->m_serverCount[1]; i++)
			newList[i] = pSelf->m_servers[1][i];

		// assign selected public server to new favorites server slot and increment favorite servers count
		newList[pSelf->m_serverCount[1]++] = pSelf->m_servers[0][pSelf->currPage*4 + pSelf->currServer];

		pSelf->btn_manageFav->setVisible(false);
	}
	else
	{
		// remove favorite
		newList = new serverInfo[pSelf->m_serverCount[1]-1];
		u32 deleteInd = pSelf->currPage*4 + pSelf->currServer;

		for (u32 i=0, addInd=0; i<pSelf->m_serverCount[1]; i++)
		{
			if (i == deleteInd) continue;
			newList[addInd++] = pSelf->m_servers[1][i];
		}

		pSelf->m_serverCount[1]--;
	}

	if (pSelf->m_servers[1]) delete[] pSelf->m_servers[1];
	pSelf->m_servers[1] = newList;
	pSelf->saveFavorites();
	if (pSelf->isFavorites) pSelf->reloadPage();
}

void UIScreenServerList::onToggleList(void* pUserData)
{
	UIScreenServerList* pSelf = (UIScreenServerList*)pUserData;
	soundPlaySample(pSelf->sndSelect, SoundFormat_16Bit, pSelf->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->isFavorites = -pSelf->isFavorites+1;
	if (pSelf->isFavorites)
	{
		pSelf->btn_manageFav->setImage((u8*)spr_deleteTiles, (u8*)spr_deletePal, 32, 64, 1);
		pSelf->btn_listToggle->setImage((u8*)spr_publicTiles, (u8*)spr_publicPal, 32, 64, 2);
	}
	else
	{
		pSelf->btn_manageFav->setImage((u8*)spr_addFavTiles, (u8*)spr_addFavPal, 32, 64, 1);
		pSelf->btn_listToggle->setImage((u8*)spr_favoritesTiles, (u8*)spr_favoritesPal, 32, 64, 2);
	}

	pSelf->currPage = 0;
	pSelf->reloadPage();
}

void UIScreenServerList::onBack(void* pUserData)
{
	UIScreenServerList* pSelf = (UIScreenServerList*)pUserData;
	soundPlaySample(pSelf->sndCancel, SoundFormat_16Bit, pSelf->sndCancelSize, 32000, 127, 64, false, 0);

	gEngine->changeScreen(new UIScreenMainMenu);
}

void UIScreenServerList::onConnect(void* pUserData)
{
	UIScreenServerList* pSelf = (UIScreenServerList*)pUserData;
	soundPlaySample(pSelf->sndSelect, SoundFormat_16Bit, pSelf->sndSelectSize, 32000, 127, 64, false, 0);

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
	soundPlaySample(pSelf->sndSelect, SoundFormat_16Bit, pSelf->sndSelectSize, 32000, 127, 64, false, 0);

	if (!pSelf->loaded && !pSelf->isFavorites) return;

	pSelf->currPage--;
	pSelf->reloadPage();
}

void UIScreenServerList::onNextPage(void* pUserData)
{
	UIScreenServerList* pSelf = (UIScreenServerList*)pUserData;
	soundPlaySample(pSelf->sndSelect, SoundFormat_16Bit, pSelf->sndSelectSize, 32000, 127, 64, false, 0);

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

	soundPlaySample(pSelf->sndSelect, SoundFormat_16Bit, pSelf->sndSelectSize, 32000, 127, 64, false, 0);

	// unselect server
	if (pSelf->currServer != -1)
		pSelf->btn_server[pSelf->currServer]->setImage((u8*)spr_serverUnselectedTiles, (u8*)spr_serverUnselectedPal, 32, 32, 7+pSelf->currServer);

	// select server
	pSelf->currServer = pData->btnInd;
	pSelf->btn_server[pSelf->currServer]->setImage((u8*)spr_serverSelectedTiles, (u8*)spr_serverSelectedPal, 32, 32, 7+pSelf->currServer);

	const serverInfo& server = pSelf->m_servers[pSelf->isFavorites][pSelf->currServer + pSelf->currPage*4];
	if (!pSelf->isFavorites)
	{
		// add fav img
		pSelf->btn_manageFav->setVisible(true);
		for (u32 i=0; i<pSelf->m_serverCount[1]; i++)
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
	if (doc.Parse(data.c_str()).HasParseError())
	{
		publicListMsg = "JSON parse error";
		if (!isFavorites)
		{
			lbl_plswait->setText(publicListMsg.c_str());
			lbl_plswait->setPos(128, 96-6, true);
		}
		return;
	}

	if (m_servers[0]) delete[] m_servers[0];
	m_serverCount[0] = doc.Size();
	m_servers[0] = new serverInfo[m_serverCount[0]];

	for (u32 i=0; i<m_serverCount[0]; i++)
	{
		const rapidjson::Value& server = doc[i];
		m_servers[0][i].name = server["name"].GetString();
		m_servers[0][i].description = server["description"].GetString();
		m_servers[0][i].ip = server["ip"].GetString();
		m_servers[0][i].players = server["players"].GetUint();
		m_servers[0][i].port = server["port"].GetUint();
		m_servers[0][i].ws_port = (server.HasMember("ws_port")) ? server["ws_port"].GetUint() : 0;
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

	m_serverCount[1] = ini.size();
	if (!m_serverCount[1]) return;
	m_servers[1] = new serverInfo[m_serverCount[1]];

	for (u32 i=0; i<m_serverCount[1]; i++)
	{
		m_servers[1][i].name = ini[std::to_string(i)]["name"];
		m_servers[1][i].description = ini[std::to_string(i)]["desc"];
		m_servers[1][i].ip = ini[std::to_string(i)]["address"];
		m_servers[1][i].players = 0;
		if (ini[std::to_string(i)]["protocol"] == "tcp")
		{
			m_servers[1][i].port = std::stoi(ini[std::to_string(i)]["port"]);
			m_servers[1][i].ws_port = 0;
		}
		else
		{
			m_servers[1][i].port = 0;
			m_servers[1][i].ws_port = std::stoi(ini[std::to_string(i)]["port"]);
		}
	}
}
