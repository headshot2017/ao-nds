#ifndef JUDGE_H_INCLUDED
#define JUDGE_H_INCLUDED

#include "ui/uicourt.h"

#include "ui/button.h"

class UICourtJudge : public UISubScreen
{
	UIButton* btn_back;
	UIButton* btn_courtRecord;
	UIButton* btn_barDefenseMinus;
	UIButton* btn_barDefensePlus;
	UIButton* btn_barProsecutorMinus;
	UIButton* btn_barProsecutorPlus;
	UIButton* btn_witnessTestimony;
	UIButton* btn_crossExamination;
	UIButton* btn_guilty;
	UIButton* btn_notGuilty;

	UIButton* spr_bars[2];

	int cbHP;

public:
	UICourtJudge(UIScreenCourt* courtUI) : UISubScreen(courtUI) {}
	~UICourtJudge();

	void init();
	void updateInput();
	void update();
	void reloadBars();

	static void onBackClicked(void* pUserData);
	static void onCourtRecord(void* pUserData);
	static void onBarDefenseMinus(void* pUserData);
	static void onBarDefensePlus(void* pUserData);
	static void onBarProsecutorMinus(void* pUserData);
	static void onBarProsecutorPlus(void* pUserData);
	static void onWitnessTestimonyClicked(void* pUserData);
	static void onCrossExaminationClicked(void* pUserData);
	static void onGuiltyClicked(void* pUserData);
	static void onNotGuiltyClicked(void* pUserData);

	static void onMessageHP(void* pUserData, std::string msg);
};

#endif // JUDGE_H_INCLUDED
