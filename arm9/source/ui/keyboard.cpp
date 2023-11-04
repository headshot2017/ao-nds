#include "ui/keyboard.h"

#include <nds/dma.h>
#include <nds/interrupts.h>

#include "mp3_shared.h"

void keyboardShowAlt(Keyboard* kb)
{
	int i;

	swiWaitForVBlank();

	kb->visible = 1;

	bgSetScroll(kb->background, 0, -192);

	bgShow(kb->background);

	if(kb->scrollSpeed)
	{
		for(i = -192; i < kb->offset_y; i += kb->scrollSpeed)
		{
			mp3_fill_buffer();
			swiWaitForVBlank();
			bgSetScroll(kb->background, 0, i);
			bgUpdate();
		}
	}

	bgSetScroll(kb->background, 0, kb->offset_y);
	bgUpdate();
}

void keyboardHideAlt(Keyboard* kb)
{
	int i;

	kb->visible = 0;

	if(kb->scrollSpeed)
	{
		for(i = kb->offset_y; i > -192; i-= kb->scrollSpeed)
		{
			mp3_fill_buffer();
			swiWaitForVBlank();
			bgSetScroll(kb->background, 0, i);
			bgUpdate();
		}
	}
	bgHide(kb->background);
	bgUpdate();
}


AOkeyboard::AOkeyboard(int lines, int oamStart, int palSlot)
{
	keyboardInit(&m_kb, 1, BgType_Text4bpp, BgSize_T_256x512, 1, 4, false, true);
	mp3_fill_buffer();

	lbl_plswrite = new UILabel(&oamSub, oamStart, 6, 1, RGB15(31,31,31), palSlot, 1);
	lbl_plswrite->setVisible(false);
	lbl_plswrite->setPos(8, 8);
	lbl_written = new UILabel(&oamSub, lbl_plswrite->nextOamInd(), 7, lines, RGB15(31,31,31), palSlot, 0);
	lbl_written->setVisible(false);
	lbl_written->setPos(8, 32);

	enterPressed.cb = 0;
	escPressed.cb = 0;
}

AOkeyboard::~AOkeyboard()
{
	dmaFillHalfWords(0, bgGetGfxPtr(m_kb.background), m_kb.tileLen);
	dmaFillHalfWords(0, bgGetMapPtr(m_kb.background), 1536);

	delete lbl_plswrite;
	delete lbl_written;
}

void AOkeyboard::show(const char* plsWrite, const char* startValue)
{
	if (!startValue) startValue = value.c_str();
	valueOld = value = startValue;

	lbl_plswrite->setVisible(true);
	lbl_plswrite->setText(plsWrite);
	lbl_written->setVisible(true);
	lbl_written->setText(startValue);
	oamUpdate(&oamSub);

	dmaCopy(m_kb.palette, BG_PALETTE_SUB, m_kb.paletteLen);
	keyboardShowAlt(&m_kb);
}

int AOkeyboard::updateInput()
{
	int c = keyboardUpdate();

	if (c == DVK_ENTER || c == DVK_FOLD)
	{
		int ret = -1;
		keyboardHideAlt(&m_kb);

		lbl_plswrite->setVisible(false);
		lbl_written->setVisible(false);

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
			lbl_written->setText(value.c_str());
		}
	}
	else if (c > 0)
	{
		value += c;
		lbl_written->setText(value.c_str());
	}

	return 0;
}

void AOkeyboard::setValue(std::string newValue)
{
	value = newValue;
	lbl_written->setText(newValue.c_str());
}
