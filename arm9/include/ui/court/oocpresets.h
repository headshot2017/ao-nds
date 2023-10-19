#ifndef OOCPRESETS_H_INCLUDED
#define OOCPRESETS_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"
#include "ui/label.h"
#include "ui/keyboard.h"

class UICourtOOCPresets : public UISubScreen
{
	UIButton* btn_back;
	UIButton* btn_addOrConfirm;
	UIButton* btn_delete;
	UIButton* btn_edit;
	UIButton* btn_prevPage;
	UIButton* btn_nextPage;
	UIButton* btn_preset[4];
	UILabel* lbl_preset[4];
	UILabel* lbl_pages;

	AOkeyboard* kb_input;

	std::vector<std::string> m_presets;
	int currPreset;
	u32 currPage;

public:
	UICourtOOCPresets(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtOOCPresets();

	void init();
	void updateInput();
	void update();

	void deselect();
	void reloadPage();
	void hideEverything();

	static void onBackClicked(void* pUserData);
	static void onAddOrConfirm(void* pUserData);
	static void onDeleteClicked(void* pUserData);
	static void onEditClicked(void* pUserData);
	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onPresetClicked(void* pUserData);

	void parsePresetsList();
	void savePresets();
};

#endif // OOCPRESETS_H_INCLUDED
