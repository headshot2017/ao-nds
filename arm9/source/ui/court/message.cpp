#include "ui/court/message.h"

#include <nds/arm9/background.h>
#include <nds/arm9/sprite.h>

#include "mp3_shared.h"
#include "engine.h"
#include "ui/court/ingamemenu.h"

UICourtMessage::~UICourtMessage()
{
	delete btn_ok;
	delete lbl_msg;
}

void UICourtMessage::init()
{
	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	loadBg("/data/ao-nds/ui/bg_ingameMainAlt", true);

	btn_ok = new UIButton(&oamSub, "/data/ao-nds/ui/spr_ok", 0, 3, 1, SpriteSize_32x32, 128-(76/2), 192-64, 76, 26, 32, 32, 0);
	lbl_msg = new UILabel(&oamSub, btn_ok->nextOamInd(), 6, 6, RGB15(31,31,31), 1, 0);

	btn_ok->assignKey(KEY_A);
	btn_ok->connect(onOKClicked, this);
	lbl_msg->setText(msg.c_str());
	lbl_msg->setPos(128, 36, true);
}

void UICourtMessage::updateInput()
{
	btn_ok->updateInput();
}

void UICourtMessage::update()
{

}

void UICourtMessage::onOKClicked(void* pUserData)
{
	UICourtMessage* pSelf = (UICourtMessage*)pUserData;
	wav_play(pSelf->pCourtUI->sndSelect);

	pSelf->pCourtUI->changeScreen(new UICourtIngameMenu(pSelf->pCourtUI));
}
