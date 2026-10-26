#ifndef UIMINORSCORELOG_H
#define UIMINORSCORELOG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>
#include <ui/Graphics/Font/FontStyle.h>
#include <ui/Util/WideString.h>

#include "ui/UIGameDefines.h"

namespace UI
{

// ---------------------
// C_UIMinorScoreMessage
// ---------------------
class C_UIMinorScoreMessage : public Btl::ListElementDouble< C_UIMinorScoreMessage >
{
public:
	C_UIMinorScoreMessage()
		: m_String()
		, m_fTime01( 0.0f )
		, m_bIsMajor( false )
	{
		//
	}

public:
	C_WideString256		m_String;
	F32					m_fTime01;
	bool				m_bIsMajor;
};

// ---------------
// UIMinorScoreLog
// ---------------
class UIMinorScoreLog : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_MinorScoreLog, "MINOR_SCORE_LOG" )

public:
	virtual	~UIMinorScoreLog();

public:
	void 	Init();
	void 	Setup( lua_State* pState );
	bool	MessageHandler( const Messaging::C_MessageBase* pMessage );
	void	FrameUpdate();
	void 	Render();

private:
	void	ProcessMinor( const Messaging::C_MessageBase* pMessage );

public: // exposed for Xt
	C_FontStylePasses		m_FontStylePasses;
	F32						m_fTimerShow;
	F32						m_fTimerFade;
	S32						m_sMaxItems;
	XtSz8					m_xtColourMajor;

private:
	S32						m_sLocalPlayerIndex;
	S8						m_sVehicleIndex;
	C_UISpriteColours		m_ColourMajor;

	Btl::List< C_UIMinorScoreMessage* >		m_Log;
};

}; // namespace UI

#endif /* UIMINORSCORELOG_H */
