#include "courtroom/chatbox.h"

#include <string.h>

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/video.h>
#include <nds/decompress.h>
#include <nds/interrupts.h>
#include <nds/timers.h>

#include "arm9_math_alt.h"
#include "courtroom/courtroom.h"
#include "global.h"
#include "fonts.h"
#include "content.h"
#include "settings.h"
#include "mini/ini.h"

#define MAX_COLOR_SWITCHES 5

//the speed of the timer when using ClockDivider_1024
#define TIMER_SPEED div32(BUS_CLOCK,1024)

static colorSwitchChar colorSwitches[MAX_COLOR_SWITCHES] = {
	{COLOR_GREEN, '`', '`', true, false, false},
	{COLOR_RED, '~', '~', true, false, false},
	{COLOR_ORANGE, '|', '|', true, false, false},
	{COLOR_BLUE, '(', ')', false, true, false},
	{COLOR_GRAY, '[', ']', true, true, false}
};
static int textSpeedsMS[7] = {10, 20, 30, 40, 60, 75, 100};

static std::u16string filterChatMsg(std::u16string& msg)
{
	std::u16string result;

	for (u32 i=0; i<msg.size(); i++)
	{
		bool add = true;
		char16_t currChar = msg.at(i);

		switch(currChar)
		{
			case '}':
			case '{':
				add = false;
				break;

			case '\\':
				i++;
				if (i < msg.size())
				{
					currChar = msg.at(i);
					switch(currChar)
					{
						case 'n':
							currChar = '\n';
							break;

						case 's':
						case 'f':
							add = false;
							break;
					}
				}
				break;

			default:
				for (u32 j=0; j<MAX_COLOR_SWITCHES; j++)
				{
					if (!colorSwitches[j].show && (currChar == colorSwitches[j].start || currChar == colorSwitches[j].stop))
					{
						add = false;
						break;
					}
				}
				break;
		}

		if (add) result += currChar;
	}

	return result;
}


Chatbox::Chatbox(Courtroom* pCourt)
{
	m_pCourt = pCourt;

	textCanvas = new u8[32*16];
	blipSnd = 0;

	xOffset = 0;
	yOffset = 0;
	bgMap = 0;
	bgPal = 0;
	nameWidth = 0;
	currTextInd = 0;
	currTextGfxInd = 0;
	currTextLine = -1;
	textX = 0;
	textTimer = 0;
	textSpeed = 0;
	blipTicks = 0;
	center = false;

	for (int i=0; i<2; i++)
	{
		nameGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x16, SpriteColorFormat_256Color);
		dmaFillHalfWords((0<<8)|0, nameGfx[i], 32*16);
		oamSet(&oamMain, 24+i, 4+(i*32), 115, 0, 0, SpriteSize_32x16, SpriteColorFormat_256Color, nameGfx[i], -1, false, false, false, false, false);
	}
	for (int i=0; i<8*3; i++)
	{
		int x = i%8;
		int y = i/8;

		textGfx[i] = oamAllocateGfx(&oamMain, SpriteSize_32x16, SpriteColorFormat_256Color);
		dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);
		oamSet(&oamMain, 26+i, 8+(x*32), 132+(y*16), 0, 0, SpriteSize_32x16, SpriteColorFormat_256Color, textGfx[i], -1, false, false, false, false, false);
	}

	memset(textCanvas, 0, 32*16); // black bg


	//bgIndex = bgInit(2, BgType_ExRotation, BgSize_ER_256x256, 2, 1);
	bgIndex = bgInit(1, BgType_Text4bpp, BgSize_T_512x256, 2, 5);
	bgSetPriority(bgIndex, 1);
	bgHide(bgIndex);
	bgUpdate();

	vramSetBankF(VRAM_F_LCD);

	arrowX = 243;
	arrowXadd = 1;
	arrowTicks = 0;

	u8* chatboxArrowImg = readFile("/data/ao-nds/ui/spr_chatboxArrow.img.bin");
	u8* chatboxArrowPal = readFile("/data/ao-nds/ui/spr_chatboxArrow.pal.bin");
	spr_arrowGfx = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
	dmaCopy(chatboxArrowImg, spr_arrowGfx, 16*16);
	dmaCopy(chatboxArrowPal, &VRAM_F_EXT_SPR_PALETTE[5], 512);
	oamSet(&oamMain, 127, arrowX, 174, 0, 5, SpriteSize_16x16, SpriteColorFormat_256Color, spr_arrowGfx, -1, false, false, false, false, false);
	delete[] chatboxArrowImg;
	delete[] chatboxArrowPal;

	VRAM_F_EXT_SPR_PALETTE[0][COLOR_WHITE] = 	PAL_WHITE;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_GREEN] = 	PAL_GREEN;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_RED] = 		PAL_RED;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_ORANGE] = 	PAL_ORANGE;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_BLUE] = 	PAL_BLUE;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_YELLOW] = 	PAL_YELLOW;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_BLACK] = 	PAL_BLACK;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_GRAY] = 	PAL_GRAY;
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_NAME] = 	PAL_WHITE;

	setTheme(Settings::defaultChatbox);

	vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

	visible = false;
	ignoreBlend = false;
	setOnChatboxFinishedCallback(0, 0);
}

Chatbox::~Chatbox()
{
	bgHide(bgIndex);
	dmaFillHalfWords(0, BG_PALETTE, 512);
	if (bgMap) delete[] bgMap;
	if (bgPal) delete[] bgPal;

	if (blipSnd)
		wav_free_handle(blipSnd);

	delete[] textCanvas;
	for (int i=0; i<2; i++)
	{
		oamClearSprite(&oamMain, 24+i);
		oamFreeGfx(&oamMain, nameGfx[i]);
	}
	for (int i=0; i<8*3; i++)
	{
		oamClearSprite(&oamMain, 26+i);
		oamFreeGfx(&oamMain, textGfx[i]);
	}

	oamFreeGfx(&oamMain, spr_arrowGfx);
	oamClearSprite(&oamMain, 127);
	timerStop(2);
}

void Chatbox::setVisible(bool on)
{
	if (visible == on) return;

	if (on)
	{
		u16 oldPal = BG_PALETTE[0];
		if (bgMap) dmaCopy(bgMap, bgGetMapPtr(bgIndex), mapLen);
		if (bgPal) dmaCopy(bgPal, BG_PALETTE, 512);
		BG_PALETTE[0] = oldPal;
		bgShow(bgIndex);
	}
	else
	{
		dmaFillHalfWords(0, bgGetMapPtr(bgIndex), mapLen);
		bgHide(bgIndex);
	}

	visible = on;

	for (int i=0; i<2; i++)
		oamSetHidden(&oamMain, 24+i, !on);

	for (int i=0; i<8*3; i++)
		oamSetHidden(&oamMain, 26+i, !on);

	oamSetHidden(&oamMain, 127, !on);
}

void Chatbox::setTheme(const std::string& name)
{
	if (currTheme == name)
		return;

	mINI::INIFile file("/data/ao-nds/misc/chatboxes/" + name + "/chatbox.ini");
	mINI::INIStructure ini;

	if (bgMap)
	{
		delete[] bgMap;
		bgMap = 0;
	}
	if (bgPal)
	{
		delete[] bgPal;
		bgPal = 0;
	}

	if (!file.read(ini))
	{
		if (name != Settings::defaultChatbox)
		{
			setTheme(Settings::defaultChatbox);
			return;
		}
		else if (name != "default")
		{
			setTheme("default");
			return;
		}

		currTheme = name;

		// set default chatbox coordinates...
		info.height = 80;
		info.nameX = 37;
		info.nameY = 3;
		info.bodyY = 20;
		info.lineSep = 16;
		info.arrowY = 62;

		vramSetBankF(VRAM_F_LCD);
		VRAM_F_EXT_SPR_PALETTE[5][1] = PAL_WHITE;
		VRAM_F_EXT_SPR_PALETTE[0][COLOR_WHITE] = PAL_WHITE;
		VRAM_F_EXT_SPR_PALETTE[0][COLOR_NAME] = PAL_WHITE;
		vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

		bgSetScroll(bgIndex, 0, -192+info.height);
		bgUpdate();

		updateBodyPosition();
		return;
	}

	currTheme = name;

	u32 dataLen;
	u8* bgData = readFile("/data/ao-nds/misc/chatboxes/" + name + "/chatbox.img.bin", &dataLen);
	bgMap = readFile("/data/ao-nds/misc/chatboxes/" + name + "/chatbox.map.bin", &mapLen);
	bgPal = readFile("/data/ao-nds/misc/chatboxes/" + name + "/chatbox.pal.bin");

	dmaCopy(bgData, bgGetGfxPtr(bgIndex), dataLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), mapLen);
	dmaCopy(bgPal, BG_PALETTE, 512);
	BG_PALETTE[0] = 0;

	delete[] bgData;

	info.height = (ini["general"].has("height")) ? std::stoi(ini["general"]["height"]) : 80;
	info.nameX = (ini["name"].has("x")) ? std::stoi(ini["name"]["x"]) : 0;
	info.nameY = (ini["name"].has("y")) ? std::stoi(ini["name"]["y"]) : 0;
	info.bodyY = (ini["body"].has("y")) ? std::stoi(ini["body"]["y"]) : 0;
	info.lineSep = (ini["body"].has("lineSeparation")) ? std::stoi(ini["body"]["lineSeparation"]) : 16;
	info.arrowY = (ini["body"].has("arrowY")) ? std::stoi(ini["body"]["arrowY"]) : 62;

	std::string color = (ini["body"].has("textColor")) ? ini["body"]["textColor"] : "255,255,255";
	std::string nameColor = (ini["name"].has("textColor")) ? ini["name"]["textColor"] : "255,255,255";
	u8 r = std::stoi(argumentAt(color, 0, ',')) >> 3;
	u8 g = std::stoi(argumentAt(color, 1, ',')) >> 3;
	u8 b = std::stoi(argumentAt(color, 2, ',')) >> 3;

	vramSetBankF(VRAM_F_LCD);
	VRAM_F_EXT_SPR_PALETTE[5][1] = RGB15(r, g, b);
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_WHITE] = RGB15(r, g, b);
	VRAM_F_EXT_SPR_PALETTE[0][COLOR_NAME] = RGB15(
		std::stoi(argumentAt(nameColor, 0, ',')) >> 3,
		std::stoi(argumentAt(nameColor, 1, ',')) >> 3,
		std::stoi(argumentAt(nameColor, 2, ',')) >> 3
	);
	vramSetBankF(VRAM_F_SPRITE_EXT_PALETTE);

	bgSetScroll(bgIndex, 0, -192+info.height);
	bgUpdate();

	updateBodyPosition();
}

void Chatbox::setName(std::u16string name)
{
	// clear old text
	memset(textCanvas, 0, 32*16);
	for (int i=0; i<2; i++)
		dmaFillHalfWords((0<<8)|0, nameGfx[i], 32*16);
	nameWidth = getTextWidth(0, name, 64);
	renderText(0, name, COLOR_NAME, 32, 16, textCanvas, SpriteSize_32x16, nameGfx, 2);

	for (int i=0; i<2; i++)
		oamSet(&oamMain, 24+i, info.nameX+(i*32)-(div32(nameWidth,2)), 192-info.height+info.nameY, 0, 0, SpriteSize_32x16, SpriteColorFormat_256Color, nameGfx[i], -1, false, !visible, false, false, false);
}

void Chatbox::setText(std::u16string text, int color, std::string blip)
{
	colorStack = {};
	colorStack.push({color, ' ', ' ', (color != COLOR_BLUE), false, false});

	currTextInd = 0;
	currTextGfxInd = 0;
	currTextLine = -1;
	textX = 0;
	textTimer = 0;
	textSpeed = 3;
	blipTicks = 0;
	center = (text.size() >= 2 && text.at(0) == '~' && text.at(1) == '~');
	currText = (center) ? text.substr(2) : text;
	if (center)
	{
		lines.clear();
		linesHalfWidth.clear();

		std::u16string filtered = filterChatMsg(currText);
		separateLines(1, filtered, 8, true, lines);
		for (u32 i=0; i<lines.size(); i++)
			linesHalfWidth.push_back(div32(getTextWidth(1, lines[i]), 2));
	}
	handleNewLine();

	if (blipSnd)
		wav_free_handle(blipSnd);
	std::string blipFile = Content::getFile("sounds/blips/" + blip + ".wav");
	blipSnd = wav_load_handle(blipFile.c_str());

	oamSetHidden(&oamMain, 127, true);

	memset(textCanvas, 0, 32*16);
	for (int i=0; i<8*3; i++)
		dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);

	timerStop(2);
	timerStart(2, ClockDivider_1024, 0, NULL);
}

void Chatbox::additiveText(std::u16string text, int color)
{
	int line = div32(currTextGfxInd, 8);
	currTextGfxInd = (line+1) * 8;

	colorStack = {};
	colorStack.push({color, ' ', ' ', (color != COLOR_BLUE), false, false});

	currTextInd = 0;
	currTextLine = -1;
	textX = 0;
	textTimer = 0;
	textSpeed = 3;
	blipTicks = 0;
	center = (text.size() >= 2 && text.at(0) == '~' && text.at(1) == '~');
	currText = (center) ? text.substr(2) : text;
	if (center)
	{
		lines.clear();
		linesHalfWidth.clear();

		std::u16string filtered = filterChatMsg(currText);
		separateLines(1, filtered, 8, true, lines);
		for (u32 i=0; i<lines.size(); i++)
			linesHalfWidth.push_back(div32(getTextWidth(1, lines[i]), 2));
	}
	handleNewLine();

	memset(textCanvas, 0, 32*16);
	oamSetHidden(&oamMain, 127, true);

	if (currTextGfxInd >= 8*3)
	{
		currTextGfxInd = 0;
		for (int i=0; i<8*3; i++)
			dmaFillHalfWords((0<<8)|0, textGfx[i], 32*16);
	}

	timerStop(2);
	timerStart(2, ClockDivider_1024, 0, NULL);
}

void Chatbox::update()
{
	if (!visible)
		return;

	if (!ignoreBlend)
	{
		// makes the chatbox transparent.
		// chatbox is in bg1 and we want to see bg0 (court) and sprites behind it
		REG_BLDCNT = BLEND_ALPHA | BLEND_SRC_BG1 | BLEND_DST_BG0 | BLEND_DST_SPRITE | BLEND_DST_BACKDROP;
		REG_BLDALPHA = 0x70f;
	}

	bgSetScroll(bgIndex, -xOffset, -192+info.height-yOffset);

	for (int i=0; i<2; i++)
		oamSetXY(&oamMain, 24+i, info.nameX+(i*32)-(div32(nameWidth,2)) + xOffset, 192-info.height+info.nameY+yOffset);

	arrowTicks++;
	if (arrowTicks >= 3)
	{
		arrowTicks = 0;
		arrowX += arrowXadd;
		if (arrowX >= 246 || arrowX <= 243)
			arrowXadd = -arrowXadd;
		oamSetXY(&oamMain, 127, arrowX, 192-info.height+info.arrowY);
	}

	// handle chatbox text typewriter
	if (isFinished())
	{
		if (visible) oamSetHidden(&oamMain, 127, false);
		return;
	}

	if (blipTicks > 0) blipTicks--;

	u32 elapsed = timerElapsed(2);
	textTimer += f32toint(mulf32(divf32(inttof32(elapsed), inttof32(TIMER_SPEED)), inttof32(1000)));

	if (textTimer > textSpeedsMS[textSpeed])
	{
		textTimer -= textSpeedsMS[textSpeed];

		bool stop = handleControlChars();
		if (stop) return;

		char16_t currChar = currText.at(currTextInd);
		if (currChar == '\\')
		{
			currTextInd++;
			if (isFinished() && onChatboxFinished)
			{
				oamSetHidden(&oamMain, 127, false);
				onChatboxFinished(pUserData);
				timerStop(2);
				return;
			}

			bool skip = handleEscape();
			if (skip)
			{
				currTextInd++;
				if (isFinished() && onChatboxFinished)
				{
					oamSetHidden(&oamMain, 127, false);
					onChatboxFinished(pUserData);
					timerStop(2);
				}
				return;
			}
		}

		if (blipSnd && currChar != ' ' && blipTicks <= 0)
		{
			wav_play(blipSnd);
			blipTicks = 5;
		}

		if (currTextGfxInd >= 8*3)
		{
			// scroll lines upwards, clear 3rd line and start from there.
			currTextGfxInd = 8*2;
			for (int i=0; i<8; i++)
			{
				dmaCopy(textGfx[8*1+i], textGfx[i], 32*16);
				dmaCopy(textGfx[8*2+i], textGfx[8*1+i], 32*16);
				dmaFillHalfWords(0, textGfx[8*2+i], 32*16);
			}
		}

		bool lastBox = (mod32(currTextGfxInd, 8) == 7);
		int boxWidth = lastBox ? 20 : 32;
		int oobFlag;
		int outWidth;
		int new_x = renderChar(1, currText.at(currTextInd), (currTextInd != currText.size()-1) ? currText.at(currTextInd+1) : 0, colorStack.top().color, textX, 32, boxWidth, 16, textCanvas, SpriteSize_32x16, textGfx[currTextGfxInd], lastBox, &oobFlag, &outWidth);

		if (oobFlag)
		{
			currTextGfxInd++;
			memset(textCanvas, 0, 32*16);

			if (mod32(currTextGfxInd, 8) == 0)
			{
				// entered a new line
				textX = 0;
				handleNewLine();
				if (oobFlag == 2)
					currTextInd--;
			}
			else
			{
				textX -= boxWidth;
				textX = renderChar(1, currText.at(currTextInd), (currTextInd != currText.size()-1) ? currText.at(currTextInd+1) : 0, colorStack.top().color, textX, 32, boxWidth, 16, textCanvas, SpriteSize_32x16, textGfx[currTextGfxInd], lastBox, &oobFlag, &outWidth);
			}
		}
		else
		{
			textX = new_x;
			if (textX > boxWidth)
			{
				currTextGfxInd++;
				if (mod32(currTextGfxInd, 8) == 0)
				{
					textX = 0;
					handleNewLine();
				}
				else
					textX -= boxWidth;
			}
		}

		if (colorStack.top().removing)
		{
			colorSwitchChar oldColor = colorStack.top();
			colorStack.pop();
			colorSwitchChar newColor = colorStack.top();
			if (m_pCourt && oldColor.talk != newColor.talk)
				m_pCourt->setTalkingAnim(newColor.talk);
		}

		currTextInd++;
		if (isFinished() && onChatboxFinished)
		{
			oamSetHidden(&oamMain, 127, false);
			onChatboxFinished(pUserData);
			timerStop(2);
		}
	}
}

void Chatbox::updateBodyPosition()
{
	int oldGfx = currTextGfxInd;
	int oldLine = currTextLine;
	currTextGfxInd = 0;
	currTextLine = -1;

	for (int i=0; i<3; i++)
	{
		handleNewLine();
		currTextGfxInd += 8;
	}

	currTextGfxInd = oldGfx;
	currTextLine = oldLine;
}

bool Chatbox::handleEscape()
{
	bool skip = true;

	char16_t escape = currText.at(currTextInd);
	switch(escape)
	{
		case 'n':
			currTextGfxInd = (div32(currTextGfxInd, 8)+1) * 8;
			textX = 0;
			handleNewLine();
			break;

		case 's':
			if (m_pCourt) m_pCourt->shake(5, 35);
			break;

		case 'f':
			if (m_pCourt) m_pCourt->flash(3);
			break;

		default:
			skip = false;
			break;
	}

	return skip;
}

bool Chatbox::handleControlChars()
{
	bool keepSearching = true;
	while (keepSearching)
	{
		char16_t currChar = currText.at(currTextInd);

		switch(currChar)
		{
			case '{':
				// slow down
				if (textSpeed < 6) textSpeed++;
				break;

			case '}':
				// speed up
				if (textSpeed > 0) textSpeed--;
				break;

			default:
				{
					bool switchingColor = false;
					colorSwitchChar oldColor;
					colorSwitchChar newColor;
					if (colorStack.size() > 1 && currChar == colorStack.top().stop)
					{
						oldColor = colorStack.top();
						if (!colorStack.top().show) colorStack.pop();
						else colorStack.top().removing = true;
						newColor = colorStack.top();
						switchingColor = true;
					}

					if (!switchingColor)
					{
						for (int i=0; i<MAX_COLOR_SWITCHES; i++)
						{
							if (currChar == colorSwitches[i].start)
							{
								oldColor = colorStack.top();
								colorStack.push(colorSwitches[i]);
								switchingColor = true;
								newColor = colorSwitches[i];
								break;
							}
						}
					}

					if (m_pCourt && switchingColor && !colorStack.top().removing && oldColor.talk != newColor.talk)
						m_pCourt->setTalkingAnim(newColor.talk);

					if (!switchingColor || newColor.show)
						keepSearching = false;
				}
				break;
		}

		if (keepSearching)
		{
			currTextInd++;
			if (isFinished() && onChatboxFinished)
			{
				oamSetHidden(&oamMain, 127, false);
				onChatboxFinished(pUserData);
				return true;
			}
		}
	}

	return false;
}

void Chatbox::handleNewLine()
{
	currTextLine++;
	int start = (currTextGfxInd < 8*3) ? currTextGfxInd : 0;

	for (int i=start; i<start+8; i++)
	{
		int x = (center) ? (128 + (mod32(i,8))*32 - linesHalfWidth[currTextLine]) : (8 + (mod32(i,8))*32);
		int y = 192-info.height+info.bodyY + (div32(i,8))*info.lineSep;
		oamSetXY(&oamMain, 26+i, x, y);
	}
}
