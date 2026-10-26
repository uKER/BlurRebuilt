
#ifndef UI_MULTIPLAYER_CACHE_H
#define UI_MULTIPLAYER_CACHE_H

#include "GameLogic/GameData/MultiplayerData.h"
#include "GameLogic/GameModes/GameModeDefines.h"

namespace UI
{


/*---------------------------------------------------------------------------------------
	Class : C_MultiplayerCache
---------------------------------------------------------------------------------------*/
class C_MultiplayerCache
{
private :
	C_MultiplayerCache( const C_MultiplayerCache& lhv );
	C_MultiplayerCache operator = ( const C_MultiplayerCache& lhv );

public :
	C_MultiplayerCache(){};

	static C_MultiplayerCache&			Instance()
										{
											return m_sInstance;
										}

	void								ClearEventCache();
	
	const MultiplayerEvent*				Event( const U32 uIndex ) const
										{
											return m_Events[uIndex];
										}

	const MultiplayerEvent*				GetEventByID( const U32 uID ) const;

	const MultiplayerEvent*				CurrentEvent() const
										{
											if (IsCurrentEventValid())
												return m_Events[m_CurrentEventIndex];
											else 
												return NULL;
										}

	const S32							NumEvents() const
										{
											return m_Events.Size();
										}

	const TEventID						GetCurrentEventID() const
										{
											if (IsCurrentEventValid() && m_Events[m_CurrentEventIndex])
												return m_Events[m_CurrentEventIndex]->ID;
											else
											{
												BIZ_ASSERT_MSG( IsCurrentEventValid(), "NO CACHED EVENTS" );
#ifdef PLATFORM_PS3
												return 0xffffffffffffffffLLU;
#else
												return _UI64_MAX;
#endif
											}
										}

	const S32							GetCurrentEventIndex() const
										{
											BIZ_ASSERT_MSG( m_CurrentEventIndex < m_Events.Size(), "NO CACHED EVENTS" );
											return m_CurrentEventIndex;
										}

	bool								IsCurrentEventValid() const;

	void								SetCurrentEventIndex( const S32 sIndex )
										{
											m_CurrentEventIndex = sIndex;
										}

	void								CachePlaylists( const Game::E_GameMode eMode );

	void								CacheNetRaceSettings();
	bool								CachePlaylistByID( const U32 uEventID );
	bool								CacheInviteSettings( const bool bParty );

private:
	typedef Btl::Vector< XtHandle< MultiplayerEvent >, Btl::VectorParameters_Static< MAX_EVENT_CACHE > > TEventList;

	static C_MultiplayerCache			m_sInstance;

	S32									m_CurrentEventIndex;
	TEventList							m_Events;

};

}; // namespace UI

#endif // UI_MULTIPLAYER_CACHE_H