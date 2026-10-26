#ifndef UIHUDMPSPEAKING_H
#define UIHUDMPSPEAKING_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIGameDefines.h"
#include <ui/Game/Gadgets/Buttons.h>
#include <ui/Graphics/Font/FontStyle.h>

#include <Network/Misc/NetDefines.h>

namespace RacerNet { class C_NetPlayer; }

namespace UI
{

// ------------------
// C_UIMpSpeakingInfo
// ------------------
class C_UIMpSpeakingInfo
{
public:
	C_UIMpSpeakingInfo()										{ Clear(); }

public:
	bool	operator < ( const C_UIMpSpeakingInfo& o ) const	{ return ( m_fTime < o.m_fTime ); }
	void	Clear()												{ m_bActive = false; m_fTime = 0.0f; m_pPlayer = NULL; }

public:
	bool					m_bActive;
	F32						m_fTime;
	RacerNet::C_NetPlayer*	m_pPlayer;
};

// ---------------------
// UIMultiplayerSpeaking
// ---------------------
class UIMultiplayerSpeaking : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_MultiplayerSpeaking, "MP_SPEAKING" )

public:
	void				Init();
	void				Setup( lua_State* pState );
	void				FrameUpdate();
	void				Render();

public: // exposed for Xt
	C_FontStylePasses	m_FontStylePasses;
	F32					m_fIconScale;
	XtSz8				m_xtColourIcon;
	F32					m_fMultGapX;
	S32					m_sMaxToShow;

private:
	C_UISpriteColours	m_ColourIcon;

	Btl::Vector< C_UIMpSpeakingInfo, Btl::VectorParameters_Array< RacerNet::NET_MAX_VEHICLES > >	m_SortInfo;
};

};	// namespace UI

#endif /* UIHUDMPSPEAKING_H */
