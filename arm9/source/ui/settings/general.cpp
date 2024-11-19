#include "ui/settings/general.h"

#include "settings.h"

UISettingsGeneral::~UISettingsGeneral()
{
	delete lbl_showname;
	delete lbl_shownameValue;
	delete lbl_oocname;
	delete lbl_oocnameValue;
	delete btn_showname;
	delete btn_oocname;
	delete kb_input;
}

void UISettingsGeneral::init()
{
	lbl_showname = new UILabel(&oamSub, pSettingsUI->getFirstOAM(), 4, 1, RGB15(31,31,31), 3, 0);
	lbl_shownameValue = new UILabel(&oamSub, lbl_showname->nextOamInd(), 4, 1, 0, 4, 0);
	lbl_oocname = new UILabel(&oamSub, lbl_shownameValue->nextOamInd(), 4, 1, RGB15(31,31,31), 3, 0);
	lbl_oocnameValue = new UILabel(&oamSub, lbl_oocname->nextOamInd(), 4, 1, 0, 4, 0);
	btn_showname = new UIButton(&oamSub, "/data/ao-nds/ui/spr_settingsInput", lbl_oocnameValue->nextOamInd(), 6, 1, SpriteSize_32x16, 128-96, 48, 192, 16, 32, 16, 5);
	btn_oocname = new UIButton(&oamSub, "/data/ao-nds/ui/spr_settingsInput", btn_showname->nextOamInd(), 6, 1, SpriteSize_32x16, 128-96, 104, 192, 16, 32, 16, 5);
	kb_input = new AOkeyboard(2, btn_oocname->nextOamInd(), 3);

	lbl_showname->setPos(btn_showname->getX(), btn_showname->getY()-12);
	lbl_shownameValue->setPos(btn_showname->getX()+3, btn_showname->getY()+1);
	lbl_oocname->setPos(btn_oocname->getX(), btn_oocname->getY()-12);
	lbl_oocnameValue->setPos(btn_oocname->getX()+3, btn_oocname->getY()+1);

	lbl_showname->setText("Default showname");
	lbl_shownameValue->setText(Settings::defaultShowname);
	lbl_oocname->setText("Default OOC name");
	lbl_oocnameValue->setText(Settings::defaultOOCname);

	btn_showname->connect(onShownameClicked, this);
	btn_oocname->connect(onOOCnameClicked, this);
}

void UISettingsGeneral::updateInput()
{
	if (kb_input->isVisible())
	{
		int result = kb_input->updateInput();
		if (result != 0)
		{
			dmaCopy(pSettingsUI->bgSubPal, BG_PALETTE_SUB, 512);
			pSettingsUI->showEverything();

			if (result > 0)
			{
				if (currEditing == btn_showname)
				{
					Settings::defaultShowname = kb_input->getValue();
					lbl_shownameValue->setText(kb_input->getValue());
				}
				else if (currEditing == btn_oocname)
				{
					Settings::defaultOOCname = kb_input->getValue();
					lbl_oocnameValue->setText(kb_input->getValue());
				}

				Settings::save();
			}
		}
		return;
	}

	btn_showname->updateInput();
	btn_oocname->updateInput();
}

void UISettingsGeneral::update()
{

}

void UISettingsGeneral::setVisible(bool on)
{
	lbl_showname->setVisible(on);
	lbl_shownameValue->setVisible(on);
	lbl_oocname->setVisible(on);
	lbl_oocnameValue->setVisible(on);
	btn_showname->setVisible(on);
	btn_oocname->setVisible(on);
}

void UISettingsGeneral::onShownameClicked(void* pUserData)
{
	UISettingsGeneral* pSelf = (UISettingsGeneral*)pUserData;
	wav_play(pSelf->pSettingsUI->sndCrtRcrd);

	pSelf->pSettingsUI->hideEverything();
	pSelf->currEditing = pSelf->btn_showname;
	pSelf->kb_input->show16("Enter default showname", Settings::defaultShowname);
}

void UISettingsGeneral::onOOCnameClicked(void* pUserData)
{
	UISettingsGeneral* pSelf = (UISettingsGeneral*)pUserData;
	wav_play(pSelf->pSettingsUI->sndCrtRcrd);

	pSelf->pSettingsUI->hideEverything();
	pSelf->currEditing = pSelf->btn_oocname;
	pSelf->kb_input->show16("Enter default OOC name", Settings::defaultOOCname);
}
