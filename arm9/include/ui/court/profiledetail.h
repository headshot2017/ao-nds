#ifndef PROFILEDETAIL_H_INCLUDED
#define PROFILEDETAIL_H_INCLUDED

#include "ui/uicourt.h"
#include "ui/button.h"
#include "ui/label.h"

class UICourtProfileDetail : public UISubScreen
{
	u32 currProfile;
	u32 currProfileID;
	bool wasInPrivateEvidence;

	UIButton* btn_back;
	UIButton* btn_profilesEvidence;
	UIButton* btn_prevPage;
	UIButton* btn_nextPage;
	UIButton* btn_kick;
	UIButton* btn_ban;

	UILabel* lbl_name;
	UILabel* lbl_desc;

	UIButton* spr_profile;

	int cbPR;
	int cbPU;

public:
	UICourtProfileDetail(UIScreenCourt* courtUI, u32 ind, bool isPriv) :
		UISubScreen(courtUI), currProfile(ind), wasInPrivateEvidence(isPriv) {}

	~UICourtProfileDetail();

	void init();
	void updateInput();
	void update();

	void reloadPage();

	static void onBackClicked(void* pUserData);
	static void onProfilesEvidenceClicked(void* pUserData);
	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onKick(void* pUserData);
	static void onBan(void* pUserData);

	static void onMessagePR(void* pUserData, std::string msg);
	static void onMessagePU(void* pUserData, std::string msg);
};

#endif // PROFILEDETAIL_H_INCLUDED
