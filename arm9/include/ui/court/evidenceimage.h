#ifndef EVIDENCEIMAGE_H_INCLUDED
#define EVIDENCEIMAGE_H_INCLUDED

#include <nds/ndstypes.h>

#include "ui/uicourt.h"
#include "ui/button.h"
#include "ui/label.h"
#include "ui/selectcross.h"

class UICourtEvidenceImage : public UISubScreen
{
	u32 currPage;
	int currEviSelected;

	u32 editingEvidence;
	std::u16string currName;
	std::u16string currDesc;
	std::string lastImage;
	bool isPrivate;
	bool adding;

	UIButton* btn_pageLeft;
	UIButton* btn_pageRight;
	UIButton* btn_back;
	UIButton* btn_confirm;
	UILabel* lbl_evidence;
	UISelectCross* sel_btn;

	UIButton* btn_evidence[8];

public:
	UICourtEvidenceImage(UIScreenCourt* courtUI, u32 ind, bool priv, std::u16string name, std::u16string desc, std::string img) :
		UISubScreen(courtUI), editingEvidence(ind), currName(name), currDesc(desc), lastImage(img), isPrivate(priv) {}
	~UICourtEvidenceImage();

	void init();
	void updateInput();
	void update();

	void reloadPage();

	static void onPrevPage(void* pUserData);
	static void onNextPage(void* pUserData);
	static void onBackClicked(void* pUserData);
	static void onConfirmClicked(void* pUserData);
	static void onEvidenceClicked(void* pUserData);
};

#endif // EVIDENCEIMAGE_H_INCLUDED
