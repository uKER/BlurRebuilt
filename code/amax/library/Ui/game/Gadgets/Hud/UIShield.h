#ifndef UIHUDSHIELD_H
#define UIHUDSHIELD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>
#include "ui/UIGameDefines.h"

namespace UI
{

// --------
// UIShield
// --------
class UIShield : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_Shield, "SHIELD" )

public:
	UIShield() : C_GadgetBase(), m_sLocalPlayerIndex( -1 ) {  }

public:
	void	Init();
	void	Setup( lua_State* pState );
	bool	MessageHandler( const Messaging::C_MessageBase* pMessage );
	void	Render();

public: // exposed for Xt
	XtSz8				m_xtColourEmpty;
	F32					m_fColourEmptyAlpha;
	S32					m_sNumSegments;
	F32					m_fGapMult;
	
private:
	S32					m_sLocalPlayerIndex;
	F32					m_fSegmentSizeX;

	C_UISpriteColours	m_ColourEmpty;	
};

};	// namespace UI

#endif /* UIHUDSHIELD_H */
