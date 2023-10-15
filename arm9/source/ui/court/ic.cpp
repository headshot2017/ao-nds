#include "ui/court/ic.h"

#include <math.h>

#include <nds/dma.h>
#include <nds/arm9/background.h>
#include <nds/arm9/sound.h>

#include "mp3_shared.h"
#include "engine.h"
#include "colors.h"
#include "ui/court/ingamemenu.h"
#include "ui/court/icchatlog.h"

struct emoteBtnData
{
	UICourtIC* pObj;
	u32 btnInd;
};

UICourtIC::~UICourtIC()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), 1536);
	dmaFillHalfWords(0, BG_PALETTE_SUB, 512);

	delete[] bgPal;

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
	delete btn_prevPage;
	delete btn_nextPage;
	for (int i=0; i<4; i++) delete btn_emote[i];
	for (int i=0; i<2; i++) delete spr_bars[i];
	delete spr_arrowLeft;
	delete spr_arrowRight;
	delete lbl_showname;
	delete lbl_color;
	delete kb_input;

	gEngine->getSocket()->removeMessageCallback("PV", cbPV);
	gEngine->getSocket()->removeMessageCallback("HP", cbHP);
}

void UICourtIC::init()
{
	bgIndex = bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	bgSetPriority(bgIndex, 1);

	u8* bgTiles = readFile("nitro:/bg_ic.img.bin", &bgTilesLen);
	u8* bgMap = readFile("nitro:/bg_ic.map.bin");
	bgPal = readFile("nitro:/bg_ic.pal.bin");

	dmaCopy(bgTiles, bgGetGfxPtr(bgIndex), bgTilesLen);
	dmaCopy(bgMap, bgGetMapPtr(bgIndex), 1536);

	delete[] bgTiles;
	delete[] bgMap;

	btn_back = new UIButton(&oamSub, "nitro:/spr_icCornerBtns", 0, 3, 1, SpriteSize_32x32, 0, 192-30, 79, 30, 32, 32, 0);
	btn_courtRecord = new UIButton(&oamSub, "nitro:/spr_icCornerBtns", btn_back->nextOamInd(), 3, 1, SpriteSize_32x32, 256-80, 0, 80, 32, 32, 32, 0);
	btn_shouts = new UIButton(&oamSub, "nitro:/spr_icCornerBtns", btn_courtRecord->nextOamInd(), 3, 1, SpriteSize_32x32, 0, 0, 80, 32, 32, 32, 0);
	btn_pair = new UIButton(&oamSub, "nitro:/spr_pairMute", btn_shouts->nextOamInd(), 3, 1, SpriteSize_32x32, 36, 114, 68, 17, 32, 32, 1);
	btn_mute = new UIButton(&oamSub, "nitro:/spr_pairMute", btn_pair->nextOamInd(), 3, 1, SpriteSize_32x32, 153, 114, 68, 17, 32, 32, 1);
	btn_optionsToggle = new UIButton(&oamSub, "nitro:/spr_icOptions", btn_mute->nextOamInd(), 1, 1, SpriteSize_64x32, 99, 113, 59, 17, 64, 32, 2);
	btn_sideSelect = new UIButton(&oamSub, "nitro:/spr_sides", btn_optionsToggle->nextOamInd(), 3, 1, SpriteSize_32x16, 183, 162, 73, 15, 32, 16, 3);
	btn_tools = new UIButton(&oamSub, "nitro:/spr_tools", btn_sideSelect->nextOamInd(), 2, 1, SpriteSize_32x16, 105, 23, 46, 14, 32, 16, 4);
	btn_additive = new UIButton(&oamSub, "nitro:/spr_icControls", btn_tools->nextOamInd(), 2, 1, SpriteSize_32x16, 38, 116, 64, 15, 32, 16, 5);
	btn_preanim = new UIButton(&oamSub, "nitro:/spr_icControls", btn_additive->nextOamInd(), 2, 1, SpriteSize_32x16, 105, 116, 47, 15, 32, 16, 5);
	btn_immediate = new UIButton(&oamSub, "nitro:/spr_icControls", btn_preanim->nextOamInd(), 2, 1, SpriteSize_32x16, 155, 116, 64, 15, 32, 16, 5);
	btn_flip = new UIButton(&oamSub, "nitro:/spr_icControls", btn_immediate->nextOamInd(), 2, 1, SpriteSize_32x16, 46, 133, 47, 15, 32, 16, 5);
	btn_shake = new UIButton(&oamSub, "nitro:/spr_icControls", btn_flip->nextOamInd(), 2, 1, SpriteSize_32x16, 96, 133, 64, 15, 32, 16, 5);
	btn_flash = new UIButton(&oamSub, "nitro:/spr_icControls", btn_shake->nextOamInd(), 2, 1, SpriteSize_32x16, 163, 133, 47, 15, 32, 16, 5);
	btn_prevPage = new UIButton(&oamSub, "nitro:/spr_emoteLeft", btn_flash->nextOamInd(), 1, 2, SpriteSize_16x32, 9, 71, 16, 40, 16, 32, 6);
	btn_nextPage = new UIButton(&oamSub, "nitro:/spr_emoteLeft", btn_prevPage->nextOamInd(), 1, 2, SpriteSize_16x32, 231, 71, 16, 40, 16, 32, 6);

	static emoteBtnData btnData[4];
	for (u32 i=0; i<4; i++)
	{
		int nextOam = (!i) ? btn_nextPage->nextOamInd() : btn_emote[i-1]->nextOamInd();
		btnData[i] = {this, i};
		btnPalettes[i] = 0;

		btn_emote[i] = new UIButton(&oamSub, "", nextOam, 1, 1, SpriteSize_64x64, 38+(i*47), 71, 40, 40, 64, 64, 7+i);
		btn_emote[i]->connect(onEmoteClicked, &btnData[i]);
	}

	for (int i=0; i<2; i++)
	{
		int nextOam = (!i) ? btn_emote[3]->nextOamInd() : spr_bars[i-1]->nextOamInd();

		std::string file = (!i) ? "spr_barDefense" : "spr_barProsecutor";
		spr_bars[i] = new UIButton(&oamSub, file, nextOam, 3, 1, SpriteSize_32x16, 35+(i*103), 43, 84, 14, 32, 16, 11+i);
	}

	spr_arrowLeft = new UIButton(&oamSub, "nitro:/spr_emoteLeftArrow", spr_bars[1]->nextOamInd(), 1, 1, SpriteSize_8x16, 13, 84, 7, 13, 8, 16, 13);
	spr_arrowRight = new UIButton(&oamSub, "nitro:/spr_emoteLeftArrow", spr_arrowLeft->nextOamInd(), 1, 1, SpriteSize_8x16, 236, 84, 7, 13, 8, 16, 13);

	lbl_showname = new UILabel(&oamSub, spr_arrowRight->nextOamInd(), 2, 1, RGB15(31,31,31), 14, 0);
	lbl_color = new UILabel(&oamSub, lbl_showname->nextOamInd(), 2, 1, RGB15(31,31,31), 15, 0);
	lbl_showname->setPos(77, 163);
	lbl_showname->setText(pCourtUI->showname.c_str());
	lbl_color->setPos(84, 178);
	lbl_color->setText("Message");

	kb_input = new AOkeyboard(4, lbl_color->nextOamInd(), 14);
	dmaCopy(bgPal, BG_PALETTE_SUB, 512);
	isWritingChat = false;

	btn_prevPage->setPriority(1);
	btn_nextPage->setPriority(1);

	btn_courtRecord->setFrame(1);
	btn_shouts->setFrame(2);
	btn_mute->setFrame(1);

	btn_additive->setFrame(pCourtUI->icControls.additive);
	btn_preanim->setFrame(2 + pCourtUI->icControls.preanim);
	btn_immediate->setFrame(4 + pCourtUI->icControls.immediate);
	btn_flip->setFrame(6 + pCourtUI->icControls.flip);
	btn_shake->setFrame(8 + pCourtUI->icControls.shake);
	btn_flash->setFrame(10 + pCourtUI->icControls.flash);
	btn_sideSelect->setFrame(pCourtUI->icControls.side);

	btn_nextPage->setFlip(true, false);
	spr_arrowRight->setFlip(true, false);

	btn_additive->setVisible(false);
	btn_preanim->setVisible(false);
	btn_immediate->setVisible(false);
	btn_flip->setVisible(false);
	btn_shake->setVisible(false);
	btn_flash->setVisible(false);
	btn_tools->setVisible(pCourtUI->icControls.side == 5);

	btn_back->assignKey(KEY_B);
	btn_courtRecord->assignKey(KEY_R);
	btn_shouts->assignKey(KEY_L);

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
				if (isWritingChat)
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

					std::string msg = "MS#chat#" +
						emote.preanim + "#" +
						character.name + "#" +
						emote.anim + "#" +
						kb_input->getValue() + "#" +
						indToSide[pCourtUI->icControls.side] + "#" +
						emote.sound + "#" +
						std::to_string(emoteMod) + "#" +
						std::to_string(pCourtUI->getCurrCharID()) + "#" +
						std::to_string(emote.delay) + "#" +
						std::to_string(currShout) + "#" +
						std::to_string(pCourtUI->icControls.evidence + 1) + "#" +
						std::to_string(pCourtUI->icControls.flip) + "#" +
						std::to_string(pCourtUI->icControls.flash) + "#" +
						std::to_string(pCourtUI->icControls.color) + "#" +
						pCourtUI->showname + "#" +
						std::to_string(pCourtUI->icControls.pairID) + "#" +
						std::to_string(pCourtUI->icControls.offset) + "#" +
						std::to_string(pCourtUI->icControls.immediate) + "#" +
						std::to_string(0) + "#" + // sfx looping
						std::to_string(pCourtUI->icControls.shake) + "#" +
						"-^(b)"+emote.anim+"^(a)"+emote.anim+"^#" + // shake frames
						"-^(b)"+emote.anim+"^(a)"+emote.anim+"^#" + // realization frames
						"-^(b)"+emote.anim+"^(a)"+emote.anim+"^#" + // sfx frames
						std::to_string(pCourtUI->icControls.additive) + "#||#%"; // "||" is effects (won't bother with these)

					pCourtUI->sendIC(msg);
				}
				else
					pCourtUI->showname = kb_input->getValue();
			}

			dmaCopy(bgPal, BG_PALETTE_SUB, 512);
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
				lbl_showname->setVisible(true);
			}
			mp3_fill_buffer();

			bgShow(bgIndex);

			btn_back->setVisible(true);
			btn_courtRecord->setVisible(true);
			btn_shouts->setVisible(true);
			btn_optionsToggle->setVisible(true);
			btn_sideSelect->setVisible(true);
			btn_tools->setVisible(pCourtUI->icControls.side == 5);
			lbl_color->setVisible(true);
			for (int i=0; i<2; i++) spr_bars[i]->setVisible(true);
			reloadPage();

			lbl_showname->setText((pCourtUI->showname.empty()) ? pCourtUI->getCurrChar().name.c_str() : pCourtUI->showname.c_str());
		}
		return;
	}

	u32 key = keysDown();
	if (key & KEY_Y)
	{
		soundPlaySample(pCourtUI->sndSelect, SoundFormat_16Bit, pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
		pCourtUI->changeScreen(new UICourtICChatLog(pCourtUI));
	}
	else if (key & KEY_TOUCH)
	{
		touchPosition pos;
		touchRead(&pos);

		if (!displayingOptions && pos.px >= 79 && pos.py >= 162 && pos.px < 79+77 && pos.py < 162+14)
		{
			// showname input
			soundPlaySample(pCourtUI->sndSelect, SoundFormat_16Bit, pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
			isWritingChat = false;
			hideEverything();
			kb_input->show("Enter a showname", pCourtUI->showname.c_str());
		}
		else if (pos.px >= 79 && pos.py >= 177 && pos.px < 79+163 && pos.py < 177+15)
		{
			// chat input
			soundPlaySample(pCourtUI->sndSelect, SoundFormat_16Bit, pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
			isWritingChat = true;
			hideEverything();
			kb_input->show("Enter IC chat message", "");
		}
		else if (pos.px >= 242 && pos.py >= 177 && pos.px < 242+14 && pos.py < 177+15)
		{
			// color switcher
			soundPlaySample(pCourtUI->sndEvPage, SoundFormat_16Bit, pCourtUI->sndEvPageSize, 32000, 127, 64, false, 0);
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
		if (btnPalettes[i]) delete btnPalettes[i];

		mp3_fill_buffer();

		u32 ind = currPage*4 + i;
		if (ind >= pCourtUI->getCharEmotes().size())
		{
			btnPalettes[i] = 0;
			btn_emote[i]->setVisible(false);
			continue;
		}
		visible++;
	}

	if (visible && currEmoteSelected >= visible)
		currEmoteSelected = visible-1;

	for (u32 i=0; i<visible; i++)
	{
		mp3_fill_buffer();

		u32 ind = currPage*4 + i;

		std::string buttonFile = "/data/ao-nds/characters/" + pCourtUI->getCurrChar().name + "/emotions/button" + std::to_string(ind+1) + "_off";
		bool exists = fileExists(buttonFile+".img.bin");
		btnPalettes[i] = (exists) ? readFile(buttonFile+".pal.bin") : 0;
		mp3_fill_buffer();

		btn_emote[i]->setImage((exists) ? buttonFile : "", 64, 64, 7+i);
		btn_emote[i]->setVisible(true);

		if (i == currEmoteSelected)
		{
			// make emote button dark
			vramSetBankI(VRAM_I_LCD);
			for (u32 j=0; j<256; j++)
			{
				u8 r=0, g=0, b=0;
				fromRGB15(VRAM_I_EXT_SPR_PALETTE[7+i][j], r, g, b);
				VRAM_I_EXT_SPR_PALETTE[7+i][j] = RGB15(r>>1, g>>1, b>>1);
				mp3_fill_buffer();
			}
			vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
		}
	}

	u32 maxPages = (u32)ceil(pCourtUI->getCharEmotes().size()/4.f);
	btn_prevPage->setVisible(currPage > 0);
	spr_arrowLeft->setVisible(currPage > 0);
	btn_nextPage->setVisible(currPage+1 < maxPages);
	spr_arrowRight->setVisible(currPage+1 < maxPages);
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
	btn_prevPage->setVisible(false);
	btn_nextPage->setVisible(false);
	for (int i=0; i<4; i++) btn_emote[i]->setVisible(false);
	for (int i=0; i<2; i++) spr_bars[i]->setVisible(false);
	spr_arrowLeft->setVisible(false);
	spr_arrowRight->setVisible(false);
	lbl_showname->setVisible(false);
	lbl_color->setVisible(false);
}

void UICourtIC::onBackClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndCancel, SoundFormat_16Bit, pSelf->pCourtUI->sndCancelSize, 32000, 127, 64, false, 0);

	pSelf->pCourtUI->changeScreen(new UICourtIngameMenu(pSelf->pCourtUI));
}

void UICourtIC::onCourtRecord(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
}

void UICourtIC::onShoutsToggled(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->currShout = (pSelf->currShout+1) % 5;

	pSelf->btn_shouts->setFrame(2 + pSelf->currShout);
}

void UICourtIC::onPairClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
}

void UICourtIC::onMuteClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
}

void UICourtIC::onOptionsToggled(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->displayingOptions = !pSelf->displayingOptions;

	delete[] pSelf->bgPal;
	u8* bgTiles;
	u8* bgMap;

	if (pSelf->displayingOptions)
	{
		bgTiles = readFile("nitro:/bg_icExt.img.bin", &pSelf->bgTilesLen);
		bgMap = readFile("nitro:/bg_icExt.map.bin");
		pSelf->bgPal = readFile("nitro:/bg_icExt.pal.bin");

		pSelf->lbl_showname->setVisible(false);
		pSelf->btn_additive->setVisible(true);
		pSelf->btn_preanim->setVisible(true);
		pSelf->btn_immediate->setVisible(true);
		pSelf->btn_flip->setVisible(true);
		pSelf->btn_shake->setVisible(true);
		pSelf->btn_flash->setVisible(true);
		pSelf->btn_pair->setVisible(false);
		pSelf->btn_mute->setVisible(false);

		pSelf->btn_optionsToggle->setPos(pSelf->btn_optionsToggle->getX(), 153);
	}
	else
	{
		bgTiles = readFile("nitro:/bg_ic.img.bin", &pSelf->bgTilesLen);
		bgMap = readFile("nitro:/bg_ic.map.bin");
		pSelf->bgPal = readFile("nitro:/bg_ic.pal.bin");

		pSelf->lbl_showname->setVisible(true);
		pSelf->btn_additive->setVisible(false);
		pSelf->btn_preanim->setVisible(false);
		pSelf->btn_immediate->setVisible(false);
		pSelf->btn_flip->setVisible(false);
		pSelf->btn_shake->setVisible(false);
		pSelf->btn_flash->setVisible(false);
		pSelf->btn_pair->setVisible(true);
		pSelf->btn_mute->setVisible(true);

		pSelf->btn_optionsToggle->setPos(pSelf->btn_optionsToggle->getX(), 113);
	}

	dmaCopy(bgTiles, bgGetGfxPtr(pSelf->bgIndex), pSelf->bgTilesLen);
	dmaCopy(bgMap, bgGetMapPtr(pSelf->bgIndex), 1536);
	dmaCopy(pSelf->bgPal, BG_PALETTE_SUB, 512);

	pSelf->btn_optionsToggle->setFrame(pSelf->displayingOptions);
}

void UICourtIC::onSideClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->pCourtUI->icControls.side = (pSelf->pCourtUI->icControls.side + 1) % 6;
	pSelf->btn_sideSelect->setFrame(pSelf->pCourtUI->icControls.side);
	pSelf->btn_tools->setVisible(pSelf->pCourtUI->icControls.side == 5); // show tools button on Judge pos
}

void UICourtIC::onToolsClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);
}

void UICourtIC::onAdditiveClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->pCourtUI->icControls.additive ^= 1;
	pSelf->btn_additive->setFrame(pSelf->pCourtUI->icControls.additive);
}

void UICourtIC::onPreanimClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->pCourtUI->icControls.preanim ^= 1;
	pSelf->btn_preanim->setFrame(2+pSelf->pCourtUI->icControls.preanim);
}

void UICourtIC::onImmediateClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->pCourtUI->icControls.immediate ^= 1;
	pSelf->btn_immediate->setFrame(4+pSelf->pCourtUI->icControls.immediate);
}

void UICourtIC::onFlipClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->pCourtUI->icControls.flip ^= 1;
	pSelf->btn_flip->setFrame(6+pSelf->pCourtUI->icControls.flip);
}

void UICourtIC::onShakeClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->pCourtUI->icControls.shake ^= 1;
	pSelf->btn_shake->setFrame(8+pSelf->pCourtUI->icControls.shake);
}

void UICourtIC::onFlashClicked(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndSelect, SoundFormat_16Bit, pSelf->pCourtUI->sndSelectSize, 32000, 127, 64, false, 0);

	pSelf->pCourtUI->icControls.flash ^= 1;
	pSelf->btn_flash->setFrame(10+pSelf->pCourtUI->icControls.flash);
}

void UICourtIC::onPrevPage(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndEvPage, SoundFormat_16Bit, pSelf->pCourtUI->sndEvPageSize, 32000, 127, 64, false, 0);

	pSelf->currPage--;
	pSelf->reloadPage();
}

void UICourtIC::onNextPage(void* pUserData)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;
	soundPlaySample(pSelf->pCourtUI->sndEvPage, SoundFormat_16Bit, pSelf->pCourtUI->sndEvPageSize, 32000, 127, 64, false, 0);

	pSelf->currPage++;
	pSelf->reloadPage();
}

void UICourtIC::onEmoteClicked(void* pUserData)
{
	emoteBtnData* pData = (emoteBtnData*)pUserData;
	UICourtIC* pSelf = pData->pObj;

	if (pData->btnInd == pSelf->currEmoteSelected) return;

	// unlock VRAM so it can be accessed here
	vramSetBankI(VRAM_I_LCD);

	// restore previous emote button palette
	dmaCopy(pSelf->btnPalettes[pSelf->currEmoteSelected], &VRAM_I_EXT_SPR_PALETTE[7+pSelf->currEmoteSelected], 512);

	pSelf->currEmoteSelected = pData->btnInd;

	// make emote button dark
	for (u32 j=0; j<256; j++)
	{
		u8 r=0, g=0, b=0;
		fromRGB15(VRAM_I_EXT_SPR_PALETTE[7+pSelf->currEmoteSelected][j], r, g, b);
		VRAM_I_EXT_SPR_PALETTE[7+pSelf->currEmoteSelected][j] = RGB15(r>>1, g>>1, b>>1);
		mp3_fill_buffer();
	}

	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);
}

void UICourtIC::onMessagePV(void* pUserData, std::string msg)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;

	if (pSelf->pCourtUI->showname.empty())
		pSelf->lbl_showname->setText(pSelf->pCourtUI->getCurrChar().name.c_str());
	pSelf->currPage = 0;
	pSelf->reloadPage();
}

void UICourtIC::onMessageHP(void* pUserData, std::string msg)
{
	UICourtIC* pSelf = (UICourtIC*)pUserData;

	pSelf->reloadBars();
}