
#ifndef UI_UISTATIC_GRAPHIC_H
#define UI_UISTATIC_GRAPHIC_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"

namespace UI
{

// Basic textured quad 
class UIStaticGraphic : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_StaticGraphic, "GRAPHIC" )

	virtual					~UIStaticGraphic( ) { }
	virtual void 			Setup( lua_State* pState );
	virtual void 			Render();
	virtual void			FrameUpdate();

#ifdef PLATFORM_PC
private:
	C_UIColour				m_MouseOverToColour;
	F32						m_fMouseOverEffectTime;
	F32						m_fMouseOverTimer;
#endif
};

} // namespace UI

#endif // UI_UISTATIC_GRAPHIC_H
