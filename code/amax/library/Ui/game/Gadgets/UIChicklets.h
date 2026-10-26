#ifndef UICHICKLETS_H
#define UICHICKLETS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>
#include <ui/Graphics/Font/FontStyle.h>

#include "ui/UIGameDefines.h"

namespace UI
{

// -----------
// UIChicklets
// -----------
class UIChicklets : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_Chicklets, "CHICKLETS" )

public:
	void	Init();
	void	Setup( lua_State* pState );
	void	FrameUpdate();
	void	Render();

	void	SetValue( const F32 fValue );

public: // exposed for Xt
	S32					m_sNumSegments;
	F32					m_fMinValue;
	F32					m_fMaxValue;
	C_FontStylePasses	m_FontStylePasses;
	XtSz8				m_xtColourOff;
	XtSz8				m_xtColourOn;
	F32					m_fSegmentGapX;
	F32					m_fTextGapX;
	F32					m_fTextHeight;
	F32					m_fEffectMult;
	XtLocId				m_xtTextId;

private:
	F32					m_fCurrentValue;
	F32					m_fTargetValue;
	C_UISpriteColours	m_ColourOff;
	C_UISpriteColours	m_ColourOn;
	C_TextStringID		m_TextId;

	bool				m_bHasResolvedColours;
};

};	// namespace UI

#endif /* UICHICKLETS_H */
