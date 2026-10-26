#ifndef UI_MP_DATA_FINDER_H
#define UI_MP_DATA_FINDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UIDataFinder.h>

namespace Game
{
	class C_LevelState;
	class C_MultiplayerProfile;
};

namespace UI
{

class C_MpDataFinder : public C_UIBaseGameDataFinder
{
public:
	static C_MpDataFinder&	Instance() 
	{ 
		return m_sInstance; 
	}

private:
	C_MpDataFinder() : C_UIBaseGameDataFinder( "MPL_" )		{}
	C_MpDataFinder( const C_MpDataFinder& o )				{}

public:
	void						Init();
	const WCHAR*				GetVariableText( const char* pVariableName );

private:
	const char* GetRaceResults( const char* pVariableName, Game::C_LevelState &levelState, Game::C_MultiplayerProfile* pMultiplayerProfile );
	const char* GetChampionshipStandings( const char* pVariableName, Game::C_LevelState &levelState, Game::C_MultiplayerProfile* pMultiplayerProfile );

	static C_MpDataFinder	m_sInstance; 
};

}; //namespace UI

#endif // UI_MP_DATA_FINDER_H
