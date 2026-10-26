
#ifndef USER_EVENT_MANAGER_H
#define USER_EVENT_MANAGER_H

#include "UI/Game/Multiplayer/MultiplayerEvent.h"

// Forward declaration
struct lua_State;

namespace UI
{

class C_UserEventManager
{
private:
	C_UserEventManager( const C_UserEventManager& lhv );
	C_UserEventManager operator = ( const C_UserEventManager& lhv );

public:
	C_UserEventManager();

	static C_UserEventManager&		Instance()
									{
										return m_sInstance;
									}

	void							Destroy();

	const bool						SaveGroups( Sys::BinarySerialiserAutoEndian& serial );
	const bool						LoadGroups( Sys::BinaryDeserialiserAutoEndian& serial );

	void							EditEventFromLua( lua_State* pState );
	void							SendEventToLua( const S32 sSlot, lua_State* pState );

	void							CacheEvents();

	void							TagEvent( TEventID uEventID )
									{
										m_UserEvents[m_EventSlot].m_ID = uEventID;
									}

	const bool						AddNewEvent();

	void							RemoveNewEvent();

	void							RemoveEvent( const S32 sSlot );

	void							ChangeEventName( const WCHAR* pStr );

	void							ChangeEventDesc( const WCHAR* pStr );

	const bool						AddNewRace();

	void							RemoveRace( const S32 sIndex );

	void							ChangeRaceCityRoute( const U32 uCityID, const U32 uRouteID );
	void							ChangeRaceMode( const E_RaceType eType );
	void							ChangeRaceLaps( const S32 sLaps );
	void							ChangeRaceTime( const S32 sTime );
	void							ChangeRaceScore( const S32 sScore );
	void							ChangeRaceEliminationEvent( const E_EliminationEvent eEvent );
	void							ChangeRaceEliminationType( const E_EliminationType eType );

	const C_MultiplayerEvent*		GetEvent( const S32 sSlot ) const
									{
										if ( sSlot < m_UserEvents.Size() )
											return &m_UserEvents[ sSlot ];
										return NULL;
									}

	const C_MultiplayerEvent*		GetCurrentEvent() const
									{
										BIZ_ASSERT_MSG( m_EventSlot != -1, "MountGroup() wasnt called" );

										return &m_UserEvents[m_EventSlot];
									}

	const C_MultiplayerRace*		GetCurrentRace() const
									{
										BIZ_ASSERT_MSG( m_EventSlot != -1, "MountGroup() wasnt called" );
										BIZ_ASSERT_MSG( m_RaceSlot != -1, "MountRace() wasnt called" );

										return &m_UserEvents[m_EventSlot].m_Races[m_RaceSlot];
									}

	const S32						GetCurrentSlot() const
									{
										BIZ_ASSERT_MSG( m_EventSlot != -1, "MountGroup() wasnt called" );
										
										return m_EventSlot;
									}

	const S32						GetCurrentRaceSlot() const
									{
										BIZ_ASSERT_MSG( m_RaceSlot != -1, "MountRace() wasnt called" );
										
										return m_RaceSlot;
									}

	const S32						GetNumEvents() const
									{
										return m_UserEvents.Size();
									}

	const bool						IsDirty() const 
									{
										return m_Dirty;
									}

	void							MountEvent( const S32 sSlot )
									{
										BIZ_ASSERT_MSG( sSlot < m_UserEvents.Size(), "Invalid group slot %i", sSlot );

										m_EventSlot = sSlot;
										m_BackupEvent = m_UserEvents[m_EventSlot];
									}

	void							MountPlaylist()
									{
										BIZ_ASSERT_MSG( m_EventSlot != -1, "MountGroup() wasnt called" );

										m_BackupPlaylist = m_UserEvents[m_EventSlot].m_Races;
									}

	void							MountRace( const S32 sSlot )
									{
										BIZ_ASSERT_MSG( m_EventSlot != -1, "MountGroup() wasnt called" );
										BIZ_ASSERT_MSG( sSlot < m_UserEvents[m_EventSlot].m_Races.Size(), "Invalid group slot %i", sSlot );

										m_RaceSlot = sSlot;
										m_BackupRace = m_UserEvents[m_EventSlot].m_Races[m_RaceSlot];
									}

	void							RestoreEvent()
									{
										// Check it was mounted first
										BIZ_ASSERT_MSG( m_EventSlot != -1, "MountGroup() wasnt called" );

										m_UserEvents[m_EventSlot] = m_BackupEvent;
										m_EventSlot = -1;
									}

	void							RestorePlaylist()
									{
										// Check it was mounted first
										BIZ_ASSERT_MSG( m_EventSlot != -1, "MountEvent() wasnt called" );

										m_UserEvents[m_EventSlot].m_Races = m_BackupPlaylist;
									}

	void							RestoreRace()
									{
										// Check it was mounted first
										BIZ_ASSERT_MSG( m_EventSlot != -1, "MountEvent() wasnt called" );
										BIZ_ASSERT_MSG( m_RaceSlot != -1, "MountRace() wasnt called" );

										m_UserEvents[m_EventSlot].m_Races[m_RaceSlot] = m_BackupRace;
										m_RaceSlot = -1;
									}

private:
	const bool						LoadImpl( Sys::BinaryDeserialiserAutoEndian& serial );
	const bool						SaveImpl( Sys::BinarySerialiserAutoEndian& serial );

private:
		static const U32					m_FileVersion = 1;
		static const U32					m_ServerVersion = 1;

		static C_UserEventManager			m_sInstance;

		bool								m_Dirty;		// Has anything been modified since last save.

		S32									m_EventSlot;	// Current event slot that is being edited.
		S32									m_RaceSlot;		// Current race slot that is being edited.

		C_MultiplayerEvent					m_BackupEvent;	// Copy of the event we are about to edit incase changes need reverting.															
		C_MultiplayerRace					m_BackupRace;	// Copy of the race we are about to edit incase changes need reverting.															
		
		Btl::Vector< C_MultiplayerRace >	m_BackupPlaylist;

		// User created groups data
		Btl::Vector< C_MultiplayerEvent, Btl::VectorParameters_Static< MAX_USER_EVENTS > >	m_UserEvents;		// Events created by the player

};

}; // namespace UI

#endif // USER_GROUP_MANAGER_H