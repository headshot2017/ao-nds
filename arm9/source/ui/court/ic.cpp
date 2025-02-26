#include "ui/court/ic.h"

#include <math.h>

#include <nds/dma.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sound.h>

#include "utf8.h"
#include "libadx.h"
#include "engine.h"
#include "content.h"
#include "colors.h"
#include "ui/court/ingamemenu.h"
#include "ui/court/icchatlog.h"
#include "ui/court/judge.h"
#include "ui/court/mute.h"
#include "ui/court/pair.h"
#include "ui/court/courtrecord.h"

struct emoteBtnData
{
	UICourtIC* pObj;
	u32 btnInd;
};

UICourtIC::~UICourtIC()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), (displayingOptions) ? bg_icExtTilesLen : bg_icTilesLen);

	mem_free(bg_icTiles);
	mem_free(bg_icMap);
	mem_free(bg_icPal);
	mem_free(bg_icExtTiles);
	mem_free(bg_icExtMap);
	mem_free(bg_icExtPal);

	delete btn_back;
	delete btn_courtRecord;
	delete btn_shouts;
	delete btn_pair;
	delete btn_mute;
	delete btn_optionsToggle;
	delete btn_sideSelect;
	delete btn_tools;
	delete btn_additive;
	delete btn_preanim;
	delete btn_immediate;
	delete btn_flip;
	delete btn_shake;
	delete btn_flash;
	delete btn_slide;
	delete btn_prevPage;
	delete btn_nextPage;
	for (int i=0; i<4; i++) delete btn_emote[i];
	for (int i=0; i<2; i++) delete spr_bars[i];
	delete spr_arrowLeft;
	delete spr_arrowRight;
	delete lbl_showname;
	delete lbl_color;
	delete lbl_shout;
	delete lbl_slide;
	delete lbl_pages;
	delete kb_input;

	gEngine->getSocket()->removeMessageCallback("PV", cbPV);
	gEngine->getSocket()->removeMessageCallback("HP", cbHP);
}

void UICourtIC::init()
{
	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(bgIndex, 1);

	bg_icTiles = readFile("/data/ao-nds/ui/bg_ic.img.bin", &bg_icTilesLen);
	bg_icMap = readFile("/data/ao-nds/ui/bg_ic.map.bin");
	bg_icPal = readFile("/data/ao-nds/ui/bg_ic.pal.bin");
	bg_icExtTiles = readFile("/data/ao-nds/ui/bg_icExt.img.bin", &bg_icExtTilesLen);
	bg_icExtMap = readFile("/data/ao-nds/ui/bg_icExt.map.bin");
	bg_icExtPal = readFile("/data/ao-nds/ui/bg_icExt.pal.bin");

	dmaCopy(bg_icTiles, bgGetGfxPtr(bgIndex), bg_icTilesLen);
	dmaCopy(bg_icMap, bgGetMapPtr(bgIndex), 1536);

	btn_back = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icBtns", 0, 3, 1, SpriteSize_32x32, 0, 192-32, 80, 32, 32, 32, 0);
	btn_courtRecord = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icBtns", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-80, 0, 80, 32, 32, 32, 0);
	btn_shouts = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icBtns", btn_courtRecord->nextOamInd(), 3, 1, SpriteSize_32x32, 0, 0, 80, 32, 32, 32, 0);
	btn_pair = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icBtns", btn_shouts->nextOamInd(), 3, 1, SpriteSize_32x32, 36, 114, 68, 17, 32, 32, 0);
	btn_mute = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icBtns", btn_pair->nextOamInd(), 3, 1, SpriteSize_32x32, 153, 114, 68, 17, 32, 32, 0);
	btn_optionsToggle = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icOptions", btn_mute->nextOamInd(), 1, 1, SpriteSize_64x32, 99, 113, 59, 17, 64, 32, 1);
	btn_sideSelect = new UIButton(&oamSub, "/data/ao-nds/ui/spr_sides", btn_optionsToggle->nextOamInd(), 3, 1, SpriteSize_32x16, 183, 162, 73, 15, 32, 16, 2);
	btn_tools = new UIButton(&oamSub, "/data/ao-nds/ui/spr_tools", btn_sideSelect->nextOamInd(), 2, 1, SpriteSize_32x16, 105, 23, 46, 14, 32, 16, 3);
	btn_additive = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icControls", btn_tools->nextOamInd(), 2, 1, SpriteSize_32x16, 38, 116, 64, 15, 32, 16, 4);
	btn_preanim = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icControls", btn_additive->nextOamInd(), 2, 1, SpriteSize_32x16, 105, 116, 47, 15, 32, 16, 4);
	btn_immediate = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icControls", btn_preanim->nextOamInd(), 2, 1, SpriteSize_32x16, 155, 116, 64, 15, 32, 16, 4);
	btn_flip = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icControls", btn_immediate->nextOamInd(), 2, 1, SpriteSize_32x16, 46, 133, 47, 15, 32, 16, 4);
	btn_shake = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icControls", btn_flip->nextOamInd(), 2, 1, SpriteSize_32x16, 96, 133, 64, 15, 32, 16, 4);
	btn_flash = new UIButton(&oamSub, "/data/ao-nds/ui/spr_icControls", btn_shake->nextOamInd(), 2, 1, SpriteSize_32x16, 163, 133, 47, 15, 32, 16, 4);
	btn_slide = new UIButton(&oamSub, "/data/ao-nds/ui/spr_checkBox", btn_flash->nextOamInd(), 1, 1, SpriteSize_16x16, 199, 133, 16, 16, 16, 16, 5);
	btn_prevPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_emoteLeft", btn_slide->nextOamInd(), 1, 2, SpriteSize_16x32, 9, 71, 16, 40, 16, 32, 6);
	btn_nextPage = new UIButton(&oamSub, "/data/ao-nds/ui/spr_emoteLeft", btn_prevPage->nextOamInd(), 1, 2, SpriteSize_16x32, 231, 71, 16, 40, 16, 32, 6);

	static emoteBtnData btnData[4];
	for (u32 i=0; i<4; i++)
	{
		adx_update();
		int nextOam = (!i) ? btn_nextPage->nextOamInd() : btn_emote[i-1]->nextOamInd();
		btnData[i] = {this, i};

		btn_emote[i] = new UIButton(&oamSub, "", nextOam, 1, 1, SpriteSize_64x64, 38+(i*47), 71, 40, 40, 64, 64, 7+i);
		btn_emote[i]->connect(onEmoteClicked, &btnData[i]);
	}

	for (int i=0; i<2; i++)
	{
		adx_update();
		int nextOam = (!i) ? btn_emote[3]->nextOamInd() : spr_bars[i-1]->nextOamInd();

		std::string file = (!i) ? "/data/ao-nds/ui/spr_barDefense" : "/data/ao-nds/ui/spr_barProsecutor";
		spr_bars[i] = new UIButton(&oamSub, file, nextOam, 3, 1, SpriteSize_32x16, 35+(i*103), 43, 84, 14, 32, 16, 11+i);
	}

	spr_arrowLeft = new UIButton(&oamSub, "/data/ao-nds/ui/spr_emoteLeftArrow", spr_bars[1]->nextOamInd(), 1, 1, SpriteSize_8x16, 13, 84, 7, 13, 8, 16, 13);
	spr_arrowRight = new UIButton(&oamSub, "/data/ao-nds/ui/spr_emoteLeftArrow", spr_arrowLeft->nextOamInd(), 1, 1, SpriteSize_8x16, 236, 84, 7, 13, 8, 16, 13);

	lbl_showname = new UILabel(&oamSub, spr_arrowRight->nextOamInd(), 2, 1, RGB15(31,31,31), 14, 0);
	lbl_color = new UILabel(&oamSub, lbl_showname->nextOamInd(), 2, 1, RGB15(31,31,31), 15, 0);
	lbl_shout = new UILabel(&oamSub, lbl_color->nextOamInd(), 6, 1, RGB15(31,31,31), 14, 0);
	lbl_slide = new UILabel(&oamSub, lbl_shout->nextOamInd(), 1, 1, RGB15(31,31,31), 14, 0);
	lbl_pages = new UILabel(&oamSub, lbl_slide->nextOamInd(), 1, 1, RGB15(31,31,31), 14, 0);

	kb_input = new AOkeyboard(4, lbl_pages->nextOamInd(), 14);
	dmaCopy(bg_icPal, BG_PALETTE_SUB, 512);
	adx_update();
	isWritingChat = false;

	lbl_showname->setPos(80, 163);
	lbl_showname->setText((pCourtUI->showname.empty()) ? utf8::utf8to16(pCourtUI->getCurrChar().name) : pCourtUI->showname);
	lbl_color->setPos(86, 178);
	lbl_color->setText("Message");
	lbl_shout->setPos(btn_shouts->getX()+4, btn_shouts->getY()+btn_shouts->getH()-1, false);
	lbl_shout->setVisible(false);
	lbl_slide->setPos(btn_slide->getX()+btn_slide->getW()+3, btn_slide->getY()+2, false);
	lbl_slide->setText("Slide");

	btn_prevPage->setPriority(1);
	btn_nextPage->setPriority(1);

	btn_courtRecord->setFrame(1);
	btn_shouts->setFrame(2);
	btn_pair->setFrame(7);
	btn_mute->setFrame(8);

	btn_additive->setFrame(pCourtUI->icControls.additive);
	btn_preanim->setFrame(2 + pCourtUI->icControls.preanim);
	btn_immediate->setFrame(4 + pCourtUI->icControls.immediate);
	btn_flip->setFrame(6 + pCourtUI->icControls.flip);
	btn_shake->setFrame(8 + pCourtUI->icControls.shake);
	btn_flash->setFrame(10 + pCourtUI->icControls.flash);
	btn_slide->setFrame(pCourtUI->icControls.slide);
	btn_sideSelect->setFrame(pCourtUI->icControls.side);

	btn_back->unloadRAM();
	btn_courtRecord->unloadRAM();
	btn_pair->unloadRAM();
	btn_mute->unloadRAM();
	btn_tools->unloadRAM();
	btn_prevPage->unloadRAM();
	btn_nextPage->unloadRAM();
	spr_arrowLeft->unloadRAM();
	spr_arrowRight->unloadRAM();

	btn_nextPage->setFlip(true, false);
	spr_arrowRight->setFlip(true, false);

	btn_additive->setVisible(false);
	btn_preanim->setVisible(false);
	btn_immediate->setVisible(false);
	btn_flip->setVisible(false);
	btn_shake->setVisible(false);
	btn_flash->setVisible(false);
	btn_tools->setVisible(pCourtUI->icControls.side == 5);
	btn_slide->setVisible(pCourtUI->getFeature("custom_blips"));
	lbl_slide->setVisible(pCourtUI->getFeature("custom_blips"));

	btn_back->assignKey(KEY_B);
	btn_courtRecord->assignKey(KEY_R);
	btn_shouts->assignKey(KEY_L);
	btn_prevPage->assignKey(KEY_LEFT);
	btn_nextPage->assignKey(KEY_RIGHT);

	btn_back->connect(onBackClicked, this);
	btn_courtRecord->connect(onCourtRecord, this);
	btn_shouts->connect(onShoutsToggled, this);
	btn_pair->connect(onPairClicked, this);
	btn_mute->connect(onMuteClicked, this);
	btn_optionsToggle->connect(onOptionsToggled, this);
	btn_sideSelect->connect(onSideClicked, this);
	btn_tools->connect(onToolsClicked, this);
	btn_additive->connect(onAdditiveClicked, this);
	btn_preanim->connect(onPreanimClicked, this);
	btn_immediate->connect(onImmediateClicked, this);
	btn_flip->connect(onFlipClicked, this);
	btn_shake->connect(onShakeClicked, this);
	btn_flash->connect(onFlashClicked, this);
	btn_slide->connect(onSlideClicked, this);
	btn_prevPage->connect(onPrevPage, this);
	btn_nextPage->connect(onNextPage, this);

	currPage = 0;
	currEmoteSelected = 0;

	arrowX = 0;
	arrowXadd = 1;
	arrowTicks = 0;
	currShout = 0;
	displayingOptions = false;

	cbPV = gEngine->getSocket()->addMessageCallback("PV", onMessagePV, this);
	cbHP = gEngine->getSocket()->addMessageCallback("HP", onMessageHP, this);

	reloadPage();
	reloadBars();
}

void UICourtIC::updateInput()
{
	if (kb_input->isVisible())
	{
		int result = kb_input->updateInput();
		if (result != 0)
		{
			if (result > 0)
			{
				if (!kb_input->getValue().empty() && isWritingChat)
				{
					const charInfo& character = pCourtUI->getCurrChar();
					const emoteInfo& emote = pCourtUI->getCharEmotes()[currPage*4 + currEmoteSelected];

					int emoteMod;
					if (currShout)
					{
						if (emote.emoteModifier <= 1) // preanim or not
							emoteMod = 2; // shout + preanim
						else
							emoteMod = 6; // shout + zoom
					}
					else
						emoteMod = (emote.emoteModifier <= 1) ? pCourtUI->icControls.preanim : emote.emoteModifier;

					bool shoutIsCustom = (currShout >= 4);
					int shoutInd = (shoutIsCustom) ? 4 : currShout;
					std::string shoutStr = std::to_string(shoutInd);
					if (shoutIsCustom && !pCourtUI->getCharShouts()[currShout-4].filename.empty())
						shoutStr += "&" + pCourtUI->getCharShouts()[currShout-4].filename;

					std::string chatmsg = kb_input->getValueUTF8();
					std::string showname = utf8::utf16to8(pCourtUI->showname);
					AOencode(chatmsg);
					AOencode(showname);

					std::string msg = "MS#" +
						std::to_string(emote.deskMod) + "#" +
						emote.preanim + "#" +
						character.name + "#" +
						emote.anim + "#" +
						chatmsg + "#" +
						indToSide[pCourtUI->icControls.side] + "#" +
						emote.sound + "#" +
						std::to_string(emoteMod) + "#" +
						std::to_string(pCourtUI->getCurrCharID()) + "#" +
						std::to_string(emote.delay) + "#" +
						shoutStr + "#" +
						std::to_string(pCourtUI->icControls.evidence + 1) + "#" +
						std::to_string(pCourtUI->icControls.flip) + "#" +
						std::to_string(pCourtUI->icControls.flash) + "#" +
						std::to_string(pCourtUI->icControls.color) + "#" +
						showname + "#" +
						std::to_string(pCourtUI->icControls.pairID) + "#" +
						std::to_string(pCourtUI->icControls.xOffset) + "&" + std::to_string(pCourtUI->icControls.yOffset) + "#" +
						std::to_string(pCourtUI->icControls.immediate) + "#" +
						std::to_string(0) + "#" + // sfx looping
						std::to_string(pCourtUI->icControls.shake) + "#" +
						emote.frameShake + "#" +
						emote.frameFlash + "#" +
						emote.frameSFX + "#";

						if (pCourtUI->getFeature("additive"))
							msg += std::to_string(pCourtUI->icControls.additive) + "#";
						if (pCourtUI->getFeature("effects"))
							msg += "||#"; // empty
						if (pCourtUI->getFeature("custom_blips"))
						{
							msg += character.blip + "#" +
								std::to_string(pCourtUI->icControls.slide) + "#";
						}

						msg += "%";

					if (pCourtUI->icControls.evidence > -1)
						pCourtUI->icControls.evidence = -1;

					pCourtUI->sendIC(msg);
				}
				else if (!isWritingChat)
					pCourtUI->showname = kb_input->getValue();
			}

			dmaCopy((displayingOptions) ? bg_icExtPal : bg_icPal, BG_PALETTE_SUB, 512);
			if (displayingOptions)
			{
				btn_additive->setVisible(true);
				btn_preanim->setVisible(true);
				btn_immediate->setVisible(true);
				btn_flip->setVisible(true);
				btn_shake->setVisible(true);
				btn_flash->setVisible(true);
			}
			else
			{
				btn_pair->setVisible(true);
				btn_mute->setVisible(true);
				btn_slide->setVisible(pCourtUI->getFeature("custom_blips"));
				lbl_showname->setVisible(true);
				lbl_slide->setVisible(pCourtUI->getFeature("custom_blips"));
				lbl_pages->setVisible(true);
			}
			adx_update();

			bgShow(bgIndex);

			btn_back->setVisible(true);
			btn_courtRecord->setVisible(true);
			btn_shouts->setVisible(true);
			btn_optionsToggle->setVisible(true);
			btn_sideSelect->setVisible(true);
			btn_tools->setVisible(pCourtUI->icControls.side == 5);
			lbl_color->setVisible(true);
			lbl_shout->setVisible(currShout >= 4);
			for (int i=0; i<2; i++) spr_bars[i]->setVisible(true);
			for (u32 i=0; i<4; i++)
			{
				u32 ind = currPage*4 + i;
				btn_emote[i]->setVisible(ind < pCourtUI->getCharEmotes().size());
			}
			u32 maxPages = (u32)ceil(pCourtUI->getCharEmotes().size()/4.f);
			btn_prevPage->setVisible(currPage > 0);
			spr_arrowLeft->setVisible(currPage > 0);
			btn_nextPage->setVisible(currPage+1 < maxPages);
			spr_arrowRight->setVisible(currPage+1 < maxPages);

			lbl_showname->setText((pCourtUI->showname.empty()) ? utf8::utf8to16(pCourtUI->getCurrChar().name) : pCourtUI->showname);
		}
		return;
	}

	u32 key = keysDown();
	if (key & KEY_Y)
	{
		wav_play(pCourtUI->sndCrtRcrd);
		pCourtUI->changeScreen(new UICourtICChatLog(pCourtUI));
	}
	else if (key & KEY_TOUCH)
	{
		touchPosition pos;
		touchRead(&pos);

		if (!displayingOptions && pos.px >= 79 && pos.py >= 162 && pos.px < 79+77 && pos.py < 162+14)
		{
			// showname input
			wav_play(pCourtUI->sndCrtRcrd);
			isWritingChat = false;
			hideEverything();
			kb_input->show16("Enter a showname", pCourtUI->showname);
		}
		else if (pos.px >= 79 && pos.py >= 177 && pos.px < 79+163 && pos.py < 177+15)
		{
			// chat input
			wav_play(pCourtUI->sndCrtRcrd);
			isWritingChat = true;
			hideEverything();
			kb_input->show("Enter IC chat message", "");
		}
		else if (pos.px >= 242 && pos.py >= 177 && pos.px < 242+14 && pos.py < 177+15)
		{
			// color switcher
			wav_play(pCourtUI->sndEvPage);
			pCourtUI->icControls.color = (pCourtUI->icControls.color+1) % 6;
			lbl_color->setColor(paletteColorInd[pCourtUI->icControls.color]);
		}
	}

	btn_back->updateInput();
	btn_shouts->updateInput();
	btn_courtRecord->updateInput();
	btn_pair->updateInput();
	btn_mute->updateInput();
	btn_sideSelect->updateInput();
	btn_tools->updateInput();
	btn_additive->updateInput();
	btn_preanim->updateInput();
	btn_immediate->updateInput();
	btn_flip->updateInput();
	btn_shake->updateInput();
	btn_flash->updateInput();
	btn_slide->updateInput();
	btn_optionsToggle->updateInput();
	btn_prevPage->updateInput();
	btn_nextPage->updateInput();
	btn_prevPage->updateInput();
	btn_nextPage->updateInput();
	for (int i=0; i<4; i++) btn_emote[i]->updateInput();
}

void UICourtIC::update()
{
	arrowTicks++;
	if (arrowTicks >= 4)
	{
		arrowTicks = 0;
		arrowX += arrowXadd;
		if (arrowX <= -1 || arrowX >= 1)
			arrowXadd = -arrowXadd;
	}

	spr_arrowLeft->setPos(13-arrowX, spr_arrowLeft->getY());
	spr_arrowRight->setPos(236+arrowX, spr_arrowRight->getY());
}

void UICourtIC::reloadPage()
{
	u32 visible = 0;

	for (u32 i=0; i<4; i++)
	{
		adx_update();

		u32 ind = currPage*4 + i;
		if (ind >= pCourtUI->getCharEmotes().size())
		{
			btn_emote[i]->setVisible(false);
			continue;
		}
		visible++;
	}

	if (visible && currEmoteSelected >= visible)
		currEmoteSelected = visible-1;

	for (u32 i=0; i<visible; i++)
	{
		adx_update();

		u32 ind = currPage*4 + i;
		std::string buttonFile = "characters/" + pCourtUI->getCurrChar().name + "/emotions/button" + std::to_string(ind+1) + "_off";
		bool exists = Content::exists(buttonFile+".img.bin", buttonFile);
		if (exists) buttonFile = buttonFile.substr(0, buttonFile.length()-8); // remove extension

		btn_emote[i]->setImage(buttonFile, 64, 64, 7+i);
		btn_emote[i]->setVisible(true);
		btn_emote[i]->unloadRAM(false);

		if (i == currEmoteSelected)
			btn_emote[i]->darken();
	}

	u32 maxPages = (u32)ceil(pCourtUI->getCharEmotes().size()/4.f);
	btn_prevPage->setVisible(currPage > 0);
	spr_arrowLeft->setVisible(currPage > 0);
	btn_nextPage->setVisible(currPage+1 < maxPages);
	spr_arrowRight->setVisible(currPage+1 < maxPages);

	char buf[32];
	sprintf(buf, "%ld/%ld", currPage+1, maxPages);
	lbl_pages->setText(buf);
	lbl_pages->setPos(128, btn_optionsToggle->getY()+btn_optionsToggle->getH()+4, true);
}

void UICourtIC::reloadBars()
{
	for (int i=0; i<2; i++)
		spr_bars[i]->setFrame(pCourtUI->bars[i]);
}

void UICourtIC::hideEverything()
{
	bgHide(bgIndex);

	btn_back->setVisible(false);
	btn_courtRecord->setVisible(false);
	btn_shouts->setVisible(false);
	btn_pair->setVisible(false);
	btn_mute->setVisible(false);
	btn_optionsToggle->setVisible(false);
	btn_sideSelect->setVisible(false);
	btn_tools->setVisible(false);
	btn_additive->setVisible(false);
	btn_preanim->setVisible(false);
	btn_immediate->setVisible(false);
	btn_flip->setVisible(false);
	btn_shake->setVisible(false);
	btn_flash->setVisible(false);
	btn_slide->setVisible(false);
	btn_prevPage->setVisible(false);
	btn_nextPage->setVisible(false);
	for (int i=0; i<4; i++) btn_emote[i]->setVisible(false);
	for (int i=0; i<2; i++) spr_bars[i]->setVisible(false);
	spr_arrowLeft->setVisible(false);
	spr_arrowRight->setVisible(false);
	lbl_showname->setVisible(false);
	lbl_color->setVisible(false);
	lbl_shout->setVisible(false);
	lbl_slide->setVisible(false);
}

void UICourtIC::onBackClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndCancel);

	pSelf->pCourtUI->changeScreen(new UICourtIngameMenu(pSelf->pCourtUI));
}

void UICourtIC::onCourtRecord(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndCrtRcrd);

	pSelf->pCourtUI->changeScreen(new UICourtEvidence(pSelf->pCourtUI));
}

void UICourtIC::onShoutsToggled(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndSelect);

	pSelf->currShout = (pSelf->currShout+1) % (4 + pSelf->pCourtUI->getCharShouts().size());

	bool isCustom = (pSelf->currShout >= 4);
	int shoutFrame = (isCustom) ? 4 : pSelf->currShout;

	pSelf->btn_shouts->setFrame(2 + shoutFrame);
	pSelf->lbl_shout->setVisible(isCustom);
	if (isCustom) pSelf->lbl_shout->setText(pSelf->pCourtUI->getCharShouts()[pSelf->currShout-4].displayname);
}

void UICourtIC::onPairClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndCrtRcrd);

	pSelf->pCourtUI->changeScreen(new UICourtPair(pSelf->pCourtUI));
}

void UICourtIC::onMuteClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndCrtRcrd);

	pSelf->pCourtUI->changeScreen(new UICourtMute(pSelf->pCourtUI));
}

void UICourtIC::onOptionsToggled(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndCrtRcrd);

	pSelf->displayingOptions = !pSelf->displayingOptions;

	if (pSelf->displayingOptions)
	{
		dmaCopy(pSelf->bg_icExtTiles, bgGetGfxPtr(pSelf->bgIndex), pSelf->bg_icExtTilesLen);
		dmaCopy(pSelf->bg_icExtMap, bgGetMapPtr(pSelf->bgIndex), 1536);
		dmaCopy(pSelf->bg_icExtPal, BG_PALETTE_SUB, 512);

		pSelf->lbl_showname->setVisible(false);
		pSelf->btn_additive->setVisible(true);
		pSelf->btn_preanim->setVisible(true);
		pSelf->btn_immediate->setVisible(true);
		pSelf->btn_flip->setVisible(true);
		pSelf->btn_shake->setVisible(true);
		pSelf->btn_flash->setVisible(true);
		pSelf->btn_pair->setVisible(false);
		pSelf->btn_mute->setVisible(false);
		pSelf->btn_slide->setVisible(false);
		pSelf->lbl_slide->setVisible(false);
		pSelf->lbl_pages->setVisible(false);

		pSelf->btn_optionsToggle->setPos(pSelf->btn_optionsToggle->getX(), 153);
	}
	else
	{
		dmaCopy(pSelf->bg_icTiles, bgGetGfxPtr(pSelf->bgIndex), pSelf->bg_icTilesLen);
		dmaCopy(pSelf->bg_icMap, bgGetMapPtr(pSelf->bgIndex), 1536);
		dmaCopy(pSelf->bg_icPal, BG_PALETTE_SUB, 512);

		pSelf->lbl_showname->setVisible(true);
		pSelf->btn_additive->setVisible(false);
		pSelf->btn_preanim->setVisible(false);
		pSelf->btn_immediate->setVisible(false);
		pSelf->btn_flip->setVisible(false);
		pSelf->btn_shake->setVisible(false);
		pSelf->btn_flash->setVisible(false);
		pSelf->btn_pair->setVisible(true);
		pSelf->btn_mute->setVisible(true);
		pSelf->btn_slide->setVisible(pSelf->pCourtUI->getFeature("custom_blips"));
		pSelf->lbl_slide->setVisible(pSelf->pCourtUI->getFeature("custom_blips"));
		pSelf->lbl_pages->setVisible(true);

		pSelf->btn_optionsToggle->setPos(pSelf->btn_optionsToggle->getX(), 113);
	}

	pSelf->btn_optionsToggle->setFrame(pSelf->displayingOptions);
}

void UICourtIC::onSideClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->pCourtUI->icControls.side = (pSelf->pCourtUI->icControls.side + 1) % 6;
	pSelf->btn_sideSelect->setFrame(pSelf->pCourtUI->icControls.side);
	pSelf->btn_tools->setVisible(pSelf->pCourtUI->icControls.side == 5); // show tools button on Judge pos
}

void UICourtIC::onToolsClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndCrtRcrd);

	pSelf->pCourtUI->changeScreen(new UICourtJudge(pSelf->pCourtUI));
}

void UICourtIC::onAdditiveClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndSelect);

	pSelf->pCourtUI->icControls.additive ^= 1;
	pSelf->btn_additive->setFrame(pSelf->pCourtUI->icControls.additive);
}

void UICourtIC::onPreanimClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndSelect);

	pSelf->pCourtUI->icControls.preanim ^= 1;
	pSelf->btn_preanim->setFrame(2+pSelf->pCourtUI->icControls.preanim);
}

void UICourtIC::onImmediateClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndSelect);

	pSelf->pCourtUI->icControls.immediate ^= 1;
	pSelf->btn_immediate->setFrame(4+pSelf->pCourtUI->icControls.immediate);
}

void UICourtIC::onFlipClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndSelect);

	pSelf->pCourtUI->icControls.flip ^= 1;
	pSelf->btn_flip->setFrame(6+pSelf->pCourtUI->icControls.flip);
}

void UICourtIC::onShakeClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndSelect);

	pSelf->pCourtUI->icControls.shake ^= 1;
	pSelf->btn_shake->setFrame(8+pSelf->pCourtUI->icControls.shake);
}

void UICourtIC::onFlashClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndSelect);

	pSelf->pCourtUI->icControls.flash ^= 1;
	pSelf->btn_flash->setFrame(10+pSelf->pCourtUI->icControls.flash);
}

void UICourtIC::onSlideClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;

	pSelf->pCourtUI->icControls.slide ^= 1;
	pSelf->btn_slide->setFrame(pSelf->pCourtUI->icControls.slide);
}

void UICourtIC::onPrevPage(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->currPage--;
	pSelf->reloadPage();
}

void UICourtIC::onNextPage(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	wav_play(pSelf->pCourtUI->sndEvPage);

	pSelf->currPage++;
	pSelf->reloadPage();
}

void UICourtIC::onEmoteClicked(void* pUserData)
{
	emoteBtnData* pData = (emoteBtnData*)pUserData;
	UICourtIC* pSelf = pData->pObj;

	if (pData->btnInd == pSelf->currEmoteSelected) return;

	// restore previous emote button palette
	pSelf->btn_emote[pSelf->currEmoteSelected]->restorePalette();

	pSelf->currEmoteSelected = pData->btnInd;

	// darken selected button palette
	pSelf->btn_emote[pSelf->currEmoteSelected]->darken();
}

void UICourtIC::onMessagePV(void* pUserData, std::string msg)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;

	if (pSelf->pCourtUI->showname.empty())
		pSelf->lbl_showname->setText(pSelf->pCourtUI->getCurrChar().name);
	pSelf->currPage = 0;
	pSelf->reloadPage();
}

void UICourtIC::onMessageHP(void* pUserData, std::string msg)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;

	pSelf->reloadBars();
}
