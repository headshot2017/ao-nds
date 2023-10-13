#ifndef IC_H_INCLUDED
#define IC_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"
#include "ui/label.h"
#include "ui/keyboard.h"

class UICourtIC : public UISubScreen
{
	int bgIndex;

	UIButton* btn_back;
	UIButton* btn_courtRecord;
	UIButton* btn_shouts;
	UIButton* btn_pair;
	UIButton* btn_mute;
	UIButton* btn_optionsToggle;
	UIButton* btn_sideSelect;
	UIButton* btn_tools;
	UIButton* btn_additive;
	UIButton* btn_preanim;
	UIButton* btn_immediate;
	UIButton* btn_flip;
	UIButton* btn_shake;
	UIButton* btn_flash;
	UIButton* btn_prevPage;
	UIButton* btn_nextPage;
	UIButton* btn_emote[4];

	UIButton* spr_bars[2];
	UIButton* spr_arrowLeft;
	UIButton* spr_arrowRight;

	UILabel* lbl_showname;
	UILabel* lbl_color;

	AOkeyboard* kb_input;
	bool isWritingChat;

	u8* btnPalettes[4];

	u32 currPage;
	u32 currEmoteSelected;

	int arrowX;
	int arrowXadd;
	int arrowTicks;
	int currShout;
	bool displayingOptions;

	int cbPV;
	int cbHP;

public:
	UICourtIC(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtIC();

	void init();
	void updateInput();
	void update();

	void hideEverything();
	void reloadPage();
	void reloadBars();

	static void onBackClicked(void* pUserData);
	static void onCourtRecord(void* pUserData);
	static void onShoutsToggled(void* pUserData);
	static void onPairClicked(void* pUserData);
	static void onMuteClicked(void* pUserData);
	static void onOptionsToggled(void* pUserData);
	static void onSideClicked(void* pUserData);
	static void onToolsClicked(void* pUserData);
	static void onAdditiveClicked(void* pUserData);
	static void onPreanimClicked(void* pUserData);
	static void onImmediateClicked(void* pUserData);
	static void onFlipClicked(void* pUserData);
	static void onShakeClicked(void* pUserData);
	static void onFlashClicked(void* pUserData);
	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onEmoteClicked(void* pUserData);

	static void onMessagePV(void* pUserData, std::string msg);
	static void onMessageHP(void* pUserData, std::string msg);
};

#endif // IC_H_INCLUDED
