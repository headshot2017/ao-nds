#ifndef MODERATORDIALOG_H_INCLUDED
#define MODERATORDIALOG_H_INCLUDED

#include "ui/uicourt.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/keyboard.h"

class UICourtModeratorDialog : public UISubScreen
{
	u32 currProfile;
	u32 currProfileID;
	bool wasInPrivateEvidence;
	bool isBan;
	bool durationsChanged;

	UIButton* btn_back;
	UIButton* btn_confirm;

	UILabel* lbl_name;
	UILabel* lbl_desc;

	UIButton* spr_profile;

	// duration (ban only)
	UILabel* lbl_duration;
	UILabel* lbl_days;
	UILabel* lbl_daysValue;
	UIButton* btn_daysInput;
	UILabel* lbl_hours;
	UILabel* lbl_hoursValue;
	UIButton* btn_hoursInput;
	UILabel* lbl_minutes;
	UILabel* lbl_minutesValue;
	UIButton* btn_minutesInput;
	UILabel* lbl_perma;
	UIButton* btn_perma;

	// reason
	UILabel* lbl_reason;
	UILabel* lbl_reasonValue;
	UIButton* btn_reasonInput;

	AOkeyboard* kb_input;
	UILabel* inputting;

public:
	UICourtModeratorDialog(UIScreenCourt* courtUI, u32 ind, bool isPriv, bool ban) :
		UISubScreen(courtUI), currProfile(ind), wasInPrivateEvidence(isPriv), isBan(ban) {}

	~UICourtModeratorDialog();

	void init();
	void updateInput();
	void update();

	void hideEverything();
	void showEverything();
	void fillInfo();

	static void onBackClicked(void* pUserData);
	static void onConfirmClicked(void* pUserData);
	static void onDaysInput(void* pUserData);
	static void onHoursInput(void* pUserData);
	static void onMinutesInput(void* pUserData);
	static void onPermaToggled(void* pUserData);
	static void onReasonInput(void* pUserData);
};

#endif // MODERATORDIALOG_H_INCLUDED
