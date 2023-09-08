#ifndef UICOURT_H_INCLUDED
#define UICOURT_H_INCLUDED

#include <string>

#include "uiscreen.h"
#include "courtroom/courtroom.h"

class UIScreenCourt;
class UISubScreen
{
public:
	UISubScreen(UIScreenCourt* courtUI) : pCourtUI(courtUI) {}
	virtual ~UISubScreen() {}

	virtual void init() {}
	virtual void updateInput() {}
	virtual void update() {}

protected:
	UIScreenCourt* pCourtUI;
};

class UIScreenCourt : public UIScreen
{
	Courtroom* court;
	UISubScreen* subScreen;
	UISubScreen* nextScreen;

public:
	UIScreenCourt();
	~UIScreenCourt();

	void init();
	void updateInput();
	void update();
	void changeScreen(UISubScreen* newScreen);

	static void onMessageDecryptor(void* pUserData, std::string msg);
	static void onMessageID(void* pUserData, std::string msg);
	static void onMessagePN(void* pUserData, std::string msg);
	static void onMessageSI(void* pUserData, std::string msg);
	static void onMessageSC(void* pUserData, std::string msg);
	static void onMessageSM(void* pUserData, std::string msg);
	static void onMessageBN(void* pUserData, std::string msg);
	static void onMessageMC(void* pUserData, std::string msg);
	static void onMessageMS(void* pUserData, std::string msg);
};

#endif // UICOURT_H_INCLUDED
