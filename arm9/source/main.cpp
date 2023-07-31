#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <nds/arm9/console.h>
#include <nds/arm9/exceptions.h>
#include <nds/arm9/video.h>
#include <nds/arm9/input.h>
#include <nds/arm9/sprite.h>
#include <nds/interrupts.h>
#include <fat.h>
#include <dswifi9.h>
#include <netinet/in.h>

#include "mp3_shared.h"
#include "global.h"
#include "courtroom.h"

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

	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankC(VRAM_C_SUB_BG);
	vramSetBankD(VRAM_D_SUB_SPRITE);
	vramSetBankE(VRAM_E_MAIN_SPRITE);

	oamInit(&oamMain, SpriteMapping_1D_128, false);

	PrintConsole subScreen;
	consoleInit(&subScreen, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
	consoleSelect(&subScreen);

	consoleDemoInit();  //setup the sub screen for printing

	//mp3_play("aaa.mp3");

    Courtroom court;
    court.setBg("dualdestinies");
    court.setVisible(true);
	
	connect_wifi()

    int ticks=0;
	while (1)
	{
        scanKeys();

	    ticks++;
	    if (ticks % 60 == 0)
        {
            std::string sides[] = {"def", "pro", "wit", "hld", "hlp", "jud"};
            int ind = (ticks/60) % 6;
            court.setBgSide(sides[ind]);
        }

	    court.update();
		mp3_fill_buffer();

		swiWaitForVBlank();
	}

    return 0;
}

void connect_wifi()
{
	struct in_addr ip, gateway, mask, dns1, dns2;

	iprintf("Connecting via WFC data ...\n");

	if(!Wifi_InitDefault(WFC_CONNECT)) {
		iprintf("Failed to connect!");
		while (1) swiWaitForVBlank();
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