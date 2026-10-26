#ifndef UIHUDTEAMTUGOFWAR_H
#define UIHUDTEAMTUGOFWAR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>
#include "ui/UIGameDefines.h"

namespace UI
{

// --------------
// UITeamTugOfWar
// --------------
class UITeamTugOfWar : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_TeamTugOfWar, "TEAM_TUG_OF_WAR" )

public:
	UITeamTugOfWar() : C_GadgetBase(), m_sLocalPlayerIndex( -1 ) {  }

public:
	void	Init();
	void	Setup( lua_State* pState );
	bool	MessageHandler( const Messaging::C_MessageBase* pMessage );
	void	FrameUpdate();
	void	Render();

public: // exposed for Xt
	//
	
private:
	S32		m_sLocalPlayerIndex;

	U8		m_uTeamMe;

	U32		m_uTeamPointsA;
	U32		m_uTeamPointsB;

	C_UISpriteColours	m_TeamColourA;
	C_UISpriteColours	m_TeamColourB;
};

};	// namespace UI

#endif /* UIHUDTEAMTUGOFWAR_H */
