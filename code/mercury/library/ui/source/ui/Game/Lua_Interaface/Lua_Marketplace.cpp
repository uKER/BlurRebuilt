#include "stdafx.h"

#include "ui/Game/Marketplace/Marketplace.h"

#include "ui/Game/UILua.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

namespace UI
{

// StartEnumeration
static S32 StartEnumeration( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "StartEnumeration( padNo ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	lua_pushboolean( pState, false );
	return 1;
}

// ContinueEnumeration
static S32 ContinueEnumeration( lua_State* pState )
{
	lua_pushboolean( pState, true );
	return 1;
}

// StartPurchase
static S32 StartPurchase( lua_State* pState )
{
	if ( ( lua_type( pState, 1 ) != LUA_TNUMBER ) || ( lua_type( pState, 2 ) != LUA_TNUMBER ) )
	{
		g_pUILua->StackDump( pState, "StartPurchase( padNo, index ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );
	U64 uIndex = (U64)lua_tonumber( pState, 2 );

	lua_pushboolean( pState, false );
	return 1;
}

// ContinuePurchase
static S32 ContinuePurchase( lua_State* pState )
{
	lua_pushboolean( pState, true );
	return 1;
}

// GetNumOffers
static S32 GetNumOffers( lua_State* pState )
{
	lua_pushnumber( pState, 0 );
	return 1;
}

// Downloading
static S32 Downloading( lua_State* pState )
{
	lua_pushboolean( pState, false );
	return 1;
}

// Cleanup
static S32 Cleanup( lua_State* pState )
{
	return 0;
}

// the library
static const luaL_reg marketplace_lib[] = 
{
	{ "StartEnumeration",			StartEnumeration },
	{ "ContinueEnumeration",		ContinueEnumeration },

	{ "StartPurchase",				StartPurchase },
	{ "ContinuePurchase",			ContinuePurchase },

	{ "GetNumOffers",				GetNumOffers },
	{ "Downloading",				Downloading },

	{ "Cleanup",					Cleanup },

	// end of list
	{ NULL, NULL }
};

LUALIB_API int luaopen_marketplace( lua_State* pState )
{
	luaL_openlib( pState, "Market", marketplace_lib, 0 );
	return 1;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", on )
#endif
