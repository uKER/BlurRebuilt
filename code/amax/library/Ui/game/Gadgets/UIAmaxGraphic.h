
#ifndef UI_AMAX_GRAPHIC_H
#define UI_AMAX_GRAPHIC_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/UIStaticGraphic.h>

#include "ui\UIGameDefines.h"

namespace UI
{

class UIAmaxGraphic : public UIStaticGraphic
{
public:
	DECLARE_GADGET( E_UIGameButtonType_AmaxGraphic, "AMAX_GRAPHIC" )

	virtual					~UIAmaxGraphic( ) { }
	virtual void 			Setup( lua_State* pState );

// Exposed for Xt
public:
	XtSz8					m_LerpColFromName;
	XtSz8					m_LerpColToName;

private:
};

}; // namespace UI

#endif // UI_AMAX_GRAPHIC_H
