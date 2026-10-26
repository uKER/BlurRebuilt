
#include "stdafx.h"
#include "NetPanelGadget.h"
#include <ui/graphics/simple2d.h>
#include <ui/Game/Gadgets/ButtonMessages.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif


NetPanelGadget::NetPanelGadget( ) 
: C_GadgetBase() 
, m_panel()
{}

NetPanelGadget::~NetPanelGadget( )
{
}

void NetPanelGadget::Init()
{
	C_GadgetBase::Init();

	m_Selected	= true;
	m_Activated = true;
	m_panel.initialise();
}

void NetPanelGadget::Setup()
{
	C_GadgetBase::Setup( NULL );
}

bool NetPanelGadget::MessageHandler(const Messaging::C_MessageBase* message)
{
	switch(message->Type())
	{
	case UI::E_UiMsg_Render:
		{
			FrameUpdate();
			Render();
			return true;
		}
	case UI::E_UiMsg_Init:
		{
			Init();
			return true;
		}
	case UI::E_UiMsg_FrameUpdate:
		{
			FrameUpdate();
			return true;
		}
	case UI::E_UiMsg_Exit:
		{
			Exit();
			return true;
		}
	default:
		{
			return UI::C_GadgetBase::MessageHandler(message);
		}
	}
}

void NetPanelGadget::FrameUpdate()
{
	C_GadgetBase::FrameUpdate();
	m_panel.update();
}


void NetPanelGadget::Render()
{
	if(!m_Activated)
		return;

	C_GadgetBase::Render();
	m_panel.render();
}

void NetPanelGadget::Exit()
{
	C_GadgetBase::Exit();
	m_panel.shutdown();
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

