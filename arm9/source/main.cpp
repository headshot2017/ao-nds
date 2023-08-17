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
#include "websocket/mongoose.h"
#include "global.h"
#include "courtroom/chatbox.h"
#include "courtroom/courtroom.h"
#include "fonts.h"
#include "bg_disclaimer.h"
#include "packets.h"

#include "NDS12_ttf.h"
#include "acename_ttf.h"
#include "Igiari_ttf.h"

Courtroom* court;

void connect_wifi()
{
	struct in_addr ip, gateway, mask, dns1, dns2;

	iprintf("Connecting via WFC data...\n");

	if(!Wifi_InitDefault(WFC_CONNECT)) {
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
	}
}

// Print HTTP response and signal that we're done
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

// Print websocket response and signal that we're done
static void wsHandler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_OPEN) {
    c->is_hexdumping = 0;
  } else if (ev == MG_EV_ERROR) {
    // On error, log error message
    MG_ERROR(("%p %s", c->fd, (char *) ev_data));
  } else if (ev == MG_EV_WS_OPEN) {
    // When websocket handshake is successful, send message
    mg_ws_send(c, "HI#NDS#%", 8, WEBSOCKET_OP_TEXT);
  } else if (ev == MG_EV_WS_MSG) {
    // When we get echo response, print it
    struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
    iprintf("S: [%.*s]\n", (int) wm->data.len, wm->data.ptr);
	handleNetworkPacket(c, *court, wm->data.ptr);
  }

  if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE || ev == MG_EV_WS_MSG) {
    *(bool *) fn_data = true;  // Signal that we're done
  }
}

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
}

void fadeDisclaimer() {
	REG_BLDCNT = BLEND_ALPHA | BLEND_SRC_BG0 | BLEND_DST_BACKDROP;
	REG_BLDALPHA = 0xf00;

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

	initFont(NDS12_ttf, 12);	// index 0
	initFont(acename_ttf, 13);	// index 1
	initFont(Igiari_ttf, 16);	// index 2
	PrintConsole subScreen;
	consoleInit(&subScreen, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
	consoleSelect(&subScreen);

	showDisclaimer();

	connect_wifi();
	struct mg_mgr mgr;        // Event manager
	bool done = false;        // Event handler flips it to true
	mg_mgr_init(&mgr);        // Initialise event manager
	mg_log_set(MG_LL_ERROR);  // Set log level
	//getServerlist(&mgr);

	fadeDisclaimer();

	bgExtPaletteEnable();

	std::string serverURL = "ws://vanilla.aceattorneyonline.com:2095/";
	while(1)
	{
		// the charselect should be here
		break;
	}

	court = new Courtroom;

	//pickRandomBG(*court);

	court->setVisible(true);
	//court->getChatbox()->setName("Adrian");
	//court->getChatbox()->setText("Test", COLOR_BLUE);
	//court->getCharacter()->setCharImage("Adrian", "(a)thinking");

	iprintf("connect server\n");
	struct mg_connection *c = mg_ws_connect(&mgr, serverURL.c_str(), wsHandler, &done, NULL);     // Create client

	int ticks=0;
	while (1)
	{
		scanKeys();
		u32 keys = keysDown();
		if (keys & KEY_A)
			pickRandomMusic(*court, "/data/ao-nds/sounds/music");
		if (keys & KEY_B)
			pickRandomBG(*court);
		if (keys & KEY_Y)
			court->shake(5, 60);

		court->update();

		bgUpdate();
		oamUpdate(&oamMain);

		mp3_fill_buffer();
		mg_mgr_poll(&mgr, 0);
		swiWaitForVBlank();
	}
	mg_mgr_free(&mgr);
	return 0;
}
