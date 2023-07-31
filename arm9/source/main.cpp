#include <stdio.h>
#include <string.h>
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "mp3_shared.h"
#include "global.h"
#include "courtroom.h"

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

	Courtroom court;
	court.setBg("dualdestinies");
	court.setVisible(true);

	connect_wifi();

	getServerlist();

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
