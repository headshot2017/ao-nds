#include "ui/court/moderatordialog.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>
#include <nds/arm9/sound.h>

#include "utf8.h"
#include "mp3_shared.h"
#include "engine.h"
#include "content.h"
#include "ui/court/courtrecord.h"
#include "ui/court/profiledetail.h"
#include "ui/court/message.h"

UICourtModeratorDialog::~UICourtModeratorDialog()
{
	delete btn_back;
	delete btn_confirm;
	delete lbl_name;
	delete lbl_desc;
	delete spr_profile;
	delete lbl_duration;
	delete lbl_days;
	delete lbl_daysValue;
	delete btn_daysInput;
	delete lbl_hours;
	delete lbl_hoursValue;
	delete btn_hoursInput;
	delete lbl_minutes;
	delete lbl_minutesValue;
	delete btn_minutesInput;
	delete lbl_perma;
	delete btn_perma;
	delete lbl_reason;
	delete lbl_reasonValue;
	delete btn_reasonInput;
	delete kb_input;
}

void UICourtModeratorDialog::init()
{
	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg((isBan) ? "/data/ao-nds/ui/bg_ban" : "/data/ao-nds/ui/bg_kick");
	durationsChanged = false;

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_back", 0, 3, 1, SpriteSize_32x32, 0, 192-32, 80, 32, 32, 32, 0);
	btn_confirm = new UIButton(&oamSub, "/data/ao-nds/ui/spr_confirm", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-82, 192-32, 82, 32, 32, 32, 1);

	lbl_name = new UILabel(&oamSub, btn_confirm->nextOamInd(), 4, 1, RGB15(31, 16, 0), 2, 0);
	lbl_desc = new UILabel(&oamSub, lbl_name->nextOamInd(), 4, 4, RGB15(4, 4, 4), 3, 0);

	spr_profile = new UIButton(&oamSub, "", lbl_desc->nextOamInd(), 1, 1, SpriteSize_64x64, 23, 28, 60, 60, 64, 64, 4);

	// duration (ban only)
	lbl_duration = new UILabel(&oamSub, spr_profile->nextOamInd(), 2, 1, RGB15(31,31,31), 5, 0);
	lbl_days = new UILabel(&oamSub, lbl_duration->nextOamInd(), 2, 1, RGB15(31,31,31), 5, 0);
	lbl_daysValue = new UILabel(&oamSub, lbl_days->nextOamInd(), 1, 1, RGB15(4, 4, 4), 3, 0);
	btn_daysInput = new UIButton(&oamSub, "/data/ao-nds/ui/spr_inputBoxShort", lbl_daysValue->nextOamInd(), 1, 1, SpriteSize_32x16, 0, 0, 32, 16, 32, 16, 6);
	lbl_hours = new UILabel(&oamSub, btn_daysInput->nextOamInd(), 2, 1, RGB15(31,31,31), 5, 0);
	lbl_hoursValue = new UILabel(&oamSub, lbl_hours->nextOamInd(), 1, 1, RGB15(4, 4, 4), 3, 0);
	btn_hoursInput = new UIButton(&oamSub, "/data/ao-nds/ui/spr_inputBoxShort", lbl_hoursValue->nextOamInd(), 1, 1, SpriteSize_32x16, 0, 0, 32, 16, 32, 16, 6);
	lbl_minutes = new UILabel(&oamSub, btn_hoursInput->nextOamInd(), 2, 1, RGB15(31,31,31), 5, 0);
	lbl_minutesValue = new UILabel(&oamSub, lbl_minutes->nextOamInd(), 1, 1, RGB15(4, 4, 4), 3, 0);
	btn_minutesInput = new UIButton(&oamSub, "/data/ao-nds/ui/spr_inputBoxShort", lbl_minutesValue->nextOamInd(), 1, 1, SpriteSize_32x16, 0, 0, 32, 16, 32, 16, 6);
	lbl_perma = new UILabel(&oamSub, btn_minutesInput->nextOamInd(), 3, 1, RGB15(31,31,31), 5, 0);
	btn_perma = new UIButton(&oamSub, "/data/ao-nds/ui/spr_checkBox", lbl_perma->nextOamInd(), 1, 1, SpriteSize_16x16, 128-96, 96-16, 16, 16, 16, 16, 7);

	lbl_duration->setPos(10, 103);
	lbl_days->setPos(44, 120);
	lbl_hours->setPos(129, 120);
	lbl_minutes->setPos(214, 120);
	btn_daysInput->setPos(lbl_days->getX()-34, lbl_days->getY()-1);
	btn_hoursInput->setPos(lbl_hours->getX()-34, lbl_hours->getY()-1);
	btn_minutesInput->setPos(lbl_minutes->getX()-34, lbl_minutes->getY()-1);
	lbl_daysValue->setPos(btn_daysInput->getX()+3, btn_daysInput->getY()+1);
	lbl_hoursValue->setPos(btn_hoursInput->getX()+3, btn_hoursInput->getY()+1);
	lbl_minutesValue->setPos(btn_minutesInput->getX()+3, btn_minutesInput->getY()+1);
	btn_perma->setPos(180, 100);
	lbl_perma->setPos(btn_perma->getX()+btn_perma->getW()+3, btn_perma->getY()+2, false);
	lbl_duration->setText("Duration:");
	lbl_days->setText("days");
	lbl_hours->setText("hours");
	lbl_minutes->setText("minutes");
	lbl_daysValue->setText("0");
	lbl_hoursValue->setText("0");
	lbl_minutesValue->setText("0");
	lbl_perma->setText("Permanent");

	// reason
	lbl_reason = new UILabel(&oamSub, btn_perma->nextOamInd(), 3, 1, RGB15(31,31,31), 5, 0);
	lbl_reasonValue = new UILabel(&oamSub, lbl_reason->nextOamInd(), 4, 1, RGB15(4, 4, 4), 3, 0);
	btn_reasonInput = new UIButton(&oamSub, "/data/ao-nds/ui/spr_settingsInput", lbl_reasonValue->nextOamInd(), 6, 1, SpriteSize_32x16, 57, 137, 192, 16, 32, 16, 8);

	lbl_reason->setPos(btn_reasonInput->getX()-47, btn_reasonInput->getY()+2);
	lbl_reasonValue->setPos(btn_reasonInput->getX()+3, btn_reasonInput->getY()+1);
	lbl_reason->setText("Reason:");

	kb_input = new AOkeyboard(1, btn_reasonInput->nextOamInd(), 5);
	inputting = 0;

	lbl_desc->setPos(99, 42);

	btn_back->assignKey(KEY_B);
	btn_confirm->assignKey(KEY_A);

	btn_back->connect(onBackClicked, this);
	btn_confirm->connect(onConfirmClicked, this);
	btn_daysInput->connect(onDaysInput, this);
	btn_hoursInput->connect(onHoursInput, this);
	btn_minutesInput->connect(onMinutesInput, this);
	btn_perma->connect(onPermaToggled, this);
	btn_reasonInput->connect(onReasonInput, this);

	fillInfo();
}

void UICourtModeratorDialog::updateInput()
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
				if (!kb_input->getValue().empty() || inputting == lbl_reasonValue)
					inputting->setText(kb_input->getValue());
				inputting = 0;
			}
		}
		return;
	}

	btn_back->updateInput();
	btn_confirm->updateInput();
	btn_daysInput->updateInput();
	btn_hoursInput->updateInput();
	btn_minutesInput->updateInput();
	btn_perma->updateInput();
	btn_reasonInput->updateInput();
}

void UICourtModeratorDialog::update()
{

}

void UICourtModeratorDialog::hideEverything()
{
	bgHide(bgIndex);

	btn_back->setVisible(false);
	btn_confirm->setVisible(false);
	lbl_name->setVisible(false);
	lbl_desc->setVisible(false);
	spr_profile->setVisible(false);
	lbl_duration->setVisible(false);
	lbl_days->setVisible(false);
	lbl_daysValue->setVisible(false);
	btn_daysInput->setVisible(false);
	lbl_hours->setVisible(false);
	lbl_hoursValue->setVisible(false);
	btn_hoursInput->setVisible(false);
	lbl_minutes->setVisible(false);
	lbl_minutesValue->setVisible(false);
	btn_minutesInput->setVisible(false);
	lbl_perma->setVisible(false);
	btn_perma->setVisible(false);
	lbl_reason->setVisible(false);
	lbl_reasonValue->setVisible(false);
	btn_reasonInput->setVisible(false);
}

void UICourtModeratorDialog::showEverything()
{
	bgShow(bgIndex);

	btn_back->setVisible(true);
	btn_confirm->setVisible(true);
	lbl_name->setVisible(true);
	lbl_desc->setVisible(true);
	spr_profile->setVisible(true);

	if (isBan)
	{
		if (!btn_perma->getFrame()) // not permanent
		{
			lbl_duration->setVisible(true);
			lbl_days->setVisible(true);
			lbl_daysValue->setVisible(true);
			btn_daysInput->setVisible(true);
			lbl_hours->setVisible(true);
			lbl_hoursValue->setVisible(true);
			btn_hoursInput->setVisible(true);
			lbl_minutes->setVisible(true);
			lbl_minutesValue->setVisible(true);
			btn_minutesInput->setVisible(true);
		}
		lbl_perma->setVisible(true);
		btn_perma->setVisible(true);
	}
	lbl_reason->setVisible(true);
	lbl_reasonValue->setVisible(true);
	btn_reasonInput->setVisible(true);
}

void UICourtModeratorDialog::fillInfo()
{
	if (!isBan)
	{
		lbl_duration->setVisible(false);
		lbl_days->setVisible(false);
		lbl_daysValue->setVisible(false);
		btn_daysInput->setVisible(false);
		lbl_hours->setVisible(false);
		lbl_hoursValue->setVisible(false);
		btn_hoursInput->setVisible(false);
		lbl_minutes->setVisible(false);
		lbl_minutesValue->setVisible(false);
		btn_minutesInput->setVisible(false);
		lbl_perma->setVisible(false);
		btn_perma->setVisible(false);
	}

	currProfileID = (currProfile >= pCourtUI->getPlayerListIDs().size()) ? 0 : pCourtUI->getPlayerListIDs()[currProfile];

	playerInfo* info = 0;
	if (pCourtUI->getPlayerList().count(currProfileID))
		info = &pCourtUI->getPlayerList()[currProfileID];
	u32 currAreaID = (info) ? (u32)info->area : 0;

	std::string character = (info) ? info->character : "";
	if (info && character.empty())
		character = "Spectator";
	lbl_name->setText(character);
	lbl_name->setPos(163, 27, true);

	std::string desc;
	if (pCourtUI->getPlayerListIDs().empty())
		desc = "Player list is empty";
	else if (currAreaID < pCourtUI->getAreaList().size())
		desc = "In area \"" + pCourtUI->getAreaList()[currAreaID].name + "\"";
	else
		desc = "In area " + std::to_string(currAreaID);

	if (info)
	{
		if (!info->showname.empty())
			desc += ", Showname \"" + info->showname + "\"";
		if (!info->oocName.empty())
			desc += ", OOC \"" + info->oocName + "\"";
	}

	std::u16string finalDesc = utf8::utf8to16(desc);
	lbl_desc->setText(finalDesc);

	std::string file = "characters/" + (info ? info->character : "") + "/char_icon_big";
	bool exists = Content::exists(file+".img.bin", file);
	if (exists) file = file.substr(0, file.length()-8); // remove extension

	spr_profile->setImage(file, 64, 64, 4);
}

void UICourtModeratorDialog::onBackClicked(void* pUserData)
{
	UICourtModeratorDialog* pSelf = (UICourtModeratorDialog*)pUserData;

	wav_play(pSelf->pCourtUI->sndCancel);
	pSelf->pCourtUI->changeScreen(new UICourtProfileDetail(pSelf->pCourtUI, pSelf->currProfile, pSelf->wasInPrivateEvidence));
}

void UICourtModeratorDialog::onConfirmClicked(void* pUserData)
{
	UICourtModeratorDialog* pSelf = (UICourtModeratorDialog*)pUserData;

	wav_play(pSelf->pCourtUI->sndSelect);

	int duration;
	if (pSelf->isBan)
	{
		if (pSelf->btn_perma->getFrame())
			duration = -1;
		else
		{
			try
			{
				int days = std::stoi(pSelf->lbl_daysValue->getText8());
				int hours = std::stoi(pSelf->lbl_hoursValue->getText8());
				int minutes = std::stoi(pSelf->lbl_minutesValue->getText8());
				duration = (days*60*60) + (hours*60) + (minutes);
			}
			catch(...)
			{
				pSelf->pCourtUI->changeScreen(new UICourtMessage(pSelf->pCourtUI, "Invalid ban duration"));
				return;
			}
		}
	}
	else
		duration = 0;

	gEngine->getSocket()->sendData("MA#" + std::to_string(pSelf->currProfileID) + "#" + std::to_string(duration) + "#" + pSelf->lbl_reasonValue->getText8() + "#%");

	pSelf->pCourtUI->changeScreen(new UICourtEvidence(pSelf->pCourtUI));
}

void UICourtModeratorDialog::onDaysInput(void* pUserData)
{
	UICourtModeratorDialog* pSelf = (UICourtModeratorDialog*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->inputting = pSelf->lbl_daysValue;
	pSelf->hideEverything();
	pSelf->kb_input->show16("Enter duration in days");
}

void UICourtModeratorDialog::onHoursInput(void* pUserData)
{
	UICourtModeratorDialog* pSelf = (UICourtModeratorDialog*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->inputting = pSelf->lbl_hoursValue;
	pSelf->hideEverything();
	pSelf->kb_input->show16("Enter duration in hours");
}

void UICourtModeratorDialog::onMinutesInput(void* pUserData)
{
	UICourtModeratorDialog* pSelf = (UICourtModeratorDialog*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->inputting = pSelf->lbl_minutesValue;
	pSelf->hideEverything();
	pSelf->kb_input->show16("Enter duration in minutes");
}

void UICourtModeratorDialog::onPermaToggled(void* pUserData)
{
	UICourtModeratorDialog* pSelf = (UICourtModeratorDialog*)pUserData;

	int isPerma = pSelf->btn_perma->getFrame()^1;
	pSelf->btn_perma->setFrame(isPerma);

	pSelf->lbl_days->setVisible(!isPerma);
	pSelf->lbl_daysValue->setVisible(!isPerma);
	pSelf->btn_daysInput->setVisible(!isPerma);
	pSelf->lbl_hours->setVisible(!isPerma);
	pSelf->lbl_hoursValue->setVisible(!isPerma);
	pSelf->btn_hoursInput->setVisible(!isPerma);
	pSelf->lbl_minutes->setVisible(!isPerma);
	pSelf->lbl_minutesValue->setVisible(!isPerma);
	pSelf->btn_minutesInput->setVisible(!isPerma);
}

void UICourtModeratorDialog::onReasonInput(void* pUserData)
{
	UICourtModeratorDialog* pSelf = (UICourtModeratorDialog*)pUserData;

	wav_play(pSelf->pCourtUI->sndCrtRcrd);
	pSelf->inputting = pSelf->lbl_reasonValue;
	pSelf->hideEverything();
	pSelf->kb_input->show16("Enter reason");
}
