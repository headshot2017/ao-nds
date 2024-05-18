#include "ui/court/ooc.h"

#include <nds/dma.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sound.h>

#include "utf8.h"
#include "mp3_shared.h"
#include "engine.h"
#include "ui/court/ingamemenu.h"
#include "ui/court/oocpresets.h"

UICourtOOC::~UICourtOOC()
{
	delete btn_back;
	delete btn_presets;
	delete btn_scrollUp;
	delete btn_scrollDown;
	delete btn_sliderHandle;
	delete lbl_log;
	delete lbl_oocName;
	delete kb_input;

	gEngine->getSocket()->removeMessageCallback("CT", cbCT);
}

void UICourtOOC::init()
{
	scrollPos = (pCourtUI->getOOCLog().size() > 12) ? pCourtUI->getOOCLog().size()-12 : 0;
	scrollPosOld = scrollPos;
	atBottom = true;

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_ooc");

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 0);
	btn_presets = new UIButton(&oamSub, "/data/ao-nds/ui/spr_presets", btn_back->nextOamInd(), 2, 1, SpriteSize_32x16, 256-59-1, 1, 59, 15, 32, 16, 1);
	btn_scrollUp = new UIButton(&oamSub, "/data/ao-nds/ui/spr_scrollUp", btn_presets->nextOamInd(), 1, 1, SpriteSize_16x32, 242, 19, 14, 19, 16, 32, 2);
	btn_scrollDown = new UIButton(&oamSub, "/data/ao-nds/ui/spr_scrollDown", btn_scrollUp->nextOamInd(), 1, 1, SpriteSize_16x32, 242, 156, 14, 19, 16, 32, 3);
	btn_sliderHandle = new UIButton(&oamSub, "/data/ao-nds/ui/spr_sliderHandle", btn_scrollDown->nextOamInd(), 1, 1, SpriteSize_16x32, btn_scrollUp->getX(), btn_scrollUp->getY()+btn_scrollUp->getH(), 14, 19, 16, 32, 4);
	lbl_log = new UILabel(&oamSub, btn_sliderHandle->nextOamInd(), 7, 12, RGB15(31,31,31), 5, 0);
	lbl_oocName = new UILabel(&oamSub, lbl_log->nextOamInd(), 4, 1, RGB15(31,31,31), 5, 0);

	kb_input = new AOkeyboard(3, lbl_oocName->nextOamInd(), 5);
	dmaCopy(bgPal, BG_PALETTE_SUB, 512);
	isWritingChat = false;

	lbl_log->setPos(9, 17);
	lbl_oocName->setPos(80, 163);
	lbl_oocName->setText((pCourtUI->oocName.empty()) ? u"Enter an OOC name..." : pCourtUI->oocName);

	btn_back->assignKey(KEY_B);
	btn_presets->assignKey(KEY_R);
	btn_scrollUp->assignKey(KEY_UP);
	btn_scrollDown->assignKey(KEY_DOWN);

	btn_back->connect(onBackClicked, this);
	btn_presets->connect(onPresetsClicked, this);
	btn_scrollUp->connect(onScrollUpPressed, this, UIButton::PRESSED);
	btn_scrollDown->connect(onScrollDownPressed, this, UIButton::PRESSED);
	btn_scrollUp->connect(onScrollBtnReleased, this, UIButton::RELEASED);
	btn_scrollDown->connect(onScrollBtnReleased, this, UIButton::RELEASED);
	btn_sliderHandle->connect(onSliderPressed, this, UIButton::PRESSED);
	btn_sliderHandle->connect(onSliderReleased, this, UIButton::RELEASED);

	holdWait = -1;
	pageAdd = 0;
	draggingHandle = false;

	cbCT = gEngine->getSocket()->addMessageCallback("CT", onMessageCT, this);

	reloadScroll(true);
}

void UICourtOOC::updateInput()
{
	if (kb_input->isVisible())
	{
		int result = kb_input->updateInput();
		if (result != 0)
		{
			if (isWritingChat)
				gEngine->getSocket()->sendData("CT#" + utf8::utf16to8(pCourtUI->oocName) + "#" + kb_input->getValueUTF8() + "#%");
			else
				pCourtUI->oocName = kb_input->getValue();

			dmaCopy(bgPal, BG_PALETTE_SUB, 512);
			bgShow(bgIndex);

			btn_back->setVisible(true);
			btn_presets->setVisible(true);
			btn_scrollUp->setVisible(true);
			btn_scrollDown->setVisible(true);
			btn_sliderHandle->setVisible(true);
			lbl_log->setVisible(true);
			lbl_oocName->setVisible(true);
			lbl_oocName->setText((pCourtUI->oocName.empty()) ? u"Enter an OOC name..." : pCourtUI->oocName);
		}
		return;
	}

	btn_back->updateInput();
	btn_presets->updateInput();
	btn_scrollUp->updateInput();
	btn_scrollDown->updateInput();
	btn_sliderHandle->updateInput();

	if (keysDown() & KEY_TOUCH)
	{
		touchPosition pos;
		touchRead(&pos);
		if (pos.px >= 79 && pos.py >= 162 && pos.px < 79+162 && pos.py < 162+14)
		{
			// ooc name input
			wav_play(pCourtUI->sndSelect);

			hideEverything();
			isWritingChat = false;
			kb_input->show16("Enter your OOC name", pCourtUI->oocName);
		}
		else if (pos.px >= 79 && pos.py >= 177 && pos.px < 79+177 && pos.py < 177+15)
		{
			// chat message input
			wav_play(pCourtUI->sndSelect);

			hideEverything();
			isWritingChat = true;
			kb_input->show("Enter OOC chat message", "");
		}
	}
}

void UICourtOOC::update()
{
	if (pageAdd)
	{
		holdWait--;
		if (holdWait <= 0)
		{
			setScroll(scrollPos + pageAdd);

			if (scrollPos == 0 || scrollPos+12 >= pCourtUI->getOOCLog().size())
			{
				(pageAdd == 1) ? btn_scrollUp->forceRelease() : btn_scrollDown->forceRelease();
				pageAdd = 0;
			}

			holdWait = 1;
		}
	}

	if (draggingHandle)
	{
		touchPosition touchPos;
		touchRead(&touchPos);

		int handleEdges[2] = {btn_scrollUp->getY()+btn_scrollUp->getH(), btn_scrollDown->getY()-btn_scrollDown->getH()};
		touchPos.py -= 9;
		int yPos = (touchPos.py < handleEdges[0]) ? handleEdges[0] : (touchPos.py > handleEdges[1]) ? handleEdges[1] : touchPos.py;
		btn_sliderHandle->setPos(btn_sliderHandle->getX(), yPos);
	}
}

void UICourtOOC::hideEverything()
{
	bgHide(bgIndex);

	btn_back->setVisible(false);
	btn_presets->setVisible(false);
	btn_scrollUp->setVisible(false);
	btn_scrollDown->setVisible(false);
	btn_sliderHandle->setVisible(false);
	lbl_log->setVisible(false);
	lbl_oocName->setVisible(false);
}

void UICourtOOC::setScroll(u32 i)
{
	scrollPos = i;
	reloadScroll();
}

void UICourtOOC::reloadScroll(bool all)
{
	int diff = scrollPos - scrollPosOld;
	if (!all && !diff)
	{
		setSliderHandle();
		return;
	}

	u32 diffAbs = abs(diff);
	int diffSign = (diff>0) ? 1 : (diff<0) ? -1 : 0;

	u32 start, end;
	int add;

	if (all || diffAbs >= 12)
	{
		start = 0;
		end = 12;
		add = 1;
	}
	else
	{
		// method for faster scrolling

		// first, move old ones
		start = (diffSign==1) ? 0 : 12-1;
		end = (diffSign==1) ? 12-diffAbs : diffAbs-1;
		add = diffSign;

		for (u32 i=start; i!=end; i+=add)
		{
			mp3_fill_buffer();

			dmaCopy(lbl_log->getGfx()[7*(i+diff)], lbl_log->getGfx()[7*i], 32*16*7);

			mp3_fill_buffer();
		}

		// then generate new text
		start = end;
		end = (diffSign==1) ? 12 : -1;
	}

	for (u32 i=start; i!=end; i+=add)
	{
		if (scrollPos+i >= pCourtUI->getOOCLog().size())
			break;
		lbl_log->setTextOnLine(pCourtUI->getOOCLog()[scrollPos+i]+u"\n", i);

		mp3_fill_buffer();
	}

	scrollPosOld = scrollPos;

	setSliderHandle();
	atBottom = (pCourtUI->getOOCLog().size() <= 12 || scrollPos == pCourtUI->getOOCLog().size()-12);
}

void UICourtOOC::setSliderHandle()
{
	int handleEdges[2] = {btn_scrollUp->getY()+btn_scrollUp->getH(), btn_scrollDown->getY()-btn_scrollDown->getH()};
	int yPos = handleEdges[0];
	if (pCourtUI->getOOCLog().size() > 12)
		yPos = handleEdges[0] + ((handleEdges[1] - handleEdges[0]) * (scrollPos / (float)(pCourtUI->getOOCLog().size()-12)));
	btn_sliderHandle->setPos(btn_sliderHandle->getX(), yPos);
}

void UICourtOOC::onBackClicked(void* pUserData)
{
	UICourtOOC* pSelf = (UICourtOOC*)pUserData;

	wav_play(pSelf->pCourtUI->sndCancel);
	pSelf->pCourtUI->changeScreen(new UICourtIngameMenu(pSelf->pCourtUI));
}

void UICourtOOC::onPresetsClicked(void* pUserData)
{
	UICourtOOC* pSelf = (UICourtOOC*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->pCourtUI->changeScreen(new UICourtOOCPresets(pSelf->pCourtUI));
}

void UICourtOOC::onScrollUpPressed(void* pUserData)
{
	UICourtOOC* pSelf = (UICourtOOC*)pUserData;

	if (!pSelf->scrollPos) return;
	pSelf->setScroll(pSelf->scrollPos-1);

	pSelf->holdWait = 20;
	pSelf->pageAdd = -1;
}

void UICourtOOC::onScrollDownPressed(void* pUserData)
{
	UICourtOOC* pSelf = (UICourtOOC*)pUserData;

	if (pSelf->scrollPos+12 >= pSelf->pCourtUI->getOOCLog().size()) return;
	pSelf->setScroll(pSelf->scrollPos+1);

	pSelf->holdWait = 20;
	pSelf->pageAdd = 1;
}

void UICourtOOC::onScrollBtnReleased(void* pUserData)
{
	UICourtOOC* pSelf = (UICourtOOC*)pUserData;

	pSelf->pageAdd = 0;
}

void UICourtOOC::onSliderPressed(void* pUserData)
{
	UICourtOOC* pSelf = (UICourtOOC*)pUserData;

	pSelf->draggingHandle = true;
}

void UICourtOOC::onSliderReleased(void* pUserData)
{
	UICourtOOC* pSelf = (UICourtOOC*)pUserData;

	pSelf->draggingHandle = false;

	int handleEdges[2] = {pSelf->btn_scrollUp->getY()+pSelf->btn_scrollUp->getH(), pSelf->btn_scrollDown->getY()-pSelf->btn_scrollDown->getH()};
	u32 yPos = 0;
	if (pSelf->pCourtUI->getOOCLog().size() > 12)
		yPos = (pSelf->btn_sliderHandle->getY() - handleEdges[0]) / (float)(handleEdges[1]-handleEdges[0]) * (pSelf->pCourtUI->getOOCLog().size()-12);
	pSelf->setScroll(yPos);
}

void UICourtOOC::onMessageCT(void* pUserData, std::string msg)
{
	UICourtOOC* pSelf = (UICourtOOC*)pUserData;

	if (pSelf->atBottom)
		pSelf->setScroll((pSelf->pCourtUI->getOOCLog().size() > 12) ? pSelf->pCourtUI->getOOCLog().size()-12 : 0);
	else
		pSelf->setSliderHandle();
}
