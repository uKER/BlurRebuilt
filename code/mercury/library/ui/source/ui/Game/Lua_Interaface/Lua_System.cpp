#include "stdafx.h"

#ifdef PLATFORM_PS3
#include <sysutil/sysutil_msgdialog.h>
#include <netex/libnetctl.h>
#endif

#include "ui/UiInterface.h"

#include "ui/Game/UILua.h"
#include "ui/Game/UIDataFinder.h"
#include "ui/Game/StringStore.h"
#include <ui/Game/UILocaliser.h>
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Game/Gadgets/UICamera.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Movie.h"
#include "ui/Graphics/UITextureManager.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

extern Debug::Bool g_UiDebugDoRender;

namespace UI
{

#ifndef CFG_GOLD
	Debug::Bool g_DebugUiDisableMovie( "UI.DisableMovies", false, Debug::Persist() );
	// Debug::Bool g_DebugUiDisableMovie( "UI.DisableMovies", true );
#ifdef GAME_RACERNOTAMAX
Debug::Bool gDisableIntroAttractMovie("UI.DisableIntroAndAttractMovie", true, Debug::Persist());
#endif
#endif

// Lua interface functions
static int PlaySound(lua_State *pState)
{
	if(!lua_isnumber(pState,1))
	{
		g_pUILua->StackDump(pState,"PlaySound(sound_num) - incorrect arguments");
		return 0;
	}
	const U32 sound_index = (U32)lua_tonumber(pState,1);
	 
	UiInterface()->PlaySound( sound_index );

	return 0;
}

static int LoadLuaScript(lua_State *pState)
{
	if(!lua_isstring(pState,1))
	{
		g_pUILua->StackDump( pState, "LoadLuaScript(str) - incorrect arguments" );
		return 0;
	}

	g_pUILua->LoadCallFile((char *)lua_tostring(pState,1));

	return 0;
}

static int MakeNumberString(lua_State *pState)
{
	if(!lua_isnumber(pState,1))
	{
		g_pUILua->StackDump(pState,"LUA_MakeNumberStr(number) - incorrect arguments");

		lua_pushstring(pState, "INVALID" );
		return 1;
	}

	char buff[ 32 ];
	sprintf_s( buff, 32, "%S", C_UILocaliser::Instance().ConvertNumberToCommas( (S32)lua_tonumber( pState, 1 ) ) );
	lua_pushstring( pState, buff );

	return 1;
}

static int GetStringLength( lua_State* pState )
{
	if ( lua_type( pState, 1 ) == LUA_TNIL )
	{
		g_pUILua->StackDump( pState, "GetStringLength : nil textId passed ... is your Sorcery up-to-date?" );
		lua_pushnumber( pState, 0 );
		return 1;
	}

	// get string / id
	C_TextStringID id;
	char pString[ 512 ];
	pString[ 0 ] = '\0';

	if ( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		id = lua_tonumber( pState, 1 );
	}
	else if( lua_type( pState, 1 ) == LUA_TSTRING )
	{
		const char* p = lua_tostring( pState, 1 );
		assert( strlen( p ) < sizeof( pString ) );
		STRCPY( pString, lengthof( pString ), p );
	}
	else
	{
		g_pUILua->StackDump( pState, "GetStringLength : invalid 'id/string' param" );
		lua_pushnumber( pState, 0 );
		return 1;
	}

	// get height
	F32 fHeight = 0.0f;

	if ( lua_type( pState, 2 ) == LUA_TNUMBER )
	{
		fHeight = (F32)lua_tonumber( pState, 2 );
	}
	else
	{
		g_pUILua->StackDump( pState, "GetStringLength : invalid 'height' param" );
		lua_pushnumber( pState, 0 );
		return 1;
	}

	// get font style
	C_FontStyle style = C_FontStyle::Instance();

	if ( lua_type( pState, 3 ) == LUA_TTABLE )
	{
		lua_settop( pState, 3 );
		style.GetFromLua( pState );
	}

	C_FontStyle* pStyle = &style;

	if ( lua_type( pState, 3 ) == LUA_TSTRING )
	{
		// resolve by name
		const char* pName = lua_tostring( pState, 3 );
		pStyle = const_cast< C_FontStyle* >( UiXt::C_FontStyleList::Instance().FindByName( pName ) );	// meh
	}

	if( !pStyle )
		pStyle = &C_FontStyle::Instance();

	// now get the length
	F32 fStringLength = 0.0f;

	if ( id.IsLocString() )
		fStringLength = gFont.StringLength( id, fHeight, *pStyle );
	else
		fStringLength = gFont.StringLength( C_UIDataFinder::Instance().GetVariableText( pString ), fHeight, *pStyle );

	lua_pushnumber( pState, fStringLength );

	return 1;
}

static int GetNumStringLines( lua_State* pState )
{
	if ( lua_type( pState, 1 ) == LUA_TNIL )
	{
		g_pUILua->StackDump( pState, "GetStringLength : nil textId passed ... is your Sorcery up-to-date?" );
		lua_pushnumber( pState, 0 );
		return 1;
	}

	// get string / id
	C_TextStringID id;
	char pString[ 512 ];
	pString[ 0 ] = '\0';

	if ( lua_type( pState,1 ) == LUA_TNUMBER )
	{
		id = lua_tonumber( pState, 1 );
	}
	else if( lua_type( pState, 1 ) == LUA_TSTRING )
	{
		const char* p = lua_tostring( pState, 1 );
		assert( strlen( p ) < sizeof( pString ) );
		STRCPY( pString, lengthof( pString ), p );
	}
	else
	{
		g_pUILua->StackDump( pState, "GetStringLength : invalid 'id/string' param" );
		lua_pushnumber( pState, 0 );
		return 1;
	}

	// get height
	F32 fHeight = 0.0f;

	if ( lua_type( pState, 2 ) == LUA_TNUMBER )
	{
		fHeight = (F32)lua_tonumber( pState, 2 );
	}
	else
	{
		g_pUILua->StackDump( pState, "GetStringLength : invalid 'height' param" );
		lua_pushnumber( pState, 0 );
		return 1;
	}

	// get wrap size
	F32 fWrapSize = 0.0f;

	if ( lua_type( pState, 3 ) == LUA_TNUMBER )
	{
		fWrapSize = (F32)lua_tonumber( pState, 3 );
	}
	else
	{
		g_pUILua->StackDump( pState, "GetStringLength : invalid 'wrap_size' param" );
		lua_pushnumber( pState, 0 );
		return 1;
	}

	// get font style
	C_FontStyle style = C_FontStyle::Instance();

	if ( lua_type( pState, 4 ) == LUA_TTABLE )
	{
		lua_settop( pState, 4 );
		style.GetFromLua( pState );
	}

	C_FontStyle* pStyle = &style;

	if ( lua_type( pState, 4 ) == LUA_TSTRING )
	{
		// resolve by name
		const char* pName = lua_tostring( pState, 4 );
		pStyle = const_cast< C_FontStyle* >( UiXt::C_FontStyleList::Instance().FindByName( pName ) );	// meh
	}

	if( !pStyle )
		pStyle = &C_FontStyle::Instance();

	// now get the number of lines
	U32 uNumLines = 0;

	if ( id.IsLocString() )
		uNumLines = gFont.StringNumLines( id, fHeight, fWrapSize, *pStyle );
	else
		uNumLines = gFont.StringNumLines( C_UIDataFinder::Instance().GetVariableText( pString ), fHeight, fWrapSize, *pStyle );

	lua_pushnumber( pState, uNumLines );

	return 1;
}

static int GetNumStringLetters( lua_State* pState )
{
	if ( lua_type( pState, 1 ) == LUA_TNIL )
	{
		g_pUILua->StackDump( pState, "GetStringLength : nil textId passed ... is your Sorcery up-to-date?" );
		lua_pushnumber( pState, 0 );
		return 1;
	}

	// Get string / id
	C_TextStringID id;
	char str[ 512 ];

	if( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		id = lua_tonumber( pState, 1 );
	}
	else if( lua_type( pState, 1 ) == LUA_TSTRING )
	{
		const char *p = lua_tostring( pState, 1 );
		assert( strlen( p ) < sizeof( str ) );
		strncpy_s( str, lengthof(str), p, _TRUNCATE ); // Remove const qualifier on pointer to LUA based string
	}
	else
	{
		g_pUILua->StackDump( pState, "GetStringLength : invalid 'id/string' param" );
		lua_pushnumber( pState, 0 );
		return 1;
	}

	// now get the number of letters
	S32 sNum = 0;
	if( id.IsLocString() )
		sNum = wcslen( id.String() );
	else
		sNum = wcslen( C_UIDataFinder::Instance().GetVariableText( str ) );

	lua_pushnumber( pState, sNum );

	return 1;
}

static int AllowedToPlayMovie(lua_State *pState)
{
#if defined(GAME_RACERNOTAMAX) && !defined(CFG_GOLD)
	lua_pushboolean( pState, (!gDisableIntroAttractMovie ? 1 : 0) );
#else
	lua_pushboolean( pState, 1 );
#endif
	return 1;
}

static int InitMovie(lua_State *pState)
{
	if(!lua_isnumber(pState,1) || !lua_isstring(pState,2)|| !lua_isboolean(pState,3))
	{
		g_pUILua->StackDump(pState,"InitMovie(slot,str,bool,[num]) - incorrect arguments");
		return 0;
	}

	if(g_pUILua->mDoingFullInit)
	{
		// [Rog] We ignore mem, as it's wrong - always stream for the F.E.
		bool mem = lua_toboolean(pState,3) ? true : false;
		int slot = (int)lua_tonumber(pState,1);

	#ifndef CFG_GOLD
		if ( g_DebugUiDisableMovie )
		{
			_UI_MESSAGE1( E_UiMsg_MovieFinished, slot );
			return 0;
		}
	#endif

		// [DG] Optional bink IO buffer size
		U32 io_memory_size = 0;
		if(lua_type(pState,4)==LUA_TNUMBER)
		{
			io_memory_size = (U32)lua_tonumber(pState,4);
		}
#ifdef _DISABLE_AUDIO
		gMovie[(int)lua_tonumber(pState,1)].UseSound(false);
#else // _DISABLE_AUDIO
//		gMovie[(int)lua_tonumber(pState,1)].UseSound(g_AudioSystemEnabled);
#endif // _DISABLE_AUDIO

		if ( lua_type( pState, 5 ) != LUA_TBOOLEAN && lua_type( pState, 6 ) != LUA_TBOOLEAN )
			gMovie[slot].Create(slot, (char *)lua_tostring(pState,2), mem, io_memory_size);
		else
		if ( lua_type( pState, 5 ) == LUA_TBOOLEAN &&  lua_type( pState, 6 ) == LUA_TNIL)
			gMovie[slot].Create(slot, (char *)lua_tostring(pState,2), mem, io_memory_size, lua_toboolean( pState, 5 ) != 0 );
		else
		if ( lua_type( pState, 6 ) == LUA_TBOOLEAN &&  lua_type( pState, 7 ) != LUA_TNUMBER)
			gMovie[slot].Create(slot, (char *)lua_tostring(pState,2), mem, io_memory_size, lua_toboolean( pState, 5 ) != 0, lua_toboolean( pState, 6 ) != 0 );
		else
			gMovie[slot].Create(slot, (char *)lua_tostring(pState,2), mem, io_memory_size, lua_toboolean( pState, 5 ) != 0, lua_toboolean( pState, 6 ) != 0, (S32)lua_tonumber( pState, 7 ) );
	}

	return 0;
}

static int RemapMovieTracks(lua_State *pState)
{
	if(	lua_type(pState,1)!=LUA_TNUMBER || 
		lua_type(pState,2)!=LUA_TNUMBER || 
		lua_type(pState,3)!=LUA_TNUMBER || 
		lua_type(pState,4)!=LUA_TNUMBER || 
		lua_type(pState,5)!=LUA_TNUMBER || 
		lua_type(pState,6)!=LUA_TNUMBER ||
		lua_type(pState,7)!=LUA_TNUMBER)
	{
		g_pUILua->StackDump(pState,"RemapMovieTracks(slot,track0,track1,track2,track3,track4,track5) - incorrect arguments");
		return 0;
	}

	const U32 slot = (U32)lua_tonumber(pState,1);

	gMovie[slot].SetTrackIDsToPlay((U32)lua_tonumber(pState,2), (U32)lua_tonumber(pState,3), (U32)lua_tonumber(pState,4), (U32)lua_tonumber(pState,5), (U32)lua_tonumber(pState,6), (U32)lua_tonumber(pState,7));

	return 0;
}

static int PauseMovie(lua_State *pState)
{
#ifndef CFG_GOLD
	if( g_DebugUiDisableMovie )
		return 0;
#endif

	if(!lua_isnumber(pState,1) && !lua_isboolean(pState,2))
	{
		g_pUILua->StackDump(pState,"PauseMovie(movie(num),pause(bool)) - incorrect arguments");
		return 0;
	}

	bool	p			= lua_toboolean(pState,2) ? true : false;
	int		movieNumber	= int(lua_tonumber(pState,1));

	gMovie[movieNumber].Pause(p);

	return 0;
}

static int EnableMovieLooping(lua_State *pState)
{
#ifndef CFG_GOLD
	if( g_DebugUiDisableMovie )
		return 0;
#endif

	if(!lua_isnumber(pState,1) && !lua_isboolean(pState,2))
	{
		g_pUILua->StackDump(pState,"EnableMovieLooping(movie(num),loop(bool)) - incorrect arguments");
		return 0;
	}
	bool	loop		= lua_toboolean(pState,2) ? true : false;
	int		movieNumber	= int(lua_tonumber(pState,1));

	gMovie[movieNumber].Looping(loop);

	return 0;
}

static int DestroyMovie(lua_State *pState)
{
#ifndef CFG_GOLD
	if( g_DebugUiDisableMovie )
		return 0;
#endif

	if(!lua_isnumber(pState,1))
	{
		g_pUILua->StackDump(pState,"DestroyMovie(slot) - incorrect arguments");
		return 0;
	}

	int movie=(int)lua_tonumber(pState,1);
	
	LOGDEBUG( g_Log, "Destroying movie number %d",movie);

	gMovie[movie].Destroy();

	return 0;
}

static int SetAsBackgroundMovie(lua_State *pState)
{
#ifndef CFG_GOLD
	if( g_DebugUiDisableMovie )
		return 0;
#endif

	if(!lua_isnumber(pState,1) && !lua_isboolean(pState,2))
	{
		g_pUILua->StackDump(pState,"SetAsBackgroundMovieMovie(movie(num),backgroundmovie(bool)) - incorrect arguments");
		return 0;
	}
	bool	backgroundmovie		= lua_toboolean(pState,2) ? true : false;
	int		movieNumber	= int(lua_tonumber(pState,1));

	gMovie[movieNumber].SetBackgroundMovie(backgroundmovie);

	return 0;
}

static int ClearStringStore(lua_State *pState)
{
	C_StringStore::Instance().ClearAll();
	return 0;
}

// Set single texture
static int SetDestroyTexture(lua_State *pState)
{
#if 1
	assert(0 && "This code path has been removed...see LloydW!");
#else
	if(lua_type(pState,1) != LUA_TNUMBER || lua_type(pState,2) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump(pState,"SetDestroyTexture(number,bool) - incorrect arguments");
		return 0;
	}

	// Set the specific texture to be flagged	
	gUITextureManager.SetDestroyTexture( (U32)lua_tonumber(pState, 1), (lua_toboolean(pState, 2) ? true : false) );
#endif
	return 0;
}

// Block set textures (non-common only)
static int SetDestroyTextures(lua_State *pState)
{
#if 1
	assert(0 && "This code path has been removed...see LloydW!");
#else
	if(lua_type(pState,1) != LUA_TBOOLEAN)
	{
		g_pUILua->StackDump(pState,"SetDestroyTextures(bool) - incorrect arguments");
		return 0;
	}

	// Set all the non-common texture flags
	gUITextureManager.SetDestroyTextures( (lua_toboolean(pState, 1) ? true : false) );
#endif
	return 0;
}

static int TextureValid(lua_State* pState)
{
	if( lua_type(pState, 1) != LUA_TSTRING )
	{
		g_pUILua->StackDump(pState,"SetDestroyTextures(string) - incorrect arguments");
		return 0;
	}

	const char *name = lua_tostring(pState,1);

	C_TextureHandleIndex index = gUITextureManager.GetTextureByName( name );
	if( !index.Active() )
		index = gSimple2D.AlternateTextureFindByName( name, true );

	lua_pushboolean( pState, index.Active() ? 1 : 0 );
	return 1;
}
static int DestroyTextures(lua_State *pState)
{
	gUITextureManager.DestroyAll();
	return 0;
}

static int ItalicMatrix(lua_State *pState)
{
	lua_pushnumber(pState, gFont.GetItalicsMatrixIndex());
	return 1;
}

static int IsPanel3D(lua_State* pState )
{
	if( lua_type(pState, 1) != LUA_TNUMBER )
	{
		g_pUILua->StackDump(pState,"Panel3D( number ) - incorrect arguments");
		return 0;
	}

	const PanelIndex panel = (PanelIndex)lua_tointeger(pState, 1);

	lua_pushboolean( pState, gSimple2D.IsPanel3D( panel ) ? 1 : 0 );
	return 1;
}

static int SetCameraScale(lua_State* pState)
{
	if( lua_type(pState, 1) != LUA_TBOOLEAN || lua_type(pState, 2) != LUA_TNUMBER || lua_type(pState, 3) != LUA_TNUMBER )
	{
		g_pUILua->StackDump(pState,"SetCameraScale( bool, number, number, [number] ) - incorrect arguments");
		return 0;
	}

	bool bIsThreeD	= (lua_toboolean( pState, 1 ) != 0 );
	S32 iIndex		= (S32)lua_tonumber( pState, 2 );

	if( bIsThreeD )
	{
		F32 fScale		= (F32)lua_tonumber( pState, 3 );

		C_UICamera::Instance().GetSettings3D( iIndex ).Scale( fScale );
	}
	else
	{
		if( lua_type(pState, 4) != LUA_TNUMBER )
		{
			g_pUILua->StackDump(pState,"SetCameraScale( bool, number, number, [number] ) - incorrect arguments");
			return 0;
		}
	
		F32 fScaleX		= (F32)lua_tonumber( pState, 3 );
		F32 fScaleY		= (F32)lua_tonumber( pState, 4 );

		C_UICamera::Instance().GetSettings2D( iIndex ).ScaleX( fScaleX );
		C_UICamera::Instance().GetSettings2D( iIndex ).ScaleY( fScaleY );
	}

	return 0;
}

static int GetMouseState(lua_State *pState)
{
	bool active = false;
	F32 x = 320.0f;
	F32 y = 240.0f;

#ifdef PLATFORM_PC
	active = UiInterface()->GetMousePos(x, y);

	if(active)
	{
		x *= gSimple2D.ScreenWidth();
		x += gSimple2D.ScreenLeft();

		y *= UI_VSCREEN_SIZEY;
	}
#endif
	lua_pushboolean(pState, active);
	lua_pushnumber(pState, x);
	lua_pushnumber(pState, y);

	return 3;
}

static int UseMouseInput(lua_State *pState)
{
	lua_pushboolean(pState, UiInterface()->UseMouseInput());
	return 1;
}

static int LaunchXboxGuide( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "LaunchXboxGuide( pad ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	if ( sPadNo < 0 || sPadNo > 4 )
		return 0;

#ifdef LIVE_ON_WINDOWS
	XShowGuideUI( (DWORD)sPadNo );
#endif
	
	return 0;
}

// Render
static S32 Render( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "Render( bool ) - incorrect arguments" );
		return 0;
	}

	g_UiDebugDoRender = ( lua_toboolean( pState, 1 ) != 0 );

	return 0;
}

static S32 SystemMessageNoCable( lua_State* pState )
{
#ifdef PLATFORM_PS3
	cellMsgDialogOpenErrorCode( CELL_NET_CTL_ERROR_NET_CABLE_NOT_CONNECTED, NULL, NULL, NULL );
#endif // PLATFORM_PS3
	return 0;
}

static S32 SystemMessageNotSignedIn( lua_State* pState )
{
#ifdef PLATFORM_PS3
    cellMsgDialogOpenErrorCode( CELL_NET_CTL_ERROR_NP_NO_ACCOUNT, NULL, NULL, NULL );
#endif // PLATFORM_PS3
	return 0;
}

// [Blur2PC] retail Blur PC (Blur.exe 1.2, T146): quit-to-desktop request. Name ours (no symbols); retail
// keeps it at 0x01162f9c. Set by UISystem.SystemQuitGame, read by C_UiManager::Process.
bool g_Blur2PCSystemQuitGame = false;

// [Blur2PC] retail 0x00d5fda0: UISystem.SystemQuitGame( bool ), called by the KillApplication popup
static int SystemQuitGame( lua_State* pState )
{
	if( !lua_isboolean( pState, 1 ) )
	{
		g_pUILua->StackDump( pState, "Quit Game - incorrect arguments" );
		return 0;
	}

	if( lua_toboolean( pState, 1 ) )
		g_Blur2PCSystemQuitGame = true;

	return 0;
}

static int SystemTicks( lua_State* pState )
{
	double time = Time::GetPerformanceCounter() / 1000000.0;

	lua_pushnumber( pState, time );

	return 1;
}

static const luaL_reg system_lib[] = {
	{"PlaySound",					PlaySound },
	{"LoadLuaScript",				LoadLuaScript },
	{"MakeNumberString",			MakeNumberString },

	{"GetStringLength",				GetStringLength },		// please don't call this every frame!
	{"GetNumStringLines",			GetNumStringLines },	// please don't call this every frame!

	{"GetNumStringLetters", 		GetNumStringLetters },
	{"AllowedToPlayMovie",			AllowedToPlayMovie },
	{"InitMovie",					InitMovie },
	{"RemapMovieTracks",			RemapMovieTracks },
	{"PauseMovie",					PauseMovie },
	{"EnableMovieLooping",			EnableMovieLooping },
	{"DestroyMovie",				DestroyMovie },
	{"SetAsBackgroundMovie",		SetAsBackgroundMovie },
	{"ClearStringStore",			ClearStringStore },
	{"SetDestroyTexture",			SetDestroyTexture },
	{"SetDestroyTextures",			SetDestroyTextures },
	{"DestroyTextures",				DestroyTextures },
	{"TextureValid",				TextureValid },
	{"ItalicMatrix",				ItalicMatrix },
	{"GetMouseState",				GetMouseState },
	{"UseMouseInput",				UseMouseInput },
	{"LaunchXboxGuide",				LaunchXboxGuide },
	{"IsPanel3D",					IsPanel3D },
	{"SetCameraScale",				SetCameraScale },

	{"SystemMessageNoCable",		SystemMessageNoCable },
	{"SystemMessageNotSignedIn",	SystemMessageNotSignedIn },

	{"Ticks",						SystemTicks },

	{"SystemQuitGame",				SystemQuitGame },	// [Blur2PC] retail Blur PC (T146)

	{"Render",				Render },

	{NULL,			NULL}		// end of list
};

LUALIB_API int luaopen_system (lua_State *pState)
{
	luaL_openlib(pState,"UISystem",system_lib,0);

	return 1;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
