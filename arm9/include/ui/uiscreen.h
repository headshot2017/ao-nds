#ifndef UISCREEN_H_INCLUDED
#define UISCREEN_H_INCLUDED

class UIScreen
{
public:
	virtual ~UIScreen() {}
	virtual int ID() = 0;

	virtual void init() {}
	virtual void updateInput() {}
	virtual void update() {}
};

#endif // UISCREEN_H_INCLUDED
