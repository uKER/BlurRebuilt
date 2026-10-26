// PR - not used - TODO - REMOVE ME!
#ifndef UIRIVALHUD_H
#define UIRIVALHUD_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIGameDefines.h"
#include <ui/Game/Gadgets/Buttons.h>

namespace UI
{

enum E_RivalHud
{
	E_RivalHud_Parent = 0,
	E_RivalHud_Pos,
	E_RivalHud_Text,
	E_RivalHud_Name,
	E_RivalHud_RevsFull,
	E_RivalHud_RevsPartial,
	E_RivalHud_Damage,
	E_RivalHud_Perks,	

	E_RivalHud_Count,
};

// ----------
// UIRivalHud
// ----------
class UIRivalHud : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_RivalHud, "RIVAL_HUD" )

public:
	void	Init();
	void	Setup( lua_State* pState );
	void	Render();

public: // exposed for Xt
	//
	
private:
	Btl::Vector< C_GadgetBase*, Btl::VectorParameters_Array< E_RivalHud_Count > >	m_Objects;
};

};	// namespace UI

#endif /* UIRIVALHUD_H */
