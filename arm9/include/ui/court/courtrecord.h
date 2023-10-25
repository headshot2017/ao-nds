#ifndef COURTRECORD_H_INCLUDED
#define COURTRECORD_H_INCLUDED

#include <nds/ndstypes.h>

#include "ui/uicourt.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/selectcross.h"

class UICourtEvidence : public UISubScreen
{
	u32 currPage;
	int currEviSelected;
	bool isPrivate;

	UIButton* btn_pageLeft;
	UIButton* btn_pageRight;
	UIButton* btn_back;
	UIButton* btn_present;
	UIButton* btn_privatePublic;
	UILabel* lbl_evidence;
	UISelectCross* sel_btn;

	UIButton* btn_evidence[8];
	int cbLE;

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
	static void onEvidenceClicked(void* pUserData);

	static void onMessageLE(void* pUserData, std::string msg);
};

#endif // COURTRECORD_H_INCLUDED
