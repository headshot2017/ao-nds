#ifndef EVIDENCEDETAIL_H_INCLUDED
#define EVIDENCEDETAIL_H_INCLUDED

#include "ui/uicourt.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/keyboard.h"

class UICourtEvidenceDetail : public UISubScreen
{
	u32 currEvidence;
	bool isPrivate;
	bool adding;

	UIButton* btn_back;
	UIButton* btn_privatePublic;
	UIButton* btn_profilesEvidence;
	UIButton* btn_topButton;
	UIButton* btn_delete;
	UIButton* btn_transfer;
	UIButton* btn_descUp;
	UIButton* btn_descDown;
	UIButton* btn_prevPage;
	UIButton* btn_nextPage;

	UILabel* lbl_name;
	UILabel* lbl_imageName;
	UILabel* lbl_desc;

	UIButton* spr_evidence;

	AOkeyboard* kb_input;
	UILabel* inputting;

	u32 scrollPos;
	u32 scrollPosOld;
	std::u16string currName;
	std::u16string currDesc;
	std::string currImage;
	std::vector<std::u16string> renderDesc;

	int cbLE;

public:
	UICourtEvidenceDetail(UIScreenCourt* courtUI, u32 ind, bool isPriv, std::u16string name=u"", std::u16string desc=u"", std::string img="") :
		UISubScreen(courtUI), currEvidence(ind), isPrivate(isPriv), currName(name), currDesc(desc), currImage(img) {}

	~UICourtEvidenceDetail();

	void init();
	void updateInput();
	void update();

	void hideEverything();
	void showEverything();
	void reloadPage();

	void setScroll(u32 i);
	void reloadDesc(bool all=false);

	static void onBackClicked(void* pUserData);
	static void onProfilesEvidenceClicked(void* pUserData);
	static void onPrivatePublicClicked(void* pUserData);
	static void onTopButtonClicked(void* pUserData);
	static void onDeleteClicked(void* pUserData);
	static void onTransferClicked(void* pUserData);
	static void onDescUp(void* pUserData);
	static void onDescDown(void* pUserData);
	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onEvidenceImage(void* pUserData);

	static void onMessageLE(void* pUserData, std::string msg);
};

#endif // EVIDENCEDETAIL_H_INCLUDED
