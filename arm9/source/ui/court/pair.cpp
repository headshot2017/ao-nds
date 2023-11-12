#include "ui/court/pair.h"

#include <string.h>

#include <nds/dma.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sound.h>

#include "ui/court/ic.h"
#include "ui/court/pairselect.h"

UICourtPair::~UICourtPair()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete btn_back;
	delete btn_disable;
	delete btn_charSelect;
	delete btn_reset;
	delete btn_xSlider;
	delete btn_ySlider;
	delete lbl_xValue;
	delete lbl_yValue;
	delete lbl_pairName;
	delete spr_previewBg;
	delete spr_previewChar;
	delete spr_previewDesk;
}

void UICourtPair::init()
{
	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_pair", true);

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 0);
	btn_disable = new UIButton(&oamSub, "/data/ao-nds/ui/spr_disable", btn_back->nextOamInd(), 1, 1, SpriteSize_64x32, 104, 170, 48, 21, 64, 32, 1);
	btn_charSelect = new UIButton(&oamSub, "/data/ao-nds/ui/spr_charSelect", btn_disable->nextOamInd(), 3, 1, SpriteSize_32x32, 256-79, 192-30, 79, 30, 32, 32, 2);
	btn_reset = new UIButton(&oamSub, "/data/ao-nds/ui/spr_reset", btn_charSelect->nextOamInd(), 2, 1, SpriteSize_32x16, 208, 0, 48, 16, 32, 16, 3);
	btn_xSlider = new UIButton(&oamSub, "/data/ao-nds/ui/spr_sliderHandleHor", btn_reset->nextOamInd(), 1, 1, SpriteSize_32x16, 118, 142, 19, 14, 32, 16, 4);
	btn_ySlider = new UIButton(&oamSub, "/data/ao-nds/ui/spr_sliderHandle", btn_xSlider->nextOamInd(), 1, 1, SpriteSize_16x32, 229, 66, 14, 19, 16, 32, 5);

	lbl_xValue = new UILabel(&oamSub, btn_ySlider->nextOamInd(), 2, 1, RGB15(31,31,31), 6, 0);
	lbl_yValue = new UILabel(&oamSub, lbl_xValue->nextOamInd(), 2, 1, RGB15(31,31,31), 6, 0);
	lbl_pairName = new UILabel(&oamSub, lbl_yValue->nextOamInd(), 6, 1, RGB15(31,31,31), 6, 0);

	spr_previewBg = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pairPreviewBg", lbl_pairName->nextOamInd(), 5, 2, SpriteSize_32x64, 48, 16, 160, 128, 32, 64, 7);
	spr_previewChar = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pairPreviewChar", spr_previewBg->nextOamInd(), 3, 2, SpriteSize_32x64, 87, 17, 84, 119, 32, 64, 8);
	spr_previewDesk = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pairPreviewDesk", spr_previewChar->nextOamInd(), 2, 1, SpriteSize_64x32, 72, 115, 112, 21, 64, 32, 9);

	spr_previewBg->setPriority(3);
	spr_previewChar->setPriority(2);
	spr_previewDesk->setPriority(1);

	btn_disable->setVisible(pCourtUI->icControls.pairID != -1);

	lbl_pairName->setText((pCourtUI->icControls.pairID != -1) ? pCourtUI->getCharList()[pCourtUI->icControls.pairID].name.c_str() : "No pair");
	lbl_pairName->setPos(128, 156, true);

	btn_back->assignKey(KEY_B);
	btn_disable->assignKey(KEY_X);
	btn_charSelect->assignKey(KEY_A);
	btn_reset->assignKey(KEY_R);

	btn_back->connect(onBackClicked, this);
	btn_disable->connect(onDisableClicked, this);
	btn_charSelect->connect(onCharSelectClicked, this);
	btn_reset->connect(onResetClicked, this);
	btn_xSlider->connect(onXSliderPressed, this);
	btn_ySlider->connect(onYSliderPressed, this);
	btn_xSlider->connect(onSliderReleased, this, UIButton::RELEASED);
	btn_ySlider->connect(onSliderReleased, this, UIButton::RELEASED);

	draggingHandle = 0;

	updateOffset();
}

void UICourtPair::updateInput()
{
	btn_back->updateInput();
	btn_disable->updateInput();
	btn_charSelect->updateInput();
	btn_reset->updateInput();
	btn_xSlider->updateInput();
	btn_ySlider->updateInput();

	u32 key = keysDown();
	if (key & KEY_LEFT && pCourtUI->icControls.xOffset > -100)
	{
		pCourtUI->icControls.xOffset--;
		updateOffset();
	}
	else if (key & KEY_RIGHT && pCourtUI->icControls.xOffset < 100)
	{
		pCourtUI->icControls.xOffset++;
		updateOffset();
	}
	else if (key & KEY_UP && pCourtUI->icControls.yOffset > -100)
	{
		pCourtUI->icControls.yOffset--;
		updateOffset();
	}
	else if (key & KEY_DOWN && pCourtUI->icControls.yOffset < 100)
	{
		pCourtUI->icControls.yOffset++;
		updateOffset();
	}

	if (draggingHandle)
	{
		touchPosition touchPos;
		touchRead(&touchPos);
		int px = touchPos.px;
		int py = touchPos.py;

		int handleEdges[2];
		int xPos = draggingHandle->getX();
		int yPos = draggingHandle->getY();
		int charX = 87 + (pCourtUI->icControls.xOffset / 100.f * 160);
		int charY = 17 + (pCourtUI->icControls.yOffset / 100.f * 120);

		if (draggingHandle == btn_xSlider)
		{
			handleEdges[0] = 37;
			handleEdges[1] = 200;
			px -= 9;
			xPos = (px < handleEdges[0]) ? handleEdges[0] : (px > handleEdges[1]) ? handleEdges[1] : px;

			int predX = (xPos - handleEdges[0]) / (float)(handleEdges[1]-handleEdges[0]) * 201 - 100;
			charX = 87 + (predX / 100.f * 160);
		}
		else
		{
			handleEdges[0] = 17;
			handleEdges[1] = 116;
			py -= 9;
			yPos = (py < handleEdges[0]) ? handleEdges[0] : (py > handleEdges[1]) ? handleEdges[1] : py;

			int predY = (draggingHandle->getY() - handleEdges[0]) / (float)(handleEdges[1]-handleEdges[0]) * 200.5f - 100;
			charY = 17 + (predY / 100.f * 120);
		}

		draggingHandle->setPos(xPos, yPos);
		spr_previewChar->setPos(charX, charY);
	}
}

void UICourtPair::update()
{

}

void UICourtPair::updateOffset()
{
	std::string xOffset = std::to_string(pCourtUI->icControls.xOffset) + "%";
	std::string yOffset = std::to_string(pCourtUI->icControls.yOffset) + "%";
	lbl_xValue->setText(xOffset.c_str());
	lbl_xValue->setPos(220, 143, false);
	lbl_yValue->setText(yOffset.c_str());
	lbl_yValue->setPos(235, 134, true);

	int xPos = 87 + (pCourtUI->icControls.xOffset / 100.f * 160);
	int yPos = 17 + (pCourtUI->icControls.yOffset / 100.f * 120);
	spr_previewChar->setPos(xPos, yPos);

	int handleEdgesX[2] = {37, 200};
	int handleEdgesY[2] = {17, 116};
	int sliderX = handleEdgesX[0] + ((handleEdgesX[1] - handleEdgesX[0]) * ((100+pCourtUI->icControls.xOffset) / 200.f));
	int sliderY = handleEdgesY[0] + ((handleEdgesY[1] - handleEdgesY[0]) * ((100+pCourtUI->icControls.yOffset) / 199.5f));
	btn_xSlider->setPos(sliderX, btn_xSlider->getY());
	btn_ySlider->setPos(btn_ySlider->getX(), sliderY);
}

void UICourtPair::onBackClicked(void* pUserData)
{
	UICourtPair* pSelf = (UICourtPair*)pUserData;
	wav_play(pSelf->pCourtUI->sndCancel);

	pSelf->pCourtUI->changeScreen(new UICourtIC(pSelf->pCourtUI));
}

void UICourtPair::onDisableClicked(void* pUserData)
{
	UICourtPair* pSelf = (UICourtPair*)pUserData;
	wav_play(pSelf->pCourtUI->sndCancel);

	pSelf->pCourtUI->icControls.pairID = -1;
	pSelf->btn_disable->setVisible(false);

	pSelf->lbl_pairName->setText("No pair");
	pSelf->lbl_pairName->setPos(128, 156, true);
}

void UICourtPair::onCharSelectClicked(void* pUserData)
{
	UICourtPair* pSelf = (UICourtPair*)pUserData;
	wav_play(pSelf->pCourtUI->sndCrtRcrd);

	pSelf->pCourtUI->changeScreen(new UICourtPairSelect(pSelf->pCourtUI));
}

void UICourtPair::onResetClicked(void* pUserData)
{
	UICourtPair* pSelf = (UICourtPair*)pUserData;
	wav_play(pSelf->pCourtUI->sndCancel);

	pSelf->pCourtUI->icControls.xOffset = 0;
	pSelf->pCourtUI->icControls.yOffset = 0;
	pSelf->btn_xSlider->setPos(118, 142);
	pSelf->btn_ySlider->setPos(229, 66);
	pSelf->updateOffset();
}

void UICourtPair::onXSliderPressed(void* pUserData)
{
	UICourtPair* pSelf = (UICourtPair*)pUserData;

	pSelf->draggingHandle = pSelf->btn_xSlider;
}

void UICourtPair::onYSliderPressed(void* pUserData)
{
	UICourtPair* pSelf = (UICourtPair*)pUserData;

	pSelf->draggingHandle = pSelf->btn_ySlider;
}

void UICourtPair::onSliderReleased(void* pUserData)
{
	UICourtPair* pSelf = (UICourtPair*)pUserData;

	int handleEdges[2];

	if (pSelf->draggingHandle == pSelf->btn_xSlider)
	{
		handleEdges[0] = 37;
		handleEdges[1] = 200;
		pSelf->pCourtUI->icControls.xOffset = (pSelf->draggingHandle->getX() - handleEdges[0]) / (float)(handleEdges[1]-handleEdges[0]) * 201 - 100;
	}
	else
	{
		handleEdges[0] = 17;
		handleEdges[1] = 116;
		pSelf->pCourtUI->icControls.yOffset = (pSelf->draggingHandle->getY() - handleEdges[0]) / (float)(handleEdges[1]-handleEdges[0]) * 200.5f - 100;
	}

	pSelf->draggingHandle = 0;
	pSelf->updateOffset();
}
