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
#include "courtroom.h"
#include "bg_disclaimer.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
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

	PrintConsole subScreen;
	consoleInit(&subScreen, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
	consoleSelect(&subScreen);

	/*
	// stb_truetype test
	{
		vramSetBankG(VRAM_G_LCD);

		// set text colors
		VRAM_G_EXT_SPR_PALETTE[0][0] = RGB15(0,0,0); // transparency
		VRAM_G_EXT_SPR_PALETTE[0][1] = RGB15(31,31,31); // white
		VRAM_G_EXT_SPR_PALETTE[0][2] = RGB15(0,31,0); // green
		VRAM_G_EXT_SPR_PALETTE[0][3] = RGB15(31,0,0); // red
		VRAM_G_EXT_SPR_PALETTE[0][4] = RGB15(31,20,0); // orange
		VRAM_G_EXT_SPR_PALETTE[0][5] = RGB15(5,18,31); // blue
		VRAM_G_EXT_SPR_PALETTE[0][6] = RGB15(31,31,0); // yellow

		vramSetBankG(VRAM_G_SPRITE_EXT_PALETTE);

		struct spriteSize
		{
			u32 w;
			u32 h;
			SpriteSize spritesize;
		};
		spriteSize size = {32, 16, SpriteSize_32x16};

		u16* gfx = oamAllocateGfx(&oamMain, size.spritesize, SpriteColorFormat_256Color);
		dmaFillHalfWords((1<<8)|1, gfx, size.w*size.h); // fill 256 color sprite with SPRITE_PALETTE_SUB color from index 2
		dmaFillHalfWords((0<<8)|0, gfx, size.w*size.h); // fill 256 color sprite with SPRITE_PALETTE_SUB color from index 2
		u8* bitmap = (u8*)calloc(size.w*size.h, sizeof(u8));

		stbtt_fontinfo info;
		if (!stbtt_InitFont(&info, Igiari_ttf, 0))
		{
			iprintf("font init failed\n");
			while (1) swiWaitForVBlank();
		}

		int l_h = 16;
		float scale = stbtt_ScaleForPixelHeight(&info, l_h);

		const char* word = "testing!";
		//const char* word = "ng!";

		int ascent, descent, lineGap;
		stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

		ascent = roundf(ascent * scale);
		descent = roundf(descent * scale);

		int x = 0;

		for (u32 i = 0; i < strlen(word); ++i)
		{
			// how wide is this character
			int ax;
			int lsb;
			stbtt_GetCodepointHMetrics(&info, word[i], &ax, &lsb);
			// (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].)

			// get bounding box for character (may be offset to account for chars that dip above or below the line)
			int c_x1, c_y1, c_x2, c_y2;
			stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

			int out_x = c_x2 - c_x1;
			bool oob = (x + out_x >= (int)size.w);
			if (oob)
				out_x = size.w - x;

			// compute y (different characters have different heights)
			int y = ascent + c_y1;

			// render character (stride and offset is important here)
			int byteOffset = x + roundf(lsb * scale) + (y * size.w);

			stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, out_x, c_y2 - c_y1, size.w, scale, scale, word[i]);
			if (oob)
			{
				x -= size.w;
				memset(bitmap, 0, size.w*size.h);
				dmaFillHalfWords((0<<8)|0, gfx, size.w*size.h);

				byteOffset = x + roundf(lsb * scale) + (y * size.w);
				out_x = c_x2 - c_x1;
				stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, out_x, c_y2 - c_y1, size.w, scale, scale, word[i]);
			}
			for (int yy = y; yy<y+l_h; yy++)
			{
				for (int xx = x; xx<x + out_x; xx++)
				{
					u32 ind = yy*size.w+xx;
					if (bitmap[ind] == 0xff)
					{
						u32 leftOrRight = ind&1;
						bool oobFlag;
						u32 targetInd = bmpIndexTo256SpriteIndex((u32)xx, (u32)yy, size.w, size.h, size.spritesize, &oobFlag);
						if (oobFlag) continue;

						gfx[targetInd] = (leftOrRight) ?
							(gfx[targetInd] & 0xf) | (2<<8) :
							2 | ((gfx[targetInd] >> 8) << 8);
					}
				}
			}
			//if (oob) break;

			// advance x
			x += roundf(ax * scale);

			// add kerning
			int kern;
			kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
			x += roundf(kern * scale);
		}

		u32 amount = 0;

		while (1)
		{
			scanKeys();
			u32 key = keysHeld();
			if (key & KEY_A)
			{
				u32 i = ++amount;
				u32 x = i%size.w;
				u32 y = i/size.w;
				u32 tilePixelX = x%8;
				u32 tilePixelY = y%8;
				u32 metaTileX = x/8;
				u32 metaTileY = y/8;
				u32 targetInd = ((tilePixelY*8) + (metaTileX*64) + (metaTileY*64 * (size.w/8)) + tilePixelX) / 2;
				u32 leftOrRight = i&1;

				gfx[targetInd] = (leftOrRight) ?
					(2<<8) | (gfx[targetInd] & 0xf) :
					((gfx[targetInd] >> 8) << 8) | 2;

				iprintf("%d, %d %d, %d %d, %d %d, %d %d\n", i, x, y, tilePixelX, tilePixelY, metaTileX, metaTileY, leftOrRight, targetInd);
			}
			if (key & KEY_B && amount)
			{
				u32 i = --amount;
				u32 x = i%size.w;
				u32 y = i/size.w;
				u32 tilePixelX = x%8;
				u32 tilePixelY = y%8;
				u32 metaTileX = x/8;
				u32 metaTileY = y/8;
				u32 targetInd = ((tilePixelY*8) + (metaTileX*64) + (metaTileY*64 * (size.w/8)) + tilePixelX) / 2;
				u32 leftOrRight = i&1;

				gfx[targetInd] = (leftOrRight) ?
					(0<<8) | (gfx[targetInd] & 0xf) :
					((gfx[targetInd] >> 8) << 8) | 0;

				iprintf("%d, %d %d, %d %d, %d %d, %d %d\n", i, x, y, tilePixelX, tilePixelY, metaTileX, metaTileY, leftOrRight, targetInd);
			}

			oamSet(&oamMain, 0, 256-size.w, 192-size.h, 0, 0, size.spritesize, SpriteColorFormat_256Color, gfx, -1, false, false, false, false, false);
			oamUpdate(&oamMain);
			swiWaitForVBlank();
		}
	}
	*/

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
			int ind = (ticks/60) % 6;
			court.setBgSide(sides[ind]);
		}

		court.update();
		mp3_fill_buffer();

		swiWaitForVBlank();
	}

	return 0;
}
