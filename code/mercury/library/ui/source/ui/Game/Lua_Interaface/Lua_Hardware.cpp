#include "stdafx.h"

#include <ui/UiInterface.h>
#include <ui/Graphics/Font/Font.h>
#include <ui/Graphics/Localisation/Loc.h>
#include <ui/Game/UILua.h>
#include <ui/Game/UIDataFinder.h>
#include <ui/GameSaveSystem.h>
#include <ui/DashboardInterface.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

static int WideScreen(lua_State *pState)
{
	lua_pushboolean(pState,UiInterface()->IsWideScreen());

	return 1;
}

// StartSigninUI (XBox HUD)
static int StartSigninUI(lua_State *pState)
{
	// Optional parameter 1 : Overide the maximum number of players to sign in
	S32 numPlayers = 4;
	if(lua_isnumber(pState,1))
	{
		numPlayers = (S32)lua_tonumber(pState,1);

		if(numPlayers<1 || numPlayers >4)
		{
			LOGERROR( g_Log, DBSTR "Wrong number of players");
			lua_pushboolean(pState,false);
			return 1;
		}
	}
	// Optional param 2 : overide the SignInType
	C_DashboardInterface::E_SignInType type = C_DashboardInterface::E_SignInType_None;
	if(lua_isnumber(pState,2))
	{
		type = (C_DashboardInterface::E_SignInType)(S32)lua_tonumber(pState,2);
	}
	const S32 success = gDashboardInterface.StartXenonSigninUI( numPlayers, type ) ? 1 : 0;

	lua_pushboolean(pState,success);

	return 1;
}

static int StartPlayersUI(lua_State *pState)
{
	if(!lua_isnumber(pState,1))
	{
		g_pUILua->StackDump(pState,"startPlayersUI(player_index) - incorrect arguments\n");
		return 0;
	}

	gDashboardInterface.SetIndex((S32)lua_tonumber(pState,1));

	const S32 success = gDashboardInterface.StartXenonPlayersUI();
	
	lua_pushboolean(pState,success);

	return 1;
}

// StartKeyboard(default_string, title, help_text, string length)
// default_string is a lua string
static int StartKeyboard(lua_State *pState)
{
	if(!lua_isnumber(pState,1) || (!lua_isstring(pState,2) && !lua_isnumber(pState,2)) || !lua_isnumber(pState,3) || !lua_isnumber(pState,4) || !lua_isnumber(pState,5))
	{
		g_pUILua->StackDump(pState,"Profile.StartKeyboard(index,df_string|ids,default,title,desc,num) - incorrect arguments\n");
		return 0;
	}


	const S32 index=(S32)lua_tonumber(pState,1);
	gDashboardInterface.SetIndex(index);

	// Get the string
	static C_WideString512 defWstr;

	if ( lua_type( pState, 2 ) == LUA_TSTRING )
	{
		// get the string from the gamedatafinder
		defWstr = C_UIDataFinder::Instance().
			GetVariableText( lua_tostring( pState, 2 ) );
	}
	else
	{
		// get the string from the IDS
		defWstr = C_TextStringID( lua_tonumber( pState, 2 ) ).String();
	}

	const int char_count	=(int)lua_tonumber(pState,5); // Number of chars
	
	// PR - flags for restricting keys?
	if ( lua_type( pState, 6 ) == LUA_TNUMBER )
		gDashboardInterface.StartXenonKeyboardUI(defWstr,C_TextStringID(lua_tonumber(pState,3)).String(),C_TextStringID(lua_tonumber(pState,4)).String(),char_count, (U64)lua_tonumber( pState, 6 ) );
	else
		gDashboardInterface.StartXenonKeyboardUI(defWstr,C_TextStringID(lua_tonumber(pState,3)).String(),C_TextStringID(lua_tonumber(pState,4)).String(),char_count);

	return 0;
}

static int ProfileEnumerate(lua_State *pState)
{
	if(!lua_isnumber(pState,1))
	{
		UI::g_pUILua->StackDump(pState,"ProfileLoad(num) - incorrect arguments\n");
		return 0;
	}

	int user_index=(int)lua_tonumber(pState,1);

	gGameSaveSystem.SetUserIndex(user_index);
	gGameSaveSystem.Enumerate();
	return 0;
}

static int FindXbox360SaveLocation( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "FindXbox360SaveLocation( pad, dataSize?, deviceFlag? ) - incorrect arguments\n");
		return 0;
	}

	S32 sPad = (S32)lua_tonumber( pState, 1 );

	gGameSaveSystem.SetUserIndex( sPad );

	// check for the extra flags
	S32 sDataSize = -1;
	if ( lua_type( pState, 2 ) == LUA_TNUMBER )
		sDataSize = (S32)lua_tonumber( pState, 2 );

	S32 sDeviceSelectorFlag = -1;
	if ( lua_type( pState, 3 ) == LUA_TNUMBER )
		sDeviceSelectorFlag = (S32)lua_tonumber( pState, 3 );
		
	gGameSaveSystem.StartXenonLocationUI( sDataSize, sDeviceSelectorFlag );

	return 0;
}

static int SpaceLeftOnDevice( lua_State* pState )
{
	lua_pushnumber( pState, 0 );
	lua_pushboolean( pState, false );

	return 2;
}

// ShowFriends360
static S32 ShowFriends360( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "ShowFriends360( pad ) - incorrect arguments" );
		return 0;
	}

	S32 sPad = (S32)lua_tonumber( pState, 1 );


	return 0;
}

// ShowParty360
static S32 ShowParty360( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "ShowParty360( pad ) - incorrect arguments" );
		return 0;
	}

	S32 sPad = (S32)lua_tonumber( pState, 1 );


	return 0;
}

static S32 ReturnToDemoLauncher( lua_State* pState ) 
{
	return 0;
}

static const luaL_reg hardware_lib[] = 
{
	{"WideScreen",						WideScreen },
	{"StartKeyboard",					StartKeyboard },
	{"StartSigninUI",					StartSigninUI },
	{"StartPlayersUI",					StartPlayersUI },
	{"ProfileEnumerate",				ProfileEnumerate },
	{"FindXbox360SaveLocation",			FindXbox360SaveLocation },
	{"SpaceLeftOnDevice",				SpaceLeftOnDevice },
	{"ShowFriends360",					ShowFriends360 },
	{"ShowParty360",					ShowParty360 },
	{"ReturnToDemoLauncher",			ReturnToDemoLauncher },

	{NULL,								NULL } // end of list
};

LUALIB_API int luaopen_hardware(lua_State *pState)
{
	luaL_openlib(pState,"UIHardware",hardware_lib,0);

	return 1;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on)
#endif
