#include "ui/uidirectconn.h"

#include <nds/dma.h>

#include "sockets/aotcpsocket.h"
#include "sockets/aowebsocket.h"
#include "ui/uicourt.h"
#include "ui/uimainmenu.h"
#include "engine.h"
#include "bg_logo.h"
#include "spr_radioBox.h"

UIScreenDirectConn::~UIScreenDirectConn()
{
	dmaFillHalfWords(0, bgGetGfxPtr(bgIndex), bg_logoTilesLen);
	dmaFillHalfWords(0, bgGetMapPtr(bgIndex), bg_logoMapLen);
	dmaFillHalfWords(0, BG_PALETTE, 512);

	delete kb_ipInput;
	delete btn_ws;
	delete btn_tcp;
	delete lbl_ws;
	delete lbl_tcp;
}

void UIScreenDirectConn::init()
{
	bgIndex = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
	dmaCopy(bg_logoTiles, bgGetGfxPtr(bgIndex), bg_logoTilesLen);
	dmaCopy(bg_logoMap, bgGetMapPtr(bgIndex), bg_logoMapLen);
	dmaCopy(bg_logoPal, BG_PALETTE, bg_logoPalLen);

	kb_ipInput = new AOkeyboard(ip, 2, 0, 0);
	kb_ipInput->setInputYOffset(16);

	btn_ws = new UIButton(&oamSub, (u8*)spr_radioBoxTiles+(16*16), (u8*)spr_radioBoxPal, kb_ipInput->nextOamInd(), 1, 1, SpriteSize_16x16, 256-76, 8, 16, 16, 16, 16, 1);
	btn_tcp = new UIButton(&oamSub, (u8*)spr_radioBoxTiles, (u8*)spr_radioBoxPal, btn_ws->nextOamInd(), 1, 1, SpriteSize_16x16, 256-76, 8+20, 16, 16, 16, 16, 1);
	lbl_ws = new UILabel(&oamSub, btn_tcp->nextOamInd(), 2, 1, RGB15(31,31,31), 2, 0);
	lbl_tcp = new UILabel(&oamSub, lbl_ws->nextOamInd(), 2, 1, RGB15(31,31,31), 2, 0);

	btn_ws->connect(onWSButton, this);
	btn_tcp->connect(onTcpButton, this);
	lbl_ws->setPos(btn_ws->getX()+20, btn_ws->getY()+2, false);
	lbl_ws->setText("WebSocket");
	lbl_tcp->setPos(btn_tcp->getX()+20, btn_tcp->getY()+2, false);
	lbl_tcp->setText("TCP");
	useWS = true;

}

void UIScreenDirectConn::updateInput()
{
	btn_ws->updateInput();
	btn_tcp->updateInput();

	int result = kb_ipInput->updateInput();
	if (result == -1)
		gEngine->changeScreen(new UIScreenMainMenu);
	else if (result == 1)
	{
		if (useWS)
		{
			AOwebSocket* sock = new AOwebSocket;
			ip = "ws://" + ip;

			sock->connectIP(ip);
			gEngine->setSocket(sock);
		}
		else
		{
			size_t pos = ip.find(":");
			std::string addr = ip.substr(0, pos);
			u16 port = (pos != std::string::npos) ? std::stoi(ip.substr(pos+1)) : 27017;

			AOtcpSocket* sock = new AOtcpSocket;
			sock->connectIP(addr, port);
			gEngine->setSocket(sock);
		}

		gEngine->changeScreen(new UIScreenCourt);
	}
}

void UIScreenDirectConn::update()
{
	if (!gEngine->isFading() && !kb_ipInput->isVisible())
		kb_ipInput->show("Enter IP address");
}

void UIScreenDirectConn::onWSButton(void* pUserData)
{
	UIScreenDirectConn* pSelf = (UIScreenDirectConn*)pUserData;

	pSelf->useWS = true;
	pSelf->btn_ws->setImage((u8*)spr_radioBoxTiles+(16*16), (u8*)spr_radioBoxPal, 16, 16, 1);
	pSelf->btn_tcp->setImage((u8*)spr_radioBoxTiles, (u8*)spr_radioBoxPal, 16, 16, 1);
}

void UIScreenDirectConn::onTcpButton(void* pUserData)
{
	UIScreenDirectConn* pSelf = (UIScreenDirectConn*)pUserData;

	pSelf->useWS = false;
	pSelf->btn_ws->setImage((u8*)spr_radioBoxTiles, (u8*)spr_radioBoxPal, 16, 16, 1);
	pSelf->btn_tcp->setImage((u8*)spr_radioBoxTiles+(16*16), (u8*)spr_radioBoxPal, 16, 16, 1);
}
