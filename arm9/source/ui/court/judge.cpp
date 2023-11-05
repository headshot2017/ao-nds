#include "ui/court/judge.h"

#include <string.h>

#include <nds/dma.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "engine.h"
#include "ui/court/ic.h"

UICourtJudge::~UICourtJudge()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete btn_back;
	delete btn_courtRecord;
	delete btn_barDefenseMinus;
	delete btn_barDefensePlus;
	delete btn_barProsecutorMinus;
	delete btn_barProsecutorPlus;
	delete btn_witnessTestimony;
	delete btn_crossExamination;
	delete btn_guilty;
	delete btn_notGuilty;
	for (int i=0; i<2; i++) delete spr_bars[i];

	gEngine->getSocket()->removeMessageCallback("HP", cbHP);
}

void UICourtJudge::init()
{
	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);

	u8* bgTiles = readFile("/data/ao-nds/ui/bg_judge.img.bin", &bgTilesLen);
	u8* bgMap = readFile("/data/ao-nds/ui/bg_judge.map.bin");
	u8* bgPal = readFile("/data/ao-nds/ui/bg_judge.pal.bin");

	dmaCopy(bgTiles, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), 1536);
	memcpy(BG_PALETTE_SUB, bgPal, 512);

	delete[] bgTiles;
	delete[] bgMap;
	delete[] bgPal;

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 0);
	btn_courtRecord = new UIButton(&oamSub, "/data/ao-nds/ui/spr_courtRecord", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-80, 0, 80, 32, 32, 32, 1);
	btn_barDefenseMinus = new UIButton(&oamSub, "/data/ao-nds/ui/spr_barDefenseMinus", btn_courtRecord->nextOamInd(), 1, 1, SpriteSize_16x16, 15, 141, 12, 12, 16, 16, 2);
	btn_barDefensePlus = new UIButton(&oamSub, "/data/ao-nds/ui/spr_barDefensePlus", btn_barDefenseMinus->nextOamInd(), 1, 1, SpriteSize_16x16, 15, 39, 12, 12, 16, 16, 3);
	btn_barProsecutorMinus = new UIButton(&oamSub, "/data/ao-nds/ui/spr_barProsecutorMinus", btn_barDefensePlus->nextOamInd(), 1, 1, SpriteSize_16x16, 230, 141, 12, 12, 16, 16, 4);
	btn_barProsecutorPlus = new UIButton(&oamSub, "/data/ao-nds/ui/spr_barProsecutorPlus", btn_barProsecutorMinus->nextOamInd(), 1, 1, SpriteSize_16x16, 230, 39, 12, 12, 16, 16, 5);
	btn_witnessTestimony = new UIButton(&oamSub, "/data/ao-nds/ui/spr_witTestBtn", btn_barProsecutorPlus->nextOamInd(), 3, 1, SpriteSize_32x64, 37, 49, 85, 42, 32, 64, 6);
	btn_crossExamination = new UIButton(&oamSub, "/data/ao-nds/ui/spr_crossExamBtn", btn_witnessTestimony->nextOamInd(), 3, 1, SpriteSize_32x64, 134, 49, 85, 42, 32, 64, 7);
	btn_guilty = new UIButton(&oamSub, "/data/ao-nds/ui/spr_guiltyBtn", btn_crossExamination->nextOamInd(), 3, 1, SpriteSize_32x64, 37, 102, 85, 42, 32, 64, 8);
	btn_notGuilty = new UIButton(&oamSub, "/data/ao-nds/ui/spr_notGuiltyBtn", btn_guilty->nextOamInd(), 3, 1, SpriteSize_32x64, 134, 102, 85, 42, 32, 64, 9);

	for (int i=0; i<2; i++)
	{
		int nextOam = (!i) ? btn_notGuilty->nextOamInd() : spr_bars[i-1]->nextOamInd();

		std::string file = (!i) ? "/data/ao-nds/ui/spr_barDefenseV" : "/data/ao-nds/ui/spr_barProsecutorV";
		spr_bars[i] = new UIButton(&oamSub, file, nextOam, 1, 3, SpriteSize_16x32, 14+(i*215), 54, 14, 84, 16, 32, 10+i);
	}

	btn_back->assignKey(KEY_B);
	btn_courtRecord->assignKey(KEY_R);

	btn_back->connect(onBackClicked, this);
	btn_courtRecord->connect(onCourtRecord, this);
	btn_barDefenseMinus->connect(onBarDefenseMinus, this);
	btn_barDefensePlus->connect(onBarDefensePlus, this);
	btn_barProsecutorMinus->connect(onBarProsecutorMinus, this);
	btn_barProsecutorPlus->connect(onBarProsecutorPlus, this);
	btn_witnessTestimony->connect(onWitnessTestimonyClicked, this);
	btn_crossExamination->connect(onCrossExaminationClicked, this);
	btn_guilty->connect(onGuiltyClicked, this);
	btn_notGuilty->connect(onNotGuiltyClicked, this);

	cbHP = gEngine->getSocket()->addMessageCallback("HP", onMessageHP, this);

	reloadBars();
}

void UICourtJudge::updateInput()
{
	btn_back->updateInput();
	btn_courtRecord->updateInput();
	btn_barDefenseMinus->updateInput();
	btn_barDefensePlus->updateInput();
	btn_barProsecutorMinus->updateInput();
	btn_barProsecutorPlus->updateInput();
	btn_witnessTestimony->updateInput();
	btn_crossExamination->updateInput();
	btn_guilty->updateInput();
	btn_notGuilty->updateInput();
}

void UICourtJudge::update()
{

}

void UICourtJudge::reloadBars()
{
	for (int i=0; i<2; i++)
		spr_bars[i]->setFrame(pCourtUI->bars[i]);
}

void UICourtJudge::onBackClicked(void* pUserData)
{
	UICourtJudge* pSelf = (UICourtJudge*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndCancel, SoundFormat_16Bit, pSelf->pCourtUI->sndCancelSize, 32000, 127, 64, false, 0);

	pSelf->pCourtUI->changeScreen(new UICourtIC(pSelf->pCourtUI));
}

void UICourtJudge::onCourtRecord(void* pUserData)
{
	UICourtJudge* pSelf = (UICourtJudge*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndCrtRcrd, SoundFormat_16Bit, pSelf->pCourtUI->sndCrtRcrdSize, 32000, 127, 64, false, 0);
}

void UICourtJudge::onBarDefenseMinus(void* pUserData)
{
	UICourtJudge* pSelf = (UICourtJudge*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndEvPage, SoundFormat_16Bit, pSelf->pCourtUI->sndEvPageSize, 32000, 127, 64, false, 0);

	if (pSelf->pCourtUI->bars[0] <= 0) return;
	gEngine->getSocket()->sendData("HP#1#" + std::to_string(pSelf->pCourtUI->bars[0]-1) + "#%");
}

void UICourtJudge::onBarDefensePlus(void* pUserData)
{
	UICourtJudge* pSelf = (UICourtJudge*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndEvPage, SoundFormat_16Bit, pSelf->pCourtUI->sndEvPageSize, 32000, 127, 64, false, 0);

	if (pSelf->pCourtUI->bars[0] >= 10) return;
	gEngine->getSocket()->sendData("HP#1#" + std::to_string(pSelf->pCourtUI->bars[0]+1) + "#%");
}

void UICourtJudge::onBarProsecutorMinus(void* pUserData)
{
	UICourtJudge* pSelf = (UICourtJudge*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndEvPage, SoundFormat_16Bit, pSelf->pCourtUI->sndEvPageSize, 32000, 127, 64, false, 0);

	if (pSelf->pCourtUI->bars[1] <= 0) return;
	gEngine->getSocket()->sendData("HP#2#" + std::to_string(pSelf->pCourtUI->bars[1]-1) + "#%");
}

void UICourtJudge::onBarProsecutorPlus(void* pUserData)
{
	UICourtJudge* pSelf = (UICourtJudge*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndEvPage, SoundFormat_16Bit, pSelf->pCourtUI->sndEvPageSize, 32000, 127, 64, false, 0);

	if (pSelf->pCourtUI->bars[1] >= 10) return;
	gEngine->getSocket()->sendData("HP#2#" + std::to_string(pSelf->pCourtUI->bars[1]+1) + "#%");
}

void UICourtJudge::onWitnessTestimonyClicked(void* pUserData)
{
	gEngine->getSocket()->sendData("RT#testimony1#%");
}

void UICourtJudge::onCrossExaminationClicked(void* pUserData)
{
	gEngine->getSocket()->sendData("RT#testimony2#%");
}

void UICourtJudge::onGuiltyClicked(void* pUserData)
{
	gEngine->getSocket()->sendData("RT#judgeruling#1#%");
}

void UICourtJudge::onNotGuiltyClicked(void* pUserData)
{
	gEngine->getSocket()->sendData("RT#judgeruling#0#%");
}

void UICourtJudge::onMessageHP(void* pUserData, std::string msg)
{
	UICourtJudge* pSelf = (UICourtJudge*)pUserData;

	pSelf->reloadBars();
}
