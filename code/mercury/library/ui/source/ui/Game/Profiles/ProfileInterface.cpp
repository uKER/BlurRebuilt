#include "stdafx.h"

#include "ui/Game/Profiles/ProfileInterface.h"
#include <ui/Game/Gadgets/ButtonMessages.h>
#include "ui/Game/Profiles/ProfileInterfaceMessages.h"

#if defined PLATFORM_PC
#include "ui/Game/Profiles/GamerPictureManager_Pc.h"
#else
#include "ui/Game/Profiles/GamerPictureManager_Ps3.h"
#endif

#include <ui/Graphics/SimpleTimer.h>

#include <Gamelogic\SocialNetworking\FriendsManager.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

// global instance
C_ProfileInterface C_ProfileInterface::m_sInstance;

// ------------------
// C_ProfileInterface
// ------------------
C_ProfileInterface::C_ProfileInterface()
	: m_bForceUpdateProfiles( false ),
	m_bAllowProfileChanges( true ),
	m_bActOnProfileChanges( true ),
	m_eAcceptGameInviteMode( E_AcceptGameInviteMode_Delay ),
	m_bAllowAllPadInput( true ),
	m_sLockToPad( -1 ),
	m_pGamerPictureManager( NULL ),
	m_bHasLostConnection( false ),
	m_bHasLostConnectionXLSP( false ),
	m_bHasLostConnectionLobby( false ),
	m_bNetworkCableRemoved( false )
{
#if defined PLATFORM_PC
	m_pGamerPictureManager = new C_GamerPictureManager_Pc();
#else
	m_pGamerPictureManager = new C_GamerPictureManager_Ps3();
#endif
}

C_ProfileInterface::~C_ProfileInterface()
{
	SAFE_DELETE_PTR( m_pGamerPictureManager );
}

void C_ProfileInterface::Initialise()
{
#ifndef CFG_GOLD
	// register message debug info
	U32 uNumMessagesRegistered = 0;

	_UI_PROFILE_MESSAGE_DEBUG( GameInvitePending );
	_UI_PROFILE_MESSAGE_DEBUG( SignInChanged );
	_UI_PROFILE_MESSAGE_DEBUG( DuplicateSignIn );
	_UI_PROFILE_MESSAGE_DEBUG( ProfilesChanged );
	_UI_PROFILE_MESSAGE_DEBUG( ProfileSettingsChanged );
	_UI_PROFILE_MESSAGE_DEBUG( StorageDeviceChanged );
	_UI_PROFILE_MESSAGE_DEBUG( NewContentInstalled );
	_UI_PROFILE_MESSAGE_DEBUG( InputDeviceChanged );
	_UI_PROFILE_MESSAGE_DEBUG( MuteInfoChanged );
	_UI_PROFILE_MESSAGE_DEBUG( ServiceConnectionLost );
	_UI_PROFILE_MESSAGE_DEBUG( ServiceConnectionLostXLSP );
	_UI_PROFILE_MESSAGE_DEBUG( ServiceConnectionEstablished );
	_UI_PROFILE_MESSAGE_DEBUG( LobbyConnectionLost );
	_UI_PROFILE_MESSAGE_DEBUG( NetworkCableRemoved );
	_UI_PROFILE_MESSAGE_DEBUG( FriendRequestPending );
	_UI_PROFILE_MESSAGE_DEBUG( FriendSignedIn );
	_UI_PROFILE_MESSAGE_DEBUG( DeviceConfigChanged );
	_UI_PROFILE_MESSAGE_DEBUG( SignedIn );
	_UI_PROFILE_MESSAGE_DEBUG( SignedOut );

	// if this asserts here then someone has changed E_ProfileNotifications - you need to add some _UI_PROFILE_MESSAGE_DEBUG calls above
	assert( uNumMessagesRegistered == ( E_ProfMsg_Count - ( E_ProfMsg_Start + 1 ) ) );
#endif
}

void C_ProfileInterface::Flush()
{
	SysProfile::FullProfile blank;

	for ( S32 i = 0; i < m_PadProfiles.Size(); i++ )
		m_PadProfiles[ i ] = blank;

	m_bHasLostConnection = false;
	m_bHasLostConnectionXLSP = false;
	m_bHasLostConnectionLobby = false;

	m_eAcceptGameInviteMode = E_AcceptGameInviteMode_Delay;

	m_bAllowAllPadInput = true;
	m_sLockToPad = -1;
}

U32 C_ProfileInterface::NumActive()
{
	U32 count = 0;

	for ( S32 i = 0; i < m_PadProfiles.Size(); i++ )
		count += ( m_PadProfiles[ i ].Active() == true ) ? 1 : 0;

	return count;
}

void C_ProfileInterface::ActOnProfileChanges( const bool bActOnProfileChanges )
{
	m_bActOnProfileChanges = bActOnProfileChanges;

	// clear the temps
	if ( !m_bActOnProfileChanges )
	{
		m_bHasLostConnection = false;
		m_bHasLostConnectionXLSP = false;
		m_bHasLostConnectionLobby = false;
		m_bNetworkCableRemoved = false;
		m_SignedInPads.Clear();
		m_SignedOutPads.Clear();
	}
}

bool C_ProfileInterface::AllowPadInput( const S32 sPadNo )
{
	if ( m_bAllowAllPadInput )
		return true;

	if ( !PadProfileActive( sPadNo ) && !m_bAllowAllPadInput )
		return false;

	if ( ( m_sLockToPad != -1 ) && ( sPadNo != m_sLockToPad ) )
		return false;

	return true;
}

SysProfile::FullProfile& C_ProfileInterface::GetPadProfile( const S32 sPadNo )
{
	static SysProfile::FullProfile blank;

	if ( ( sPadNo < 0 ) || ( sPadNo >= MAX_USERS ) )
		return blank;

	return m_PadProfiles[ sPadNo ];
}

S32	C_ProfileInterface::GetPadIndexFromID( const SysProfile::UserIdentifier sUserID ) const
{
	for ( S32 i = 0; i < m_PadProfiles.Size(); i++ )
	{
		if ( ((SysProfile::UserIdentifier)m_PadProfiles[i]) == sUserID )
			return i;
	}

	return -1;
}

bool C_ProfileInterface::PadProfileActive( const S32 sPadNo )
{
	if ( ( sPadNo < 0 ) || ( sPadNo >= MAX_USERS ) )
		return false;

	return ( m_PadProfiles[ sPadNo ].m_State != SysProfile::eProfileStateNotActive );
}

bool C_ProfileInterface::PadProfileOnline( const S32 sPadNo )
{
	if ( ( sPadNo < 0 ) || ( sPadNo >= MAX_USERS ) )
		return false;

	return ( m_PadProfiles[ sPadNo ].m_State == SysProfile::eProfileStateOnline );
}

bool C_ProfileInterface::PadProfileOffline( const S32 sPadNo )
{
	if ( ( sPadNo < 0 ) || ( sPadNo >= MAX_USERS ) )
		return false;

	return ( m_PadProfiles[ sPadNo ].m_State == SysProfile::eProfileStateOffline );
}

bool C_ProfileInterface::PadProfileBlagged( const S32 sPadNo )
{
	if ( ( sPadNo < 0 ) || ( sPadNo >= MAX_USERS ) )
		return false;

	return ( m_PadProfiles[ sPadNo ].m_State == SysProfile::eProfileStateBlagged );
}

void C_ProfileInterface::Update()
{
	// poll profile manager
	SysProfile::GameInvite invite;
	SysProfile::Notifications notifications;

	SysProfile::C_Manager::Instance().Poll( invite, notifications, theFETimer.FrameTime() );

	bool bUpdateProfiles = false, bUpdatePicture = false;

	if ( invite.m_Active )
	{
		C_ProfMsg_GameInvitePending msg( invite );
		_UI_MESSAGE_FULL( msg );
	}
	if ( notifications.m_SignInChanged )
	{
		C_ProfMsg_SignInChanged msg;
		_UI_MESSAGE_FULL( msg );
		bUpdateProfiles = true;
	}
	if ( notifications.m_DuplicateSignIn )
	{
		C_ProfMsg_DuplicateSignIn msg;
		_UI_MESSAGE_FULL( msg );
		bUpdateProfiles = true;
	}
	if ( notifications.m_ProfilesChanged )
	{
		C_ProfMsg_ProfilesChanged msg;
		_UI_MESSAGE_FULL( msg );
		bUpdateProfiles = true;
	}
	if ( notifications.m_ProfileSettingsChanged )
	{
		C_ProfMsg_ProfileSettingsChanged msg;
		_UI_MESSAGE_FULL( msg );
		bUpdateProfiles = true;
		bUpdatePicture = true;
	}
	if ( notifications.m_StorageDeviceChanged )
	{
		C_ProfMsg_StorageDeviceChanged msg;
		_UI_MESSAGE_FULL( msg );
	}
	if ( notifications.m_NewContentInstalled )
	{
		C_ProfMsg_NewContentInstalled msg;
		_UI_MESSAGE_FULL( msg );
	}
	if ( notifications.m_InputDeviceChanged )
	{
		C_ProfMsg_InputDeviceChanged msg;
		_UI_MESSAGE_FULL( msg );
	}
	if ( notifications.m_MuteInfoChanged )
	{
		C_ProfMsg_MuteInfoChanged msg;
		_UI_MESSAGE_FULL( msg );
	}
	if ( notifications.m_ServiceConnectionLost )
	{
		if ( m_bActOnProfileChanges )
		{
			C_ProfMsg_ServiceConnectionLost msg;
			_UI_MESSAGE_FULL( msg );
			bUpdateProfiles = true;
		}
		else
			m_bHasLostConnection = true;
	}
	if ( m_bForceUpdateProfiles && m_bHasLostConnection )
	{
		C_ProfMsg_ServiceConnectionLost msg;
		_UI_MESSAGE_FULL( msg );
		bUpdateProfiles = true;
		m_bHasLostConnection = false;
	}

	if ( notifications.m_ServiceConnectionEstablished )
	{
		C_ProfMsg_ServiceConnectionEstablished msg;
		_UI_MESSAGE_FULL( msg );
		bUpdateProfiles = true;
	}
	if ( notifications.m_DeviceConfigChanged )
	{
		C_ProfMsg_DeviceConfigChanged msg( notifications.m_DeviceConfigChanged );
		_UI_MESSAGE_FULL( msg );
	}
	// Network cable disconnect messages
	if ( notifications.m_NetworkCableRemoved )
	{
		if ( m_bActOnProfileChanges )
		{
			C_ProfMsg_NetworkCableRemoved msg;
			_UI_MESSAGE_FULL( msg );
		}
		else
			m_bNetworkCableRemoved = true;
	}

#ifdef PLATFORM_PS3
	//ps3 only, the friends manager handles the xbox
	if ( notifications.m_FriendsUpdated )
	{
		Game::C_FriendsManager::Instance().StartEnumeration();
	}
#endif

	if ( m_bForceUpdateProfiles && m_bNetworkCableRemoved )
	{
		C_ProfMsg_NetworkCableRemoved msg;
		_UI_MESSAGE_FULL( msg );
		bUpdateProfiles = true;
		m_bNetworkCableRemoved = false;
	}
	// check for any stored messages
	if ( m_bForceUpdateProfiles )
	{
		for ( S32 i = 0; i < m_SignedInPads.Size(); i++ )
		{
			UI::C_ProfMsg_SignedIn msg( m_SignedInPads[ i ] );
			_UI_MESSAGE_FULL( msg );

			bUpdateProfiles = true;
		}

		m_SignedInPads.Clear();

		for ( S32 i = 0; i < m_SignedOutPads.Size(); i++ )
		{
			UI::C_ProfMsg_SignedOut msg( m_SignedOutPads[ i ] );
			_UI_MESSAGE_FULL( msg );

			bUpdateProfiles = true;
		}
		
		m_SignedOutPads.Clear();
	}

	// check for signing in/out
	if ( bUpdateProfiles 
#ifdef PLATFORM_PS3
		&& ( SysProfile::C_Manager::Instance().GetPrimaryPadNo() != -1 ) 
#endif
	)
	{
		for ( S32 i = 0; i < MAX_USERS; i++ )
		{
			// what we currently have
			SysProfile::FullProfile& currentProfile = m_PadProfiles[ i ];

			// re-read
			SysProfile::FullProfile newProfile;
			SysProfile::C_Manager::Instance().GetProfile( i, newProfile );

			bool bGetPicture = false, bClearPicture = false;

			// signed in?
			if ( ( currentProfile.m_State == SysProfile::eProfileStateNotActive || currentProfile.m_State == SysProfile::eProfileStateBlagged )
				&& ( newProfile.m_State == SysProfile::eProfileStateOnline || newProfile.m_State == SysProfile::eProfileStateOffline ) )
			{
				if ( m_bActOnProfileChanges )
				{
					UI::C_ProfMsg_SignedIn msg( i );
					_UI_MESSAGE_FULL( msg );
				}
				else
				{
					m_SignedInPads.PushBack( i );
					_UI_MESSAGE( E_UiMsg_ProfileChanged );
				}

				bGetPicture = true;
			}

			// signed out?
			if ( ( currentProfile.m_State == SysProfile::eProfileStateOnline || currentProfile.m_State == SysProfile::eProfileStateOffline )
					&& ( newProfile.m_State == SysProfile::eProfileStateNotActive ) )
			{
				if ( m_bActOnProfileChanges )
				{
					UI::C_ProfMsg_SignedOut msg( i );
					_UI_MESSAGE_FULL( msg );
				}
				else
				{
					m_SignedOutPads.PushBack( i );
					_UI_MESSAGE( E_UiMsg_ProfileChanged );
				}

				bClearPicture = true;
			}

			if ( ( newProfile.m_State == SysProfile::eProfileStateNotActive ) && ( currentProfile.m_State == SysProfile::eProfileStateBlagged ) )
				; // do nothing so we don't wipe the blagged name
			else
			if ( m_bAllowProfileChanges || m_bActOnProfileChanges )
				currentProfile = newProfile;	// update

			// Update gamerpicture logic.
			if ( bGetPicture || bUpdatePicture )
				GetLocalGamerPicture( i );
		
			if ( bClearPicture )
				ClearLocalGamerPicture( i );
		}
	}

#ifdef PLATFORM_PS3
	if ( notifications.m_ServiceConnectionEstablished )
	{
		if ( SysProfile::C_Manager::Instance().GetPrimaryPadNo() != -1 )
			GetLocalGamerPicture( SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
	}
	if ( notifications.m_ServiceConnectionLost )
	{
		if ( SysProfile::C_Manager::Instance().GetPrimaryPadNo() != -1 )
			ClearLocalGamerPicture( SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
	}
#endif // PLATFORM_PS3


	// check XLSP last
	if ( m_bHasLostConnectionXLSP && ( m_bActOnProfileChanges || m_bForceUpdateProfiles ) )
	{
		C_ProfMsg_ServiceConnectionLostXLSP msg;
		_UI_MESSAGE_FULL( msg );

		m_bHasLostConnectionXLSP = false;
	}

	// Check lobby connection
	if ( m_bHasLostConnectionLobby && ( m_bActOnProfileChanges || m_bForceUpdateProfiles )  )
	{
		C_ProfMsg_LobbyConnectionLost msg;
		_UI_MESSAGE_FULL( msg );

		m_bHasLostConnectionLobby = false;
	}

	// this is only valid for one frame
	m_bForceUpdateProfiles = false;

	if ( m_pGamerPictureManager )
	{
		m_pGamerPictureManager->Update();
	}
}

void C_ProfileInterface::GetLocalGamerPicture( const S32 sPadNo )
{
	BIZ_ASSERT( sPadNo >= 0 && sPadNo < (S32)Max() );

	if ( !m_pGamerPictureManager )
	{
		LOGWARN( g_Log, "GamerPicure Manager was never created. This is bad!!" );
		return;
	}

	if ( !PadProfileBlagged( sPadNo ) )
		m_pGamerPictureManager->GetLocalPicture( sPadNo );
}

void C_ProfileInterface::ClearLocalGamerPicture( const S32 sPadNo )
{
	BIZ_ASSERT( sPadNo >= 0 && sPadNo < (S32)Max() );

	if ( !m_pGamerPictureManager )
	{
		LOGWARN( g_Log, "GamerPicure Manager was never created. This is bad!!" );
		return;
	}

	m_pGamerPictureManager->ClearLocalPicture( sPadNo );
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
