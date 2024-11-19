#include "ui/court/evidencedetail.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "utf8.h"
#include "mp3_shared.h"
#include "engine.h"
#include "fonts.h"
#include "content.h"
#include "ui/court/courtrecord.h"
#include "ui/court/evidenceimage.h"
#include "ui/court/icchatlog.h"
#include "ui/court/profiledetail.h"

UICourtEvidenceDetail::~UICourtEvidenceDetail()
{
	delete btn_back;
	delete btn_privatePublic;
	delete btn_profilesEvidence;
	delete btn_topButton;
	delete btn_delete;
	delete btn_transfer;
	delete btn_descUp;
	delete btn_descDown;
	delete btn_prevPage;
	delete btn_nextPage;
	delete lbl_name;
	delete lbl_imageName;
	delete lbl_desc;
	delete spr_evidence;
	delete kb_input;

	gEngine->getSocket()->removeMessageCallback("LE", cbLE);
}

void UICourtEvidenceDetail::init()
{
	adding = (currEvidence == pCourtUI->getEvidenceList(isPrivate).size());

	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_evidenceDetails");

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-32, 80, 32, 32, 32, 0);
	btn_privatePublic = new UIButton(&oamSub, "/data/ao-nds/ui/spr_privatePublic2", btn_back->nextOamInd(), 2, 1, SpriteSize_32x16, 128-(64/2), btn_back->getY()-2, 64, 15, 32, 16, 1);
	btn_profilesEvidence = new UIButton(&oamSub, "/data/ao-nds/ui/spr_profilesEvidence", btn_privatePublic->nextOamInd(), 1, 1, SpriteSize_64x32, 256-55, 0, 55, 31, 64, 32, 2);
	btn_topButton = new UIButton(&oamSub, (adding ? "/data/ao-nds/ui/spr_addTop" : "/data/ao-nds/ui/spr_present"), btn_profilesEvidence->nextOamInd(), 3, 1, SpriteSize_32x32, 128-(85/2), 0, 85, 32, 32, 32, 3);
	btn_delete = new UIButton(&oamSub, "/data/ao-nds/ui/spr_deleteEvidence", btn_topButton->nextOamInd(), 1, 1, SpriteSize_32x32, 91, 84, 20, 20, 32, 32, 4);
	btn_transfer = new UIButton(&oamSub, "/data/ao-nds/ui/spr_transferEvidence", btn_delete->nextOamInd(), 1, 1, SpriteSize_32x32, 215, 84, 20, 20, 32, 32, 5);
	btn_descUp = new UIButton(&oamSub, "/data/ao-nds/ui/spr_scrollUp", btn_transfer->nextOamInd(), 1, 1, SpriteSize_16x32, 233, 115, 14, 19, 16, 32, 6);
	btn_descDown = new UIButton(&oamSub, "/data/ao-nds/ui/spr_scrollDown", btn_descUp->nextOamInd(), 1, 1, SpriteSize_16x32, 233, 139, 14, 19, 16, 32, 7);
	btn_prevPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageLeft_medium", btn_descDown->nextOamInd(), 1, 2, SpriteSize_16x32, 0, 40, 16, 63, 16, 32, 8);
	btn_nextPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_pageRight_medium", btn_prevPage->nextOamInd(), 1, 2, SpriteSize_16x32, 256-16, 40, 16, 63, 16, 32, 9);

	lbl_name = new UILabel(&oamSub, btn_nextPage->nextOamInd(), 4, 1, RGB15(31, 16, 0), 10, 0);
	lbl_imageName = new UILabel(&oamSub, lbl_name->nextOamInd(), 4, 2, RGB15(4, 4, 4), 11, 0);
	lbl_desc = new UILabel(&oamSub, lbl_imageName->nextOamInd(), 7, 4, RGB15(31,31,31), 12, 0);
	lbl_desc->setLineOffset(11);
	lbl_desc->setPos(10, 113);

	spr_evidence = new UIButton(&oamSub, "", lbl_desc->nextOamInd(), 1, 1, SpriteSize_64x64, 21, 40, 68, 68, 64, 64, 13);

	kb_input = new AOkeyboard(4, spr_evidence->nextOamInd(), 14);
	mp3_fill_buffer();

	inputting = 0;

	if (!adding)
	{
		btn_privatePublic->setFrame(isPrivate ? 1 : 0);
		btn_transfer->setFrame(isPrivate ? 1 : 0);

		if (pCourtUI->getEvidenceList(!isPrivate).empty())
			btn_privatePublic->setVisible(false); // if we're on public evidence list and private list is empty (or viceversa), don't let the user switch

		if (isPrivate)
			btn_topButton->setVisible(false);
		if (pCourtUI->icControls.evidence > -1)
			btn_topButton->setFrame(1);
	}
	else
	{
		btn_privatePublic->setVisible(false);
	}

	btn_back->assignKey(KEY_B);
	btn_profilesEvidence->assignKey(KEY_R);
	btn_topButton->assignKey(KEY_X);
	btn_descUp->assignKey(KEY_UP);
	btn_descDown->assignKey(KEY_DOWN);
	btn_prevPage->assignKey(KEY_LEFT);
	btn_nextPage->assignKey(KEY_RIGHT);

	btn_back->connect(onBackClicked, this);
	btn_privatePublic->connect(onPrivatePublicClicked, this);
	btn_profilesEvidence->connect(onProfilesEvidenceClicked, this);
	btn_topButton->connect(onTopButtonClicked, this);
	btn_delete->connect(onDeleteClicked, this);
	btn_transfer->connect(onTransferClicked, this);
	btn_descUp->connect(onDescUp, this);
	btn_descDown->connect(onDescDown, this);
	btn_prevPage->connect(onPrevPage, this);
	btn_nextPage->connect(onNextPage, this);
	spr_evidence->connect(onEvidenceImage, this);

	reloadPage();

	cbLE = gEngine->getSocket()->addMessageCallback("LE", onMessageLE, this);
}

void UICourtEvidenceDetail::updateInput()
{
	if (kb_input->isVisible())
	{
		int result = kb_input->updateInput();
		if (result != 0)
		{
			dmaCopy(bgPal, BG_PALETTE_SUB, 512);
			showEverything();

			if (result > 0)
			{
				if (inputting == lbl_name)
					currName = kb_input->getValue();
				else
					currDesc = kb_input->getValue();

				if (!adding)
				{
					// editing evidence
					if (!isPrivate)
						gEngine->getSocket()->sendData("EE#" + std::to_string(currEvidence) + "#" + utf8::utf16to8(currName) + "#" + utf8::utf16to8(currDesc) + "#" + currImage + ".png#%");
					else
					{
						Settings::privateEvidence[currEvidence].name = currName;
						Settings::privateEvidence[currEvidence].description = currDesc;
						Settings::privateEvidence[currEvidence].image = currImage;
						Settings::savePrivateEvidence();
					}
				}
				else
					reloadPage();
			}
		}
		return;
	}

	btn_back->updateInput();
	btn_privatePublic->updateInput();
	btn_profilesEvidence->updateInput();
	btn_topButton->updateInput();
	btn_delete->updateInput();
	btn_transfer->updateInput();
	btn_descUp->updateInput();
	btn_descDown->updateInput();
	btn_prevPage->updateInput();
	btn_nextPage->updateInput();
	spr_evidence->updateInput();

	u32 key = keysDown();
	if (key & KEY_Y)
	{
		wav_play(pCourtUI->sndCrtRcrd);
		pCourtUI->changeScreen(new UICourtICChatLog(pCourtUI));
	}

	if (key & KEY_TOUCH)
	{
		touchPosition pos;
		touchRead(&pos);

		if (pos.px >= 91 && pos.py >= 40 && pos.px < 91+144 && pos.py < 40+17)
		{
			wav_play(pCourtUI->sndCrtRcrd);
			inputting = lbl_name;
			hideEverything();
			kb_input->show16("Enter evidence name", currName);
		}
		else if (pos.px >= 9 && pos.py >= 114 && pos.px < 9+223 && pos.py < 114+45)
		{
			wav_play(pCourtUI->sndCrtRcrd);
			inputting = lbl_desc;
			hideEverything();
			kb_input->show16("Enter evidence description", currDesc);
		}
	}
}

void UICourtEvidenceDetail::update()
{

}

void UICourtEvidenceDetail::hideEverything()
{
	bgHide(bgIndex);

	btn_back->setVisible(false);
	btn_privatePublic->setVisible(false);
	btn_profilesEvidence->setVisible(false);
	btn_topButton->setVisible(false);
	btn_delete->setVisible(false);
	btn_transfer->setVisible(false);
	btn_descUp->setVisible(false);
	btn_descDown->setVisible(false);
	btn_prevPage->setVisible(false);
	btn_nextPage->setVisible(false);
	lbl_name->setVisible(false);
	lbl_imageName->setVisible(false);
	lbl_desc->setVisible(false);
	spr_evidence->setVisible(false);
}

void UICourtEvidenceDetail::showEverything()
{
	bgShow(bgIndex);

	btn_back->setVisible(true);
	btn_descUp->setVisible(true);
	btn_descDown->setVisible(true);
	lbl_name->setVisible(true);
	lbl_imageName->setVisible(true);
	lbl_desc->setVisible(true);
	spr_evidence->setVisible(true);

	if (!adding)
	{
		btn_prevPage->setVisible(true);
		btn_nextPage->setVisible(true);
		btn_privatePublic->setVisible(true);
		btn_profilesEvidence->setVisible(true);
		btn_delete->setVisible(true);
		btn_transfer->setVisible(true);

		if (!isPrivate)
			btn_topButton->setVisible(true);
	}
	else
		btn_topButton->setVisible(true);
}

void UICourtEvidenceDetail::reloadPage()
{
	if (!adding)
	{
		const evidenceInfo& info = (currEvidence >= pCourtUI->getEvidenceList(isPrivate).size()) ? evidenceInfo() : pCourtUI->getEvidenceList(isPrivate)[currEvidence];
		currName = info.name;
		currDesc = info.description;
		currImage = info.image;

		btn_transfer->setVisible(true);
	}
	else
	{
		btn_prevPage->setVisible(false);
		btn_nextPage->setVisible(false);
		btn_delete->setVisible(false);
		btn_transfer->setVisible(false);
	}

	lbl_name->setText(currName);
	lbl_name->setPos(163, 41, true);
	lbl_imageName->setText(currImage);
	lbl_imageName->setPos(163, 57, true);

	scrollPos = 0;
	scrollPosOld = 0;
	renderDesc.clear();
	separateLines(0, currDesc, 7, false, renderDesc);
	reloadDesc(true);

	if (!adding)
	{
		btn_descUp->setVisible(false);
		btn_descDown->setVisible(renderDesc.size() > 4);
	}

	std::string file = "evidence/large/" + currImage;
	bool exists = Content::exists(file+".img.bin", file);
	if (exists) file = file.substr(0, file.length()-8); // remove extension

	spr_evidence->setImage(file, 64, 64, 13);
}

void UICourtEvidenceDetail::setScroll(u32 i)
{
	scrollPos = i;
	reloadDesc();
}

void UICourtEvidenceDetail::reloadDesc(bool all)
{
	int diff = scrollPos - scrollPosOld;
	if (!all && !diff) return;

	u32 diffAbs = abs(diff);
	int diffSign = (diff>0) ? 1 : (diff<0) ? -1 : 0;

	u32 start, end;
	int add;

	if (all || diffAbs >= 4)
	{
		lbl_desc->setText("");
		start = 0;
		end = 4;
		add = 1;
	}
	else
	{
		// method for faster scrolling

		// first, move old ones
		start = (diffSign==1) ? 0 : 4-1;
		end = (diffSign==1) ? 4-diffAbs : diffAbs-1;
		add = diffSign;

		for (u32 i=start; i!=end; i+=add)
		{
			mp3_fill_buffer();

			dmaCopy(lbl_desc->getGfx()[7*(i+diff)], lbl_desc->getGfx()[7*i], 32*16*7);

			mp3_fill_buffer();
		}

		// then generate new text
		start = end;
		end = (diffSign==1) ? 4 : -1;
	}

	for (u32 i=start; i!=end; i+=add)
	{
		if (scrollPos+i >= renderDesc.size())
			break;
		lbl_desc->setTextOnLine(renderDesc[scrollPos+i]+u"\n", i);

		mp3_fill_buffer();
	}

	scrollPosOld = scrollPos;
}

void UICourtEvidenceDetail::onBackClicked(void* pUserData)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndCancel);
	pSelf->pCourtUI->changeScreen(new UICourtEvidence(pSelf->pCourtUI));
}

void UICourtEvidenceDetail::onPrivatePublicClicked(void* pUserData)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);

	pSelf->isPrivate = !pSelf->isPrivate;

	pSelf->btn_privatePublic->setFrame(pSelf->isPrivate ? 1 : 0);
	pSelf->btn_transfer->setFrame(pSelf->isPrivate ? 1 : 0);
	pSelf->btn_topButton->setVisible(!pSelf->isPrivate);

	pSelf->currEvidence = 0;
	pSelf->reloadPage();
}

void UICourtEvidenceDetail::onProfilesEvidenceClicked(void* pUserData)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);

	pSelf->pCourtUI->changeScreen(new UICourtProfileDetail(pSelf->pCourtUI, 0, pSelf->isPrivate));
}

void UICourtEvidenceDetail::onTopButtonClicked(void* pUserData)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndSelect);
	if (!pSelf->adding)
	{
		if (pSelf->pCourtUI->icControls.evidence > -1)
		{
			pSelf->pCourtUI->icControls.evidence = -1;
			pSelf->btn_topButton->setFrame(0);
		}
		else
		{
			pSelf->pCourtUI->icControls.evidence = pSelf->currEvidence;
			pSelf->btn_topButton->setFrame(1);
		}
	}
	else
	{
		if (!pSelf->isPrivate)
			gEngine->getSocket()->sendData("PE#" + utf8::utf16to8(pSelf->currName) + "#" + utf8::utf16to8(pSelf->currDesc) + "#" + pSelf->currImage + ".png#%");
		else
		{
			Settings::privateEvidence.push_back({pSelf->currName, pSelf->currDesc, pSelf->currImage});
			Settings::savePrivateEvidence();
		}
		pSelf->pCourtUI->changeScreen(new UICourtEvidence(pSelf->pCourtUI));
	}
}

void UICourtEvidenceDetail::onDeleteClicked(void* pUserData)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndSelect);

	if (!pSelf->isPrivate)
		gEngine->getSocket()->sendData("DE#" + std::to_string(pSelf->currEvidence) + "#%");
	else
	{
		Settings::privateEvidence.erase(Settings::privateEvidence.begin() + pSelf->currEvidence);
		Settings::savePrivateEvidence();
	}

	pSelf->pCourtUI->changeScreen(new UICourtEvidence(pSelf->pCourtUI));
}

void UICourtEvidenceDetail::onTransferClicked(void* pUserData)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndSelect);
	pSelf->btn_transfer->setVisible(false);

	const evidenceInfo& info = pSelf->pCourtUI->getEvidenceList(pSelf->isPrivate)[pSelf->currEvidence];
	if (pSelf->isPrivate)
	{
		// copy to public
		gEngine->getSocket()->sendData("PE#" + utf8::utf16to8(info.name) + "#" + utf8::utf16to8(info.description) + "#" + info.image + ".png#%");
	}
	else
	{
		// copy to private
		Settings::privateEvidence.push_back(info);
		Settings::savePrivateEvidence();
	}
}

void UICourtEvidenceDetail::onDescUp(void* pUserData)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	if (!pSelf->scrollPos) return;
	pSelf->setScroll(pSelf->scrollPos-1);

	pSelf->btn_descDown->setVisible(true);
	if (!pSelf->scrollPos)
		pSelf->btn_descUp->setVisible(false);
}

void UICourtEvidenceDetail::onDescDown(void* pUserData)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	if (pSelf->scrollPos+4 >= pSelf->renderDesc.size()) return;
	pSelf->setScroll(pSelf->scrollPos+1);

	pSelf->btn_descUp->setVisible(true);
	if (pSelf->scrollPos+4 >= pSelf->renderDesc.size())
		pSelf->btn_descDown->setVisible(false);
}

void UICourtEvidenceDetail::onPrevPage(void* pUserData)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndEvPage);
	if (pSelf->currEvidence == 0) pSelf->currEvidence = pSelf->pCourtUI->getEvidenceList(pSelf->isPrivate).size();
	pSelf->currEvidence--;

	pSelf->reloadPage();
}

void UICourtEvidenceDetail::onNextPage(void* pUserData)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndEvPage);
	pSelf->currEvidence++;
	if (pSelf->currEvidence >= pSelf->pCourtUI->getEvidenceList(pSelf->isPrivate).size()) pSelf->currEvidence = 0;

	pSelf->reloadPage();
}

void UICourtEvidenceDetail::onEvidenceImage(void* pUserData)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->pCourtUI->changeScreen(new UICourtEvidenceImage(pSelf->pCourtUI, pSelf->currEvidence, pSelf->isPrivate, pSelf->currName, pSelf->currDesc, pSelf->currImage));
}

void UICourtEvidenceDetail::onMessageLE(void* pUserData, std::string msg)
{
	UICourtEvidenceDetail* pSelf = (UICourtEvidenceDetail*)pUserData;

	if (pSelf->isPrivate) return;

	if (pSelf->pCourtUI->getEvidenceList(false).empty())
	{
		pSelf->pCourtUI->changeScreen(new UICourtEvidence(pSelf->pCourtUI));
		return;
	}

	while (pSelf->currEvidence >= pSelf->pCourtUI->getEvidenceList(false).size())
		pSelf->currEvidence--;

	pSelf->reloadPage();
}
