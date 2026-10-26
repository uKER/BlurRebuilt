#ifndef GAMEDATAFINDER_H
#define GAMEDATAFINDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UIDataFinder.h>
#include <ui/Game/UIScreenControl.h>
#include <ui/Game/UIVariableStore.h>

namespace UI
{

class C_GameDataFinder : public C_UIBaseGameDataFinder
{
public:
	static C_GameDataFinder&	Instance()						{ static C_GameDataFinder gameDataFinder; return gameDataFinder; }

private:
	C_GameDataFinder() : C_UIBaseGameDataFinder( "GAME_" )		{  }
	C_GameDataFinder( const C_UIDataFinder& o )					{  }

public:
	void						Init();
	const WCHAR*				GetVariableText( const char* pVariableName );

private:
	//
};

}; //namespace UI

#endif /* GAMEDATAFINDER_H */
