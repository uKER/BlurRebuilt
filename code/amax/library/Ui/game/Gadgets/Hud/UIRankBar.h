#ifndef UIRANKBAR_H
#define UIRANKBAR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>
#include "ui/UIGameDefines.h"

namespace Game
{
class C_MultiplayerProfile;
}

namespace UI
{

// --------
// UIRankBar
// --------
class UIRankBar : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_RankBar, "RankBar" )

public:
	void	Init();
	void	Setup( lua_State* pState );
	void	Render();

public:
	XtSz8							m_xtColourBacking;

private:
	Game::C_MultiplayerProfile*		m_pMpProfile;
	C_UISpriteColours				m_BackingColour;
	S32								m_sCurrentRank;
};

}	// namespace UI

#endif /* UIRANKBAR_H */
