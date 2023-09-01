#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <vector>

#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <nds/arm9/exceptions.h>
#include <nds/arm9/video.h>
#include <nds/arm9/input.h>
#include <nds/arm9/sprite.h>
#include <nds/interrupts.h>
#include <nds/ndstypes.h>
#include <fat.h>

#include "mp3_shared.h"
#include "global.h"
#include "fonts.h"
#include "bg_disclaimer.h"
#include "engine.h"
#include "sockets/aowebsocket.h"
#include "sockets/aotcpsocket.h"
#include "ui/uiwificonnect.h"

#include "acename_ttf.h"
#include "Igiari_ttf.h"

/*
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
*/

void showDisclaimer()
{
	bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	dmaCopy(bg_disclaimerTiles, bgGetGfxPtr(0), bg_disclaimerTilesLen);
	dmaCopy(bg_disclaimerMap, bgGetMapPtr(0), bg_disclaimerMapLen);
	dmaCopy(bg_disclaimerPal, BG_PALETTE, bg_disclaimerPalLen);

	REG_BLDCNT = BLEND_FADE_BLACK | BLEND_SRC_BG0;
	REG_BLDY = 16;
}

void fadeDisclaimer() {
	int ticks = 0;
	int alpha = 16;
	int alphaAdd = -1;
	while (1)
	{
		swiWaitForVBlank();

		alpha += alphaAdd;
		REG_BLDY = alpha;
		if (alpha == 0 && alphaAdd)
			alphaAdd = 0;
		else if (alpha == 16)
			break;
		else if (!alphaAdd && ticks++ >= 60*3)
			alphaAdd = 1;
	}

	dmaFillHalfWords(0, bgGetGfxPtr(0), bg_disclaimerTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(0), bg_disclaimerMapLen);
	dmaFillHalfWords(0, BG_PALETTE, 512);
	REG_BLDCNT = BLEND_NONE;
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

	showDisclaimer();

	fadeDisclaimer();

	gEngine = new Engine;
	gEngine->changeScreen(new UIScreenWifi);

	while (1)
	{
		scanKeys();

		gEngine->updateInput();
		gEngine->update();

		bgUpdate();
		oamUpdate(&oamMain);
		oamUpdate(&oamSub);

		mp3_fill_buffer();
		swiWaitForVBlank();
	}

	delete gEngine;

	return 0;
}
