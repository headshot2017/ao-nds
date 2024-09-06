#ifndef COURTRECORD_H_INCLUDED
#define COURTRECORD_H_INCLUDED

#include <nds/ndstypes.h>

#include "ui/uicourt.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/selectcross.h"

class UICourtEvidence : public UISubScreen
{
	u32 bg_evidenceTilesLen;
	u32 bg_profilesTilesLen;

	u8* bg_evidenceTiles;
	u8* bg_evidenceMap;
	u8* bg_evidencePal;
	u8* bg_profilesTiles;
	u8* bg_profilesMap;
	u8* bg_profilesPal;

	u32 currPage;
	int currEviSelected;
	bool isPrivate;
	bool isProfiles;

	UIButton* btn_pageLeft;
	UIButton* btn_pageRight;
	UIButton* btn_back;
	UIButton* btn_present;
	UIButton* btn_privatePublic;
	UIButton* btn_profilesEvidence;
	UILabel* lbl_name;
	UISelectCross* sel_btn;

	UIButton* btn_evidence[8];
	int cbLE;
	int cbPR;

public:
	UICourtEvidence(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtEvidence();

	void init();
	void updateInput();
	void update();

	void reloadPage();

	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onBackClicked(void* pUserData);
	static void onPresentClicked(void* pUserData);
	static void onPrivatePublicClicked(void* pUserData);
	static void onProfilesEvidenceClicked(void* pUserData);
	static void onEvidenceClicked(void* pUserData);

	static void onMessageLE(void* pUserData, std::string msg);
	static void onMessagePR(void* pUserData, std::string msg);
};

#endif // COURTRECORD_H_INCLUDED
