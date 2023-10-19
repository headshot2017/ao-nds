#ifndef PAIR_H_INCLUDED
#define PAIR_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"
#include "ui/label.h"

class UICourtPair : public UISubScreen
{
	UIButton* btn_back;
	UIButton* btn_disable;
	UIButton* btn_charSelect;
	UIButton* btn_reset;
	UIButton* btn_xSlider;
	UIButton* btn_ySlider;
	UILabel* lbl_xValue;
	UILabel* lbl_yValue;
	UILabel* lbl_pairName;

	UIButton* spr_previewBg;
	UIButton* spr_previewChar;
	UIButton* spr_previewDesk;

	UIButton* draggingHandle;

public:
	UICourtPair(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtPair();

	void init();
	void updateInput();
	void update();

	void updateOffset();

	static void onBackClicked(void* pUserData);
	static void onDisableClicked(void* pUserData);
	static void onCharSelectClicked(void* pUserData);
	static void onResetClicked(void* pUserData);
	static void onXSliderPressed(void* pUserData);
	static void onYSliderPressed(void* pUserData);
	static void onSliderReleased(void* pUserData);
};

#endif // PAIR_H_INCLUDED
