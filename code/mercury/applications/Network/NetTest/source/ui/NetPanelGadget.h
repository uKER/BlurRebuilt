#ifndef NETPANELGADGET_H
#define NETPANELGADGET_H

#ifdef _MSC_VER
#pragma once
#endif


#include "ui/Game/Gadgets/ButtonController.h"
#include "gadgets/UiNetPanel_Main.h"
#include "uiManager.h"

struct NetPanelGadget: public UI::C_GadgetBase
{
public:
	NetPanelGadget( );
	~NetPanelGadget( );

	static const UI::UIButtonTypeID	TypeID()	{ return UI::E_ShooterButtonType_NetPanel; }
	static const Util::C_String32	TypeName()	{ return "NETWORKPANEL"; }

	virtual void Init();
	virtual void Setup();
	virtual void FrameUpdate();
	virtual void Render();
	virtual void Exit();
	virtual bool MessageHandler(const Messaging::C_MessageBase *msg);

	UiNetPanel::NetPanel m_panel;
};

#endif //NETPANELGADGET_H