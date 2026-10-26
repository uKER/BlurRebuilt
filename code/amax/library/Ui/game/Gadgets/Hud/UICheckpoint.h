#ifndef UICHECKPOINT_H
#define UICHECKPOINT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>
#include <ui/Graphics/Font/FontStyle.h>

#include "ui/UIGameDefines.h"

namespace UI
{

// ------------
// UICheckpoint
// ------------
class UICheckpoint : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_Checkpoint, "Checkpoint" )

public:
	void	Init();
	void	Setup( lua_State* pState );
	void	Render();

public:	// exposed for Xt
	C_FontStylePasses	m_FontStylePasses;
	F32					m_fFadeNear;
	F32					m_fFadeFar;
	F32					m_fRedBoundary;
	XtSz8				m_xtColourRed;

private:
	C_Vector3Position	m_PositionOriginal;
	C_UISpriteColours	m_ColourRed;
};

}	// namespace UI

#endif /* UICHECKPOINT_H */
