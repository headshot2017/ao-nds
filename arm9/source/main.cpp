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
#include "chatbox.h"
#include "courtroom.h"
#include "fonts.h"
#include "bg_disclaimer.h"

#include "NDS12_ttf.h"
#include "acename_ttf.h"
#include "Igiari_ttf.h"

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

void getServerlist()
{
	// store the HTTP request for later
	const char * request_text =
		"GET /servers HTTP/1.1\r\n"
		"Host: servers.aceattorneyonline.com\r\n"
		"User-Agent: Nintendo DS\r\n\r\n";

	// Find the IP address of the server, with gethostbyname
	struct hostent * myhost = gethostbyname( "servers.aceattorneyonline.com" );
	iprintf("Found IP Address!\n");

	// Create a TCP socket
	int my_socket;
	my_socket = socket( AF_INET, SOCK_STREAM, 0 );
	iprintf("Created Socket!\n");

	// Tell the socket to connect to the IP address we found, on port 80 (HTTP)
	struct sockaddr_in sain;
	sain.sin_family = AF_INET;
	sain.sin_port = htons(80);
	sain.sin_addr.s_addr= *( (unsigned long *)(myhost->h_addr_list[0]) );
	connect( my_socket,(struct sockaddr *)&sain, sizeof(sain) );
	iprintf("Connected to server!\n");

	// send our request
	send( my_socket, request_text, strlen(request_text), 0 );
	iprintf("Sent our request!\n");

	// Print incoming data
	iprintf("Printing incoming data:\n");

	int recvd_len;
	char incoming_buffer[256];

	while( ( recvd_len = recv( my_socket, incoming_buffer, 255, 0 ) ) > 1 ) { // if recv returns 0, the socket has been closed.
		if(recvd_len>0) { // data was received!
			incoming_buffer[recvd_len] = 0; // null-terminate
			iprintf(incoming_buffer);
		}
	}

	iprintf("Other side closed connection!");

	shutdown(my_socket,0); // good practice to shutdown the socket.

	closesocket(my_socket); // remove the socket.
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

	court.setBg(items[rand() % items.size()]);
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

	// show disclaimer screen
	{
		bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
		dmaCopy(bg_disclaimerTiles, bgGetGfxPtr(0), bg_disclaimerTilesLen);
		dmaCopy(bg_disclaimerMap, bgGetMapPtr(0), bg_disclaimerMapLen);
		dmaCopy(bg_disclaimerPal, BG_PALETTE, bg_disclaimerPalLen);

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

	bgExtPaletteEnable();

	Courtroom court;
	pickRandomBG(court);
	court.setVisible(true);
	court.getChatbox()->setName("Phoenix");

	//connect_wifi();

	//getServerlist();

	pickRandomMusic(court, "/data/ao-nds/sounds/music");

	int ticks=0;
	while (1)
	{
		scanKeys();
		u32 keys = keysDown();
		if (keys & KEY_A)
			pickRandomMusic(court, "/data/ao-nds/sounds/music");
		if (keys & KEY_B)
			pickRandomBG(court);

		ticks++;
		if (ticks % 60 == 0)
		{
			std::string sides[] = {"def", "pro", "wit", "hld", "hlp", "jud"};
			std::string names[] = {"Phoenix", "Payne", "Sahwit", "Mia", "noby", "Judge"};
			int ind = (ticks/60) % 6;
			court.setBgSide(sides[ind]);
			court.getChatbox()->setName(names[ind].c_str());
		}

		court.update();
		mp3_fill_buffer();

		swiWaitForVBlank();
	}

	return 0;
}
