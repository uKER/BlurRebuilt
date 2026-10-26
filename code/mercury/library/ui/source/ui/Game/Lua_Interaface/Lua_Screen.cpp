#include "stdafx.h"

#include "ui/Game/UIScreenControl.h"
#include "ui/Game/UILua.h"

#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/Gadgets/ButtonMessages.h"

#ifdef _VINCE_
#include "System\MGSTech\Vince\Vince.h"
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

static int SetNextScreen(lua_State *pState)
{
	if(lua_type(pState,1) != LUA_TSTRING)
	{
		g_pUILua->StackDump( pState, "SetNextScreen( number, [number] ) - incorrect arguments" );
		return 0;
	}

	// Optional context param
	S32 context = -1;
	if( lua_type( pState, 2 ) == LUA_TNUMBER )
	{
		context = (S32)lua_tonumber( pState, 2 );
	}

#ifdef _VINCE_
		VINCE_EVENT_UIScreenChange(gUIScreenControl.GetCurrentScreen(), str);
#endif
	const char *str = lua_tostring(pState,1);
	if( !str )
		return 0;
	
	C_ScreenControlManager::Instance().Get( context ).SetNextScreen( str );

	return 0;
}

static int EndScreen( lua_State* pState )
{
	// Optional context param
	S32 context = -1;
	if( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		context = (S32)lua_tonumber( pState, 1 );
	}

	C_ScreenControlManager::Instance().Get( context ).End( NULL );

	return 0;
}

static int ExitScreen( lua_State* pState )
{
	// Optional context param
	S32 context = -1;
	if( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		context = (S32)lua_tonumber( pState, 1 );
	}

	C_ScreenControlManager::Instance().Get( context ).Exit();

	return 0;
}

static int Suspend( lua_State* pState )
{
	// Mandatory context param
	if(lua_type(pState,1) != LUA_TNUMBER)
	{
		g_pUILua->StackDump( pState, "Suspend( number ) - incorrect arguments" );
		return 0;
	}


	S32 context = (S32)lua_tonumber(pState,1);
	C_ScreenControlManager::Instance().Get( context ).Suspend();

	return 0;
}

static int Resume( lua_State* pState )
{
	// Mandatory context param
	if(lua_type(pState,1) != LUA_TNUMBER)
	{
		g_pUILua->StackDump( pState, "Resume( number ) - incorrect arguments" );
		return 0;
	}

	S32 context = (S32)lua_tonumber(pState,1);
	C_ScreenControlManager::Instance().Get( context ).Resume();

	return 0;
}

static int GetCurrentScreen(lua_State *pState)
{
	// Optional param (context)
	S32 context = 0;
	if( lua_type(pState,1) == LUA_TNUMBER )
		context = (S32)lua_tonumber(pState,1);

	const Util::C_String currentScreen = C_ScreenControlManager::Instance().Get( context ).GetCurrentScreen();
	Util::C_String currentScreenLower = currentScreen.Lower();

	lua_pushstring(pState, currentScreenLower.Chars());

	return 1;   
}

static int SetScreenTimers(lua_State *pState)
{
	if( lua_type(pState,1) != LUA_TNUMBER || lua_type(pState,2)  != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetScreenTimers( number, number, [number] ) - incorrect arguments");
		return 0;
	}

	// Optional param (context)
	S32 context = -1;
	if( lua_type(pState,3) == LUA_TNUMBER )
		context = (S32)lua_tonumber(pState,3);

	C_ScreenControlManager::Instance().Get( context ).SetTimers( (F32)lua_tonumber(pState,1), (F32)lua_tonumber(pState,2) );

	return 0;
}

static int GetScreenTimers(lua_State *pState)
{
	lua_pushnumber( pState, gUIScreenControl.GetStartTime() );
	lua_pushnumber( pState, gUIScreenControl.GetEndTime() );

	return 2;
}

static int SetPopupTimers(lua_State *pState)
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,2))
	{
		g_pUILua->StackDump( pState, "SetPopupTimers(num,num) - incorrect arguments");
		return 0;
	}

	gUIPopupControl.SetTimers((float)lua_tonumber(pState,1),(float)lua_tonumber(pState,2));

	return 0;
}

static int SetInitialScreen(lua_State *pState)
{
	if(!lua_isstring(pState,1))
	{
		g_pUILua->StackDump( pState, "SetInitialScreen(num) - incorrect arguments");
		return 0;
	}

	if(g_pUILua->mDoingFullInit)
	{
		gUIScreenControl.SetNextScreen((char *)lua_tostring(pState,1));
	}

	return 0;
}

static int AddPopup(lua_State *pState)
{
	if(!lua_isstring(pState,1))
	{
		g_pUILua->StackDump( pState, "AddPopup(str) - incorrect arguments");
		return 0;
	}

	bool bIgnoreActive = ( ( lua_type( pState, 2 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 2 ) != 0 ) );

	if ( gUIPopupControl.Active() && !bIgnoreActive )
	{
		g_pUILua->StackDump( pState, "AddPopup() - popup already active!" );
		return 0;
	}

	C_ScreenControlManager::Instance().ResetContext( true, 1 );

	gUIPopupControl.SetNextScreen((char *)lua_tostring(pState,1));
	gUIPopupControl.Init();

	return 0;
}

static int CancelPopup(lua_State *pState)
{
	if(lua_type(pState,1) == LUA_TSTRING )
	{
		const char *str = lua_tostring(pState,1);

#ifdef _VINCE_
		VINCE_EVENT_UIScreenChange(gUIScreenControl.GetCurrentScreen(), str);
#endif
		S32 context = 0;
		if( lua_type(pState,2) == LUA_TNUMBER )
			context = (S32)lua_tonumber( pState, 2 );

		gUIPopupControl.End( str, context );
	}
	else
	{
		gUIPopupControl.End( NULL );
	}

	C_ScreenControlManager::Instance().ResetContext( true );
	
	return 0;
}

static int IsPopupActive(lua_State *pState)
{
	lua_pushboolean(pState,gUIPopupControl.Active());

	return 1;
}

static int IsContextActive(lua_State *pState)
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "IsContextActive( number ) - incorrect arguments");
		return 0;
	}

	lua_pushboolean( pState, C_ScreenControlManager::Instance().Get( (S32)lua_tonumber( pState, 1 ) ).Active() );

	return 1;
}

static int AddMessage( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "AddMessage( ID, [opt], [opt], [opt], [opt] ) - incorrect arguments" );
		return 0;
	}

	// this just uses the C_UiMessageBase class
	F64 arg0, arg1, arg2, arg3;
	arg0 = ( lua_type( pState, 2 ) == LUA_TNUMBER ) ? lua_tonumber( pState, 2 ) : 0.0;
	arg1 = ( lua_type( pState, 3 ) == LUA_TNUMBER ) ? lua_tonumber( pState, 3 ) : 0.0;
	arg2 = ( lua_type( pState, 4 ) == LUA_TNUMBER ) ? lua_tonumber( pState, 4 ) : 0.0;
	arg3 = ( lua_type( pState, 5 ) == LUA_TNUMBER ) ? lua_tonumber( pState, 5 ) : 0.0;

	Messaging::TMsgType _type = (Messaging::TMsgType)(S32)lua_tonumber( pState, 1 );

	// send it
	C_UiMessageData msg( _type, sizeof(C_UiMessageData) );
	msg.AddParam( arg0 );
	msg.AddParam( arg1 );
	msg.AddParam( arg2 );
	msg.AddParam( arg3 );
	UI::gButtons.BroadcastMessage( &msg, 0xffffffff );

	return 0;
}

static int AddMessageNow( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "AddMessage( ID, [opt], [opt], [opt], [opt] ) - incorrect arguments" );
		return 0;
	}

	// this just uses the C_UiMessageBase class
	F64 arg0, arg1, arg2, arg3;
	arg0 = ( lua_type( pState, 2 ) == LUA_TNUMBER ) ? lua_tonumber( pState, 2 ) : 0.0;
	arg1 = ( lua_type( pState, 3 ) == LUA_TNUMBER ) ? lua_tonumber( pState, 3 ) : 0.0;
	arg2 = ( lua_type( pState, 4 ) == LUA_TNUMBER ) ? lua_tonumber( pState, 4 ) : 0.0;
	arg3 = ( lua_type( pState, 5 ) == LUA_TNUMBER ) ? lua_tonumber( pState, 5 ) : 0.0;

	Messaging::TMsgType _type = (Messaging::TMsgType)(S32)lua_tonumber( pState, 1 );

	// send it
	C_UiMessageData msg( _type, sizeof(C_UiMessageData) );
	msg.AddParam( arg0 );
	msg.AddParam( arg1 );
	msg.AddParam( arg2 );
	msg.AddParam( arg3 );
	UI::gButtons.BroadcastMessageNow( 0xffffffff, &msg );

	return 0;
}

static int SetInitFunction(lua_State* pState)
{
	if(lua_type(pState,1) != LUA_TSTRING)
	{
		g_pUILua->StackDump( pState, "SetInitFunction( name ) - incorrect arguments" );
		return 0;
	}

	gUIScreenControl.SetInitFunctionName(lua_tostring(pState,1));

	return 0;
}

static int SetEndFunction(lua_State* pState)
{
	if(lua_type(pState,1) != LUA_TSTRING)
	{
		g_pUILua->StackDump( pState, "SetEndFunction( string ) - incorrect arguments" );
		return 0;
	}

	gUIScreenControl.SetExitFunctionName(lua_tostring(pState,1));

	return 0;
}

static int AddXtRecord( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TSTRING )
	{
		g_pUILua->StackDump( pState, "AddXtRecord( number, [bool] ) - incorrect arguments" );
		return 0;
	}
	bool bCreateXtElements = true;
	if( lua_type( pState, 2 ) == LUA_TBOOLEAN )
		bCreateXtElements = lua_toboolean( pState, 2 ) != 0;

	gUIScreenControl.AddXtRecord( lua_tostring( pState, 1 ), false, bCreateXtElements );

	return 0;
}

// Context
static int Context( lua_State* pState )
{
	if ( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		// set
		C_ScreenControlManager::Instance().Context( (S32)lua_tonumber( pState, 1 ) );
		return 0;
	}

	// get
	lua_pushnumber( pState, C_ScreenControlManager::Instance().Context() );
	return 1;
}

static int EnableControllerInput( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "EnableControllerInput( bool,[number] ) - incorrect arguments" );
		return 0;
	}

	S32 context = -1;
	if( lua_type( pState, 2 ) == LUA_TNUMBER )
	{
		context = (S32)lua_tonumber( pState, 2 );
	}

	const bool enable = lua_toboolean( pState, 1 ) != 0;

	_UI_MESSAGE1_MASK( E_UiMsg_EnableControllerInput, enable, C_ScreenControlManager::Instance().Get( context ).GetMessageMask() );

	return 0;
}

static int GetScreenActive( lua_State* pState )
{
	S32 context = -1;
	if( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		context = (S32)lua_tonumber( pState, 1 );
	}

	C_ScreenControl& sc = C_ScreenControlManager::Instance().Get( context );

	lua_pushboolean( pState, sc.ScreenTransitionActive() ? 0 : 1 );
	return 1;
}

// BlockInputToContext
static S32 BlockInputToContext( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "BlockMessagesToContext( bool, [number] ) - incorrect arguments" );
		return 0;
	}

	bool bEnable = ( lua_toboolean( pState, 1 ) != 0 );
	S32 sContext = C_ScreenControlManager::Instance().Context();

	if( lua_type( pState, 2 ) == LUA_TNUMBER )
		sContext = (S32)lua_tonumber( pState, 2 );

	C_ScreenControlManager::Instance().Get( sContext ).BlockInputToMe( bEnable );

	return 0;
}

static const luaL_reg screen_lib[] = 
{
	{"SetNextScreen",			SetNextScreen},
	{"EndScreen",				EndScreen},
	{"ExitScreen",				ExitScreen},
	{"GetScreenActive",			GetScreenActive},
	{"GetCurrentScreen",		GetCurrentScreen},
	{"SetScreenTimers",			SetScreenTimers},
	{"GetScreenTimers",			GetScreenTimers},
	{"SetPopupTimers",			SetPopupTimers},
	{"SetInitialScreen",		SetInitialScreen},
	{"AddMessage",				AddMessage},
	{"AddMessageNow",			AddMessageNow},
	{"SetInitFunction",			SetInitFunction},
	{"SetEndFunction",			SetEndFunction},
	{"Suspend",					Suspend},
	{"Resume",					Resume},	
	{"AddPopup",				AddPopup},
	{"CancelPopup",				CancelPopup},
	{"IsPopupActive",			IsPopupActive},
	{"IsContextActive",			IsContextActive},

	{"AddXtRecord",				AddXtRecord},

	{"Context",					Context},
	{"EnableControllerInput",	EnableControllerInput},

	{"BlockInputToContext",		BlockInputToContext},

	{NULL,			NULL}		// end of list
};

LUALIB_API int luaopen_screen (lua_State *pState)
{
	luaL_openlib(pState,"UIScreen",screen_lib,0);

	return 1;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
