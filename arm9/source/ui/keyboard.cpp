#include "ui/keyboard.h"

#include <nds/dma.h>
#include <nds/interrupts.h>

#include "utf8.h"
#include "libadx.h"
#include "wifikb/wifikb.h"

AOkeyboard::AOkeyboard(int lines, int oamStart, int palSlot)
{
	keyboardInit(NULL, 1, BgType_Text4bpp, BgSize_T_256x512, 1, 4, false, false);

	decompress(keyboardGetDefault()->tiles, bgGetGfxPtr(5), LZ77Vram);
	const KeyMap *map = keyboardGetDefault()->mappings[keyboardGetDefault()->state];
	size_t map_size = (map->width * map->height *
					   keyboardGetDefault()->grid_height * keyboardGetDefault()->grid_width * 2) / 64;
	dmaCopy(map->mapDataReleased, bgGetMapPtr(5), map_size);

	adx_update();

	lbl_plswrite = new UILabel(&oamSub, oamStart, 6, 1, RGB15(31,31,31), palSlot, 1);
	lbl_plswrite->setVisible(false);
	lbl_plswrite->setPos(8, 8);
	lbl_written = new UILabel(&oamSub, lbl_plswrite->nextOamInd(), 7, lines, RGB15(31,31,31), palSlot, 0);
	lbl_written->setVisible(false);
	lbl_written->setPos(8, 32);

	enterPressed.cb = 0;
	escPressed.cb = 0;
	visible = false;
}

AOkeyboard::~AOkeyboard()
{
	delete lbl_plswrite;
	delete lbl_written;
}

void AOkeyboard::show16(const char* plsWrite, std::u16string startValue)
{
	if (startValue.size() == 1 && startValue.at(0) == u'\0') startValue = value;
	valueOld = value = startValue;

	lbl_plswrite->setVisible(true);
	lbl_plswrite->setText(plsWrite);
	lbl_written->setVisible(true);
	lbl_written->setText(startValue);
	oamUpdate(&oamSub);

	wifikb::send(plsWrite);
	wifikb::start();

	dmaCopy(keyboardGetDefault()->palette, BG_PALETTE_SUB, keyboardGetDefault()->paletteLen);
	keyboardShow();

	visible = true;
}

void AOkeyboard::show(const char* plsWrite, std::string startValue)
{
	show16(plsWrite, utf8::utf8to16(startValue));
}

void AOkeyboard::hide()
{
	wifikb::stop();
	keyboardHide();

	lbl_plswrite->setVisible(false);
	lbl_written->setVisible(false);

	visible = false;
}

int AOkeyboard::updateInput()
{
	int c = keyboardUpdate();
	s32 wifikey;
	if (wifikb::getKey(&wifikey))
		c = wifikey;

	if (c == DVK_ENTER || c == DVK_FOLD)
	{
		int ret = -1;
		hide();

		if (c == DVK_FOLD)
			value = valueOld;
		else
			ret = 1;

		return ret;
	}
	else if (c == DVK_BACKSPACE)
	{
		if (!value.empty())
		{
			value = value.substr(0, value.size()-1);
			lbl_written->setText(value);
		}
	}
	else if (c > 0)
	{
		value += c;
		lbl_written->setText(value);
	}

	return 0;
}

void AOkeyboard::setValue(std::u16string newValue)
{
	value = newValue;
	lbl_written->setText(newValue);
}

bool AOkeyboard::isVisible()
{
	return visible;
}

std::u16string& AOkeyboard::getValue()
{
	return value;
}

std::string AOkeyboard::getValueUTF8()
{
	return utf8::utf16to8(value);
}
