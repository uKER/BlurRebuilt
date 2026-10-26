#include "stdafx.h"

#include "ui/UIDefines.h"
#include "ui/GameSaveSystem.h"
#include "ui/Game/UILua.h"
#include "ui/Game/Gadgets/UIFriendsList.h"
#include "ui/Graphics/Localisation/loc.h"

#include "ui/Game/Profiles/ProfileInterface.h"
#include "ui/Game/Profiles/PcDriverName.h" // [Blur2PC] optional offline name
#include "UI/Game/Profiles/GamerPictureManager.h"
#include "ui/Game/SaveSystem/SaveSystem.h"
#include "ui/Util/WideString.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

#ifndef CFG_GOLD
Debug::Bool g_UiProfileShowStates( "UI.Profiles.ShowStates", false, Debug::Persist() );
#endif

// ***
// Sorcery dependencies
// PRO_BLAGGED_NAME => "Player %d"
// ***

namespace UI
{

// ShowStates
static S32 ShowStates( lua_State* pState )
{
#ifdef CFG_GOLD
	lua_pushboolean( pState, false );
#else
	lua_pushboolean( pState, g_UiProfileShowStates );
#endif
	return 1;
}

// Flush
static S32 Flush( lua_State* pState )
{
	SysProfile::C_Manager::Instance().SetPrimaryPadNo( -1 );
	C_ProfileInterface::Instance().Flush();
	return 0;
}

// AllowProfileChanges
static S32 AllowProfileChanges( lua_State* pState )
{
	if ( lua_type( pState, 1 ) == LUA_TNONE )
	{
		lua_pushboolean( pState, C_ProfileInterface::Instance().AllowProfileChanges() );
		return 1;
	}

	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "AllowProfileChanges( bool ) - incorrect arguments" );
		return 0;
	}

	C_ProfileInterface::Instance().AllowProfileChanges( lua_toboolean( pState, 1 ) != 0 );

	return 0;
}

// ActOnProfileChanges
static S32 ActOnProfileChanges( lua_State* pState )
{
	if ( lua_type( pState, 1 ) == LUA_TNONE )
	{
		lua_pushboolean( pState, C_ProfileInterface::Instance().ActOnProfileChanges() );
		return 1;
	}

	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "ActOnProfileChanges( bool ) - incorrect arguments" );
		return 0;
	}

	C_ProfileInterface::Instance().ActOnProfileChanges( lua_toboolean( pState, 1 ) != 0 );

	return 0;
}

// ClearProfile
static S32 ClearProfile( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "ClearPadProfile( padNo ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	C_ProfileInterface::Instance().GetPadProfile( sPadNo ) = SysProfile::FullProfile();

	return 0;
}

// ReadPadProfile
static S32 ReadPadProfile( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "ReadPadProfile( padNo ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	SysProfile::C_Manager::Instance().GetProfile( sPadNo, C_ProfileInterface::Instance().GetPadProfile( sPadNo ) );

	return 0;
}

// PadProfileActive
static S32 PadProfileActive( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "PadProfileActive( padNo ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	lua_pushboolean( pState, C_ProfileInterface::Instance().PadProfileActive( sPadNo ) );

	return 1;
}

// PadProfileSignedIn
static S32 PadProfileSignedIn( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "PadProfileSignedIn( padNo ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	lua_pushboolean( pState, C_ProfileInterface::Instance().PadProfileOnline( sPadNo ) || C_ProfileInterface::Instance().PadProfileOffline( sPadNo ) );

	return 1;
}

// PadProfileBlagged
static S32 PadProfileBlagged( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "PadProfileBlagged( padNo ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	lua_pushboolean( pState, C_ProfileInterface::Instance().PadProfileBlagged( sPadNo ) );

	return 1;
}

// PadProfileOnline
static S32 PadProfileOnline( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "PadProfileOnline( padNo ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	lua_pushboolean( pState, C_ProfileInterface::Instance().PadProfileOnline( sPadNo ) );

	return 1;
}

// PadProfileOffline
static S32 PadProfileOffline( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "PadProfileOffline( padNo ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	lua_pushboolean( pState, C_ProfileInterface::Instance().PadProfileOffline( sPadNo ) );

	return 1;
}

// SetPrimaryPad
static S32 SetPrimaryPad( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetPrimaryPadNo( padNo ) - incorrect arguments" );
		return 0;
	}

	SysProfile::C_Manager::Instance().SetPrimaryPadNo( (S32)lua_tonumber( pState, 1 ) );

	return 0;
}

// GetPrimaryPad
static S32 GetPrimaryPad( lua_State* pState )
{
	lua_pushnumber( pState, SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
	return 1;
}

// Setup
static S32 Setup( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "Setup( padNo ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	// do we need to blag this profile?
	bool bIsBlagged = ( ( lua_type( pState, 2 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 2 ) != 0 ) );

	// set as primary profile if not already set
	if ( SysProfile::C_Manager::Instance().GetPrimaryPadNo() == -1 )
		SysProfile::C_Manager::Instance().SetPrimaryPadNo( sPadNo );

	if ( !bIsBlagged )
	{
		// make sure we're up-to-date
		SysProfile::C_Manager::Instance().GetProfile( sPadNo, C_ProfileInterface::Instance().GetPadProfile( sPadNo ) );
	}
	
	if ( bIsBlagged )
	{
		SysProfile::FullProfile& padProfile = C_ProfileInterface::Instance().GetPadProfile( sPadNo );

		// the blagged profile's name
		C_WideString< PROFILE_MAX_NAME_LENGTH > buff;
		buff.Format( C_TextStringID( "PRO_BLAGGED_NAME" ).String(), sPadNo + 1 );

		// create a blagged profile
		SysProfile::C_Manager::Instance().CreateBlaggedProfile( sPadNo, buff, padProfile );
	}

	return 0;
}

// ForceProfileUpdate
static S32 ForceProfileUpdate( lua_State* pState )
{
	C_ProfileInterface::Instance().ForceProfileUpdate();
	return 0;
}

// StoreDeviceId
static S32 StoreDeviceId( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "StoreDeviceId( padNo ) - incorrect arguments" );
		return 0;
	}


	return 0;
}

// StartAsyncLoad
static S32 StartAsyncLoad( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "StartAsyncLoad( pad ) - incorrect arguments" );
		lua_pushboolean( pState, false );
		return 1;
	}

	const S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	// start the load
	lua_pushboolean( pState, C_SaveSystem::Instance().StartAsyncLoad( sPadNo ) );

	return 1;
}

// ContinueAsyncLoad
static S32 ContinueAsyncLoad( lua_State* pState )
{
	if ( C_SaveSystem::Instance().ContinueAsyncLoad() )
	{
		// we're finished
		lua_pushboolean( pState, true );
		lua_pushnumber( pState, C_SaveSystem::Instance().GetLoadErrorCode() );
	}
	else
	{
		lua_pushboolean( pState, false );
		lua_pushnil( pState );
	}

	return 2;
}

// StartAsyncSave
static S32 StartAsyncSave( lua_State* pState )
{  
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "StartAsyncSave( pad ) - incorrect arguments" );
		lua_pushboolean( pState, false );
		return 1;
	}

	const S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	// start the save
	lua_pushboolean( pState, C_SaveSystem::Instance().StartAsyncSave( sPadNo ) );

	return 1;
}

// ContinueAsyncSave
static S32 ContinueAsyncSave( lua_State* pState )
{
	if ( C_SaveSystem::Instance().ContinueAsyncSave() )
	{
		// we're finished
		lua_pushboolean( pState, true );
		lua_pushnumber( pState, C_SaveSystem::Instance().GetSaveErrorCode() );
	}
	else
	{
		lua_pushboolean( pState, false );
		lua_pushnil( pState );
	}

	return 2;
}

// GetCurrentSaveSize
static S32 GetCurrentSaveSize( lua_State* pState )
{
	lua_pushnumber( pState, C_SaveSystem::Instance().GetCurrentSaveSize() );
	return 1;
}

// StartAsyncDelete
static S32 StartAsyncDelete( lua_State* pState )
{  
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "StartAsyncDelete( pad ) - incorrect arguments" );
		lua_pushboolean( pState, false );
		return 1;
	}

	const S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	// start the delete
#ifdef PLATFORM_PS3
	// delete from a list instead?
	bool bDeleteFromList = ( lua_type( pState, 2 ) == LUA_TBOOLEAN ) ? ( lua_toboolean( pState, 2 ) != 0 ) : false;
	lua_pushboolean( pState, C_SaveSystem::Instance().StartAsyncDelete( sPadNo, bDeleteFromList ) );
#else
	lua_pushboolean( pState, C_SaveSystem::Instance().StartAsyncDelete( sPadNo ) );
#endif	

	return 1;
}

// ContinueAsyncDelete
static S32 ContinueAsyncDelete( lua_State* pState )
{
	if ( C_SaveSystem::Instance().ContinueAsyncDelete() )
	{
		// we're finished
		lua_pushboolean( pState, true );
		lua_pushnumber( pState, C_SaveSystem::Instance().GetDeleteErrorCode() );
	}
	else
	{
		lua_pushboolean( pState, false );
		lua_pushnil( pState );
	}

	return 2;
}

// GetEnumerationCount
static S32 GetEnumerationCount( lua_State* pState )
{
	lua_pushnumber( pState, C_SaveSystem::Instance().GetEnumerationCount() );
	return 1;
}

// IsValidEnumerationFileType
static S32 IsValidEnumerationFileType( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "IsValidEnumerationFileType( index ) - incorrect arguments" );
		lua_pushboolean( pState, false );
		return 1;
	}

	lua_pushboolean( pState, C_SaveSystem::Instance().IsValidEnumerationFileType( (S32)lua_tonumber( pState, 1 ) - 1 ) );
	return 1;
}

// SetGameInviteMode
static S32 SetGameInviteMode( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetGameInviteMode( padNo ) - incorrect arguments" );
		return 0;
	}

	C_ProfileInterface::Instance().AcceptGameInviteMode( (E_AcceptGameInviteMode)(S32)lua_tonumber( pState, 1 ) );

	return 0;
}

// SetFriendsActive
static S32 SetFriendsActive( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "SetFriendsActive( active ) - incorrect arguments" );
		return 0;
	}

	C_UIFriends::Instance().Active( lua_toboolean( pState, 1 ) != 0 );

	return 0;
}

// IsFriendsEnabled
static S32 IsFriendsEnabled( lua_State* pState )
{
	lua_pushboolean( pState, C_UIFriends::Instance().Active() );
	return 1;
}

// IsFriendsDisplayed
static S32 IsFriendsDisplayed( lua_State* pState )
{
	lua_pushboolean( pState, C_UIFriends::Instance().Displayed() );
	return 1;
}

// CloseFriends
static S32 CloseFriends( lua_State* pState )
{
	C_UIFriends::Instance().Cancel();
	return 0;
}

// AllowAllPadInput
static S32 AllowAllPadInput( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "AllowAllPadInput( bool ) - incorrect arguments" );
		return 0;
	}

	C_ProfileInterface::Instance().AllowAllPadInput( lua_toboolean( pState, 1 ) != 0 );

	return 0;
}

// LockToPad
static S32 LockToPad( lua_State* pState )
{
	if ( lua_type( pState, 1 ) == LUA_TNIL )
	{
		// reset
		C_ProfileInterface::Instance().SetLockToPad();
	}
	else if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "LockToPad( nil || padNo ) - incorrect arguments" );
		return 0;
	}

	// else lock to a specific pad
	C_ProfileInterface::Instance().SetLockToPad( (S32)lua_tonumber( pState, 1 ) );

	return 0;
}

// GuideActive
static S32 GuideActive( lua_State* pState )
{
	lua_pushboolean( pState, SysProfile::C_Manager::Instance().ShowingSystemOverlay() );
	return 1;
}

// pc only - need to inform profile manager what we wish our profile to be named.
// this just defaults the name until either a save game is loaded or we get the username/password
// from the user so we can authenticate against demonware
static S32 SetDefaultUserName(lua_State* pState)
{
#if defined ( PLATFORM_PC )
	// [Blur2PC] Keep the native profile/ID derivation; allow unique offline LAN names.
	char name[17];
	ReadPcDriverName("Driver", name);
	SysProfile::C_Manager::Instance().SetDemonwareAuthenticatedPlayerData(name);
#endif
	return 0;
}

// This is safe to call for all platforms but it should only run on the PS3 as its handled by the sign in/sign out message
// on the xbox.
static S32 GetPrimaryProfilePicture( lua_State* pState )
{
#ifdef PLATFORM_PS3
	// Check we have created the picture manager
	I_GamerPictureManager* pGamerPictureManager = C_ProfileInterface::Instance().GamerPictureManager();
	if ( !pGamerPictureManager )
		return 0;

	pGamerPictureManager->GetLocalPicture( SysProfile::C_Manager::Instance().GetPrimaryPadNo() );
#endif
	return 0;
}

static S32 GetRemoteGamerPictureMap( lua_State* pState )
{
	lua_newtable( pState );

	// Check we have created the picture manager
	I_GamerPictureManager* pGamerPictureManager = C_ProfileInterface::Instance().GamerPictureManager();

	// Create a map to index
	for ( S32 i = 0; i < pGamerPictureManager->TotalSize(); i++ )
	{
		const C_GamerPicture* pPicture = pGamerPictureManager->RemoteSlot( i );

		if ( pPicture && !pPicture->Free() )
		{
			lua_pushnumber( pState, i );
			lua_rawseti( pState, -2, pPicture->Ref() );
		}
	}

	return 1;
}

// ClearRemotePictureByRef
static S32 ClearRemotePictureByRef( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "ClearRemotePictureByRef( ref ) - incorrect arguments" );
		return 0;
	}

	if ( !C_ProfileInterface::Instance().GamerPictureManager() )
		return 0;

	C_ProfileInterface::Instance().GamerPictureManager()->ClearRemotePicture( (S32)lua_tonumber( pState, 1 ) );

	return 0;
}

// FlushRemotePictures
static S32 FlushRemotePictures( lua_State* pState )
{
	if ( !C_ProfileInterface::Instance().GamerPictureManager() )
		return 0;

	C_ProfileInterface::Instance().GamerPictureManager()->Flush();

	return 0;
}

// the library
static const luaL_reg profile_lib[] = 
{
	// debug
	{ "ShowStates",					ShowStates },

	// pad profiles
	{ "Flush",						Flush },
	{ "AllowProfileChanges",		AllowProfileChanges },
	{ "ActOnProfileChanges",		ActOnProfileChanges },
	{ "ClearProfile",				ClearProfile },
	{ "ReadPadProfile",				ReadPadProfile },
	{ "PadProfileActive",			PadProfileActive },
	{ "PadProfileSignedIn",			PadProfileSignedIn },
	{ "PadProfileBlagged",			PadProfileBlagged },
	{ "PadProfileOnline",			PadProfileOnline },
	{ "PadProfileOffline",			PadProfileOffline },
	{ "SetPrimaryPad",				SetPrimaryPad },
	{ "GetPrimaryPad",				GetPrimaryPad },
	{ "Setup",						Setup },
	{ "ForceProfileUpdate",			ForceProfileUpdate },

	// saving/loading/enumerating
	{ "StoreDeviceId",				StoreDeviceId },
	{ "StartAsyncLoad",				StartAsyncLoad },
	{ "ContinueAsyncLoad",			ContinueAsyncLoad },
	{ "StartAsyncSave",				StartAsyncSave },
	{ "ContinueAsyncSave",			ContinueAsyncSave },
	{ "GetCurrentSaveSize",			GetCurrentSaveSize },
	{ "StartAsyncDelete",			StartAsyncDelete },
	{ "ContinueAsyncDelete",		ContinueAsyncDelete },
	{ "GetEnumerationCount",		GetEnumerationCount },
	{ "IsValidEnumerationFileType",	IsValidEnumerationFileType },

	// invites
	{ "SetGameInviteMode",			SetGameInviteMode },

	// friends
	{ "SetFriendsActive",			SetFriendsActive },
	{ "IsFriendsEnabled",			IsFriendsEnabled },
	{ "IsFriendsDisplayed",			IsFriendsDisplayed },
	{ "CloseFriends",				CloseFriends },


	// misc
	{ "AllowAllPadInput",			AllowAllPadInput },
	{ "LockToPad",					LockToPad },
	{ "GuideActive",				GuideActive },
	{ "SetDefaultUserName",			SetDefaultUserName},

	// gamerpics
	{ "GetPrimaryProfilePicture",	GetPrimaryProfilePicture },
	{ "GetRemoteGamerPictureMap",	GetRemoteGamerPictureMap },
	{ "ClearRemotePictureByRef",	ClearRemotePictureByRef },
	{ "FlushRemotePictures",		FlushRemotePictures },

	// end of list
	{ NULL, NULL }
};

LUALIB_API int luaopen_profile( lua_State* pState )
{
	luaL_openlib( pState, "Profile", profile_lib, 0 );
	return 1;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on)
#endif
