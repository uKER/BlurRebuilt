#ifndef UIREINFORCETEXT_H
#define UIREINFORCETEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/UIStaticText.h>

#include "ui/UIGameDefines.h"

namespace UI
{

// -----------
// UIReinforceText
// -----------
class UIReinforceText : public UIStaticText
{
public:
	DECLARE_GADGET( E_UIGameButtonType_ReinforceText, "REINFORCETEXT" )

public:
	void	Init();
	void	Render();

public: // exposed for Xt
	XtF32				m_Amount;
	XtF32				m_ScaleRange;
	XtF32				m_AlphaRange;
};

};	// namespace UI

#endif /* UIREINFORCETEXT_H */
