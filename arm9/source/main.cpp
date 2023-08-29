#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <vector>

#include <nds/arm9/console.h>
#include <nds/arm9/exceptions.h>
#include <nds/arm9/video.h>
#include <nds/arm9/input.h>
#include <nds/arm9/sprite.h>
#include <nds/interrupts.h>
#include <fat.h>
#include <dswifi9.h>

#include "mp3_shared.h"
#include "global.h"
#include "courtroom/chatbox.h"
#include "courtroom/courtroom.h"
#include "fonts.h"
#include "bg_disclaimer.h"
#include "engine.h"
#include "sockets/aowebsocket.h"
#include "sockets/aotcpsocket.h"
#include "ui/uicourt.h"

#include "NDS12_ttf.h"
#include "acename_ttf.h"
#include "Igiari_ttf.h"

Courtroom* court;

void connect_wifi()
{
	struct in_addr ip, gateway, mask, dns1, dns2;

	iprintf("Connecting via WFC data...\n");
	Wifi_InitDefault(INIT_ONLY);
	unsigned char mac[6] = {0};
	Wifi_GetData(WIFIGETDATA_MACADDRESS, 6, mac);
	char macStr[32] = {0};
	for (int i=0; i<6; i++)
	{
		char buf[4];
		sprintf(buf, "%x", mac[i]);
		strcat(macStr, buf);
	}

	Wifi_AutoConnect();
	int wifiStatus = ASSOCSTATUS_DISCONNECTED;
	while (wifiStatus != ASSOCSTATUS_ASSOCIATED && wifiStatus != ASSOCSTATUS_CANNOTCONNECT)
	{
		wifiStatus = Wifi_AssocStatus();
		swiWaitForVBlank();
	}

	if(wifiStatus == ASSOCSTATUS_CANNOTCONNECT) {
		iprintf("Failed to connect! Please check your WiFi settings\n");
		//while (1) swiWaitForVBlank();
	} else {

		iprintf("Connected\n\n");

		ip = Wifi_GetIPInfo(&gateway, &mask, &dns1, &dns2);


		iprintf("ip     : %s\n", inet_ntoa(ip) );
		iprintf("gateway: %s\n", inet_ntoa(gateway) );
		iprintf("mask   : %s\n", inet_ntoa(mask) );
		iprintf("dns1   : %s\n", inet_ntoa(dns1) );
		iprintf("dns2   : %s\n", inet_ntoa(dns2) );
		iprintf("mac: '%s'\n", macStr);
	}
}

// Print HTTP response and signal that we're done
/*
static void handleServerlist(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  static const char *s_url = "http://servers.aceattorneyonline.com/servers";
  static const char *s_post_data = NULL;
  if (ev == MG_EV_OPEN) {
    // Connection created. Store connect expiration time in c->data
    *(uint64_t *) c->data = mg_millis() + 1000;
  } else if (ev == MG_EV_POLL) {
    if (mg_millis() > *(uint64_t *) c->data &&
        (c->is_connecting || c->is_resolving)) {
      mg_error(c, "Connect timeout");
    }
  } else if (ev == MG_EV_CONNECT) {
    // Connected to server. Extract host name from URL
    struct mg_str host = mg_url_host(s_url);

    // Send request
    int content_length = s_post_data ? strlen(s_post_data) : 0;
    mg_printf(c,
              "%s %s HTTP/1.0\r\n"
              "Host: %.*s\r\n"
              "Content-Type: octet-stream\r\n"
              "Content-Length: %d\r\n"
              "\r\n",
              s_post_data ? "POST" : "GET", mg_url_uri(s_url), (int) host.len,
              host.ptr, content_length);
    mg_send(c, s_post_data, content_length);
  } else if (ev == MG_EV_HTTP_MSG) {
    // Response is received. Print it
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    iprintf("%.*s", (int) hm->message.len, hm->message.ptr);
    c->is_closing = 1;         // Tell mongoose to close this connection
    *(bool *) fn_data = true;  // Tell event loop to stop
  } else if (ev == MG_EV_ERROR) {
    *(bool *) fn_data = true;  // Error, tell event loop to stop
  }
}

void getServerlist(mg_mgr *mgr)
{
  static const char *s_url = "http://servers.aceattorneyonline.com/servers";
  static const uint64_t s_timeout_ms = 1500;  // Connect timeout in milliseconds
  bool done = false;              // Event handler flips it to true
  mg_http_connect(mgr, s_url, handleServerlist, &done);  // Create client connection
  while (!done) mg_mgr_poll(mgr, 50);      // Event manager loops until 'done'
  mg_mgr_free(mgr);                        // Free resources
}
*/

void pickRandomMusic(Courtroom& court, std::string name)
{
	dirent* dir;
	DIR* d = opendir(name.c_str());
	if (!d)
		return;

	struct file
	{
		std::string name;
		u8 type;
	};
	std::vector<file> items;

	while ((dir = readdir(d)) != 0)
	{
		if (dir->d_name[0] == '.') continue;
		items.push_back({dir->d_name, dir->d_type});
	}

	closedir(d);

	file& item = items[rand() % items.size()];
	std::string newName = name+"/"+item.name;
	if (item.type == DT_DIR)
		pickRandomMusic(court, newName);
	else
		court.playMusic(newName.c_str());
}

void pickRandomBG(Courtroom& court)
{
	dirent* dir;
	DIR* d = opendir("/data/ao-nds/background");
	if (!d)
		return;

	std::vector<std::string> items;

	while ((dir = readdir(d)) != 0)
	{
		if (dir->d_name[0] == '.') continue;
		items.push_back(dir->d_name);
	}

	closedir(d);

	court.getBackground()->setBg(items[rand() % items.size()]);
}

void showDisclaimer()
{
	bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	dmaCopy(bg_disclaimerTiles, bgGetGfxPtr(0), bg_disclaimerTilesLen);
	dmaCopy(bg_disclaimerMap, bgGetMapPtr(0), bg_disclaimerMapLen);
	dmaCopy(bg_disclaimerPal, BG_PALETTE, bg_disclaimerPalLen);

	REG_BLDCNT = BLEND_ALPHA | BLEND_SRC_BG0 | BLEND_DST_BACKDROP;
	REG_BLDALPHA = 0xf00;
}

void fadeDisclaimer() {
	int ticks = 0;
	int alphaAdd = 1;
	while (1)
	{
		swiWaitForVBlank();

		REG_BLDALPHA += alphaAdd;
		if ((REG_BLDALPHA & 0xf) == 0xf && alphaAdd)
			alphaAdd = 0;
		else if ((REG_BLDALPHA & 0xf) == 0)
			break;
		else if (!alphaAdd && ticks++ >= 60*3)
			alphaAdd = -1;
	}

	dmaFillHalfWords(0, bgGetGfxPtr(0), bg_disclaimerTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(0), bg_disclaimerMapLen);
	dmaFillHalfWords(0, BG_PALETTE, 512);
}

int main()
{
	defaultExceptionHandler();
	mp3_init();
	srand(time(0));

	if (!fatInitDefault())
	{
		consoleDemoInit();
		iprintf("Failed to initialize libfat\nPlease check your microSD card\n");
		while (1) swiWaitForVBlank();
	}

	videoSetMode(MODE_3_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankC(VRAM_C_SUB_BG);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	oamInit(&oamMain, SpriteMapping_1D_128, true);
	oamInit(&oamSub, SpriteMapping_1D_128, true);

	initFont(acename_ttf, 13);	// index 0
	initFont(Igiari_ttf, 16);	// index 1
	//initFont(NDS12_ttf, 12);	// index 2

	// printconsole will be removed once UI work actually begins
	PrintConsole subScreen;
	consoleInit(&subScreen, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
	consoleSelect(&subScreen);

	showDisclaimer();

	connect_wifi();

	fadeDisclaimer();

	bgExtPaletteEnable();

	std::string ip = "ws://vanilla.aceattorneyonline.com:2095/";
	gEngine = new Engine;

	//pickRandomBG(*court);

	AOwebSocket* sock = new AOwebSocket;
	//AOtcpSocket* sock = new AOtcpSocket;
	//sock->connectIP(ip, 2086);
	sock->connectIP(ip);
	gEngine->setSocket(sock);

	gEngine->changeScreen(new UIScreenCourt);

	while (1)
	{
		scanKeys();
		if (keysDown() & KEY_SELECT)
		{
			iprintf("disconnecting\n");
			sock->disconnect();
		}

		gEngine->updateInput();
		gEngine->update();

		bgUpdate();
		oamUpdate(&oamMain);

		mp3_fill_buffer();
		swiWaitForVBlank();
	}

	delete gEngine;

	return 0;
}
