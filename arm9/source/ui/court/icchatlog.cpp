#include "ui/court/icchatlog.h"

#include <nds/dma.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sound.h>

#include "libadx.h"
#include "engine.h"
#include "ui/court/ingamemenu.h"
#include "ui/court/courtrecord.h"

UICourtICChatLog::~UICourtICChatLog()
{
	delete btn_back;
	delete btn_courtRecord;
	delete btn_scrollUp;
	delete btn_scrollDown;
	delete btn_sliderHandle;
	delete lbl_log;

	gEngine->getSocket()->removeMessageCallback("MS", cbMS);
	gEngine->getSocket()->removeMessageCallback("MC", cbMC);
}

void UICourtICChatLog::init()
{
	scrollPos = (pCourtUI->getICLog().size() > 10) ? pCourtUI->getICLog().size()-10 : 0;
	scrollPosOld = scrollPos;
	atBottom = true;

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_icLog", true);

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-32, 80, 32, 32, 32, 0);
	btn_courtRecord = new UIButton(&oamSub, "/data/ao-nds/ui/spr_courtRecord", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-80, 0, 80, 32, 32, 32, 1);
	btn_scrollUp = new UIButton(&oamSub, "/data/ao-nds/ui/spr_scrollUp", btn_courtRecord->nextOamInd(), 1, 1, SpriteSize_16x32, 242, 33, 14, 19, 16, 32, 2);
	btn_scrollDown = new UIButton(&oamSub, "/data/ao-nds/ui/spr_scrollDown", btn_scrollUp->nextOamInd(), 1, 1, SpriteSize_16x32, 242, 156, 14, 19, 16, 32, 3);
	btn_sliderHandle = new UIButton(&oamSub, "/data/ao-nds/ui/spr_sliderHandle", btn_scrollDown->nextOamInd(), 1, 1, SpriteSize_16x32, btn_scrollUp->getX(), btn_scrollUp->getY()+btn_scrollUp->getH(), 14, 19, 16, 32, 4);
	lbl_log = new UILabel(&oamSub, btn_sliderHandle->nextOamInd(), 7, 10, RGB15(31,31,31), 5, 0);
	lbl_log->setPos(9, 32);

	btn_back->assignKey(KEY_B);
	btn_courtRecord->assignKey(KEY_R);
	btn_scrollUp->assignKey(KEY_UP);
	btn_scrollDown->assignKey(KEY_DOWN);

	btn_back->connect(onBackClicked, this);
	btn_courtRecord->connect(onCourtRecord, this);
	btn_scrollUp->connect(onScrollUpPressed, this, UIButton::PRESSED);
	btn_scrollDown->connect(onScrollDownPressed, this, UIButton::PRESSED);
	btn_scrollUp->connect(onScrollBtnReleased, this, UIButton::RELEASED);
	btn_scrollDown->connect(onScrollBtnReleased, this, UIButton::RELEASED);
	btn_sliderHandle->connect(onSliderPressed, this, UIButton::PRESSED);
	btn_sliderHandle->connect(onSliderReleased, this, UIButton::RELEASED);

	holdWait = -1;
	pageAdd = 0;
	draggingHandle = false;

	cbMS = gEngine->getSocket()->addMessageCallback("MS", onMessageMS, this);
	cbMC = gEngine->getSocket()->addMessageCallback("MC", onMessageMC, this);

	reloadScroll(true);
}

void UICourtICChatLog::updateInput()
{
	btn_back->updateInput();
	btn_courtRecord->updateInput();
	btn_scrollUp->updateInput();
	btn_scrollDown->updateInput();
	btn_sliderHandle->updateInput();
}

void UICourtICChatLog::update()
{
	if (pageAdd)
	{
		holdWait--;
		if (holdWait <= 0)
		{
			setScroll(scrollPos + pageAdd);

			if (scrollPos == 0 || scrollPos+10 >= pCourtUI->getICLog().size())
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

void UICourtICChatLog::setScroll(u32 i)
{
	scrollPos = i;
	reloadScroll();
}

void UICourtICChatLog::reloadScroll(bool all)
{
	int diff = scrollPos - scrollPosOld;
	if (!all && !diff)
	{
		setSliderHandle();
		if (pCourtUI->getICLog().size() <= 10)
		{
			for (u32 i=0; i<pCourtUI->getICLog().size(); i++)
			{
				lbl_log->setTextOnLine(pCourtUI->getICLog()[i]+u"\n", i);
				adx_update();
			}
		}
		return;
	}

	u32 diffAbs = abs(diff);
	int diffSign = (diff>0) ? 1 : (diff<0) ? -1 : 0;

	u32 start, end;
	int add;

	if (all || diffAbs >= 10)
	{
		start = 0;
		end = 10;
		add = 1;
	}
	else
	{
		// method for faster scrolling

		// first, move old ones
		start = (diffSign==1) ? 0 : 10-1;
		end = (diffSign==1) ? 10-diffAbs : diffAbs-1;
		add = diffSign;

		for (u32 i=start; i!=end; i+=add)
		{
			adx_update();

			dmaCopy(lbl_log->getGfx()[7*(i+diff)], lbl_log->getGfx()[7*i], 32*16*7);

			adx_update();
		}

		// then generate new text
		start = end;
		end = (diffSign==1) ? 10 : -1;
	}

	for (u32 i=start; i!=end; i+=add)
	{
		if (scrollPos+i >= pCourtUI->getICLog().size())
			break;
		lbl_log->setTextOnLine(pCourtUI->getICLog()[scrollPos+i]+u"\n", i);

		adx_update();
	}

	scrollPosOld = scrollPos;

	setSliderHandle();
	atBottom = (pCourtUI->getICLog().size() <= 10 || scrollPos == pCourtUI->getICLog().size()-10);
}

void UICourtICChatLog::setSliderHandle()
{
	int handleEdges[2] = {btn_scrollUp->getY()+btn_scrollUp->getH(), btn_scrollDown->getY()-btn_scrollDown->getH()};
	int yPos = handleEdges[0];
	if (pCourtUI->getICLog().size() > 10)
		yPos = handleEdges[0] + ((handleEdges[1] - handleEdges[0]) * (scrollPos / (float)(pCourtUI->getICLog().size()-10)));
	btn_sliderHandle->setPos(btn_sliderHandle->getX(), yPos);
}

void UICourtICChatLog::onBackClicked(void* pUserData)
{
	UICourtICChatLog* pSelf = (UICourtICChatLog*)pUserData;

	wav_play(pSelf->pCourtUI->sndCancel);
	pSelf->pCourtUI->changeScreen(new UICourtIngameMenu(pSelf->pCourtUI));
}

void UICourtICChatLog::onCourtRecord(void* pUserData)
{
	UICourtICChatLog* pSelf = (UICourtICChatLog*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->pCourtUI->changeScreen(new UICourtEvidence(pSelf->pCourtUI));
}

void UICourtICChatLog::onScrollUpPressed(void* pUserData)
{
	UICourtICChatLog* pSelf = (UICourtICChatLog*)pUserData;

	if (!pSelf->scrollPos) return;
	pSelf->setScroll(pSelf->scrollPos-1);

	pSelf->holdWait = 20;
	pSelf->pageAdd = -1;
}

void UICourtICChatLog::onScrollDownPressed(void* pUserData)
{
	UICourtICChatLog* pSelf = (UICourtICChatLog*)pUserData;

	if (pSelf->scrollPos+10 >= pSelf->pCourtUI->getICLog().size()) return;
	pSelf->setScroll(pSelf->scrollPos+1);

	pSelf->holdWait = 20;
	pSelf->pageAdd = 1;
}

void UICourtICChatLog::onScrollBtnReleased(void* pUserData)
{
	UICourtICChatLog* pSelf = (UICourtICChatLog*)pUserData;

	pSelf->pageAdd = 0;
}

void UICourtICChatLog::onSliderPressed(void* pUserData)
{
	UICourtICChatLog* pSelf = (UICourtICChatLog*)pUserData;

	pSelf->draggingHandle = true;
}

void UICourtICChatLog::onSliderReleased(void* pUserData)
{
	UICourtICChatLog* pSelf = (UICourtICChatLog*)pUserData;

	pSelf->draggingHandle = false;

	int handleEdges[2] = {pSelf->btn_scrollUp->getY()+pSelf->btn_scrollUp->getH(), pSelf->btn_scrollDown->getY()-pSelf->btn_scrollDown->getH()};
	u32 yPos = 0;
	if (pSelf->pCourtUI->getICLog().size() > 10)
		yPos = (pSelf->btn_sliderHandle->getY() - handleEdges[0]) / (float)(handleEdges[1]-handleEdges[0]) * (pSelf->pCourtUI->getICLog().size()-10);
	pSelf->setScroll(yPos);
}

void UICourtICChatLog::onMessageMS(void* pUserData, std::string msg)
{
	UICourtICChatLog* pSelf = (UICourtICChatLog*)pUserData;

	if (pSelf->atBottom)
		pSelf->setScroll((pSelf->pCourtUI->getICLog().size() > 10) ? pSelf->pCourtUI->getICLog().size()-10 : 0);
	else
		pSelf->setSliderHandle();
}

void UICourtICChatLog::onMessageMC(void* pUserData, std::string msg)
{
	UICourtICChatLog* pSelf = (UICourtICChatLog*)pUserData;

	if (pSelf->atBottom)
		pSelf->setScroll((pSelf->pCourtUI->getICLog().size() > 10) ? pSelf->pCourtUI->getICLog().size()-10 : 0);
	else
		pSelf->setSliderHandle();
}
