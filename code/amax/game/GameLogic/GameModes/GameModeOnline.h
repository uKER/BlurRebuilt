#ifndef GAMEMODE_ONLINE_H
#define GAMEMODE_ONLINE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "GameLogic/GameModes/GameModeNetwork.h"

namespace Game
{

class C_GameModeOnline : public C_GameModeNetwork
{
public:
	C_GameModeOnline();

	bool	IsOnline() const { return true; }

	bool	IsRanked() const { return m_Ranked; }

	void	SetRanked( const bool bRanked )
			{
				m_Ranked = bRanked;
			}

private:
	bool m_Ranked;
};

} // namespace Game

#endif // GAMEMODE_SPLITSCREEN_H
