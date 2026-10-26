#ifndef _LOBBY_DATAFINDER_H_
#define _LOBBY_DATAFINDER_H_

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UIDataFinder.h>
#include <ui/Game/UIScreenControl.h>
#include <ui/Game/UIVariableStore.h>

namespace UI
{

// Forward declare
class UILobby;

class C_LobbyDataFinder : public C_UIBaseGameDataFinder
{
public:
	static C_LobbyDataFinder&	Instance()						{ static C_LobbyDataFinder lobbyDataFinder; return lobbyDataFinder; }

private:
	C_LobbyDataFinder() : C_UIBaseGameDataFinder( "LOBBY_" )		{  }
	C_LobbyDataFinder( const C_LobbyDataFinder& o )					{  }

public:
	void						Init();
	const WCHAR*				GetVariableText( const char* pVariableName );
	void						SetLobbyPtr( const UILobby* pLobby ) { m_pLobby = pLobby; }

private:
	const UILobby*				m_pLobby; // So we can get slot information for rendering names etc
};

}; //namespace UI

#endif /* _LOBBY_DATAFINDER_H_ */
