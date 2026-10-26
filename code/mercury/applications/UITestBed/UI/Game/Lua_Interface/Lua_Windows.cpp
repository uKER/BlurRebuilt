#include "Stdafx.h"

#include <ui/UIDefines.h>
#include <ui/Game/UILua.h>

#include "ui/Game/Gadgets/Windows/UIFrame.h"
#include "ui/Game/Gadgets/Windows/UICanvas.h"

#include "ui/Game/Gadgets/Windows/UIScrollBar.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{


static S32 AddItem( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "AddItem( id, itemId ) - incorrect arguments" );
		return 0;
	}

	S32 id = (S32)lua_tonumber( pState, 1 );
	S32 itemId = (S32)lua_tonumber( pState, 2 );

	C_GadgetBase* pGadget = gButtons.Find( id );
	C_GadgetBase* pItem = gButtons.Find( itemId );

	if ( pGadget && pItem )
	{
		if ( pGadget->GetType() == E_UIButtonType_Frame )
		{
			((UIFrame*)pGadget)->AddItem( pItem );
		}
		else
		if ( pGadget->GetType() == E_UIButtonType_Canvas )
		{
			((UICanvas*)pGadget)->AddItem( pItem );
		}
	}
	else
		LOGWARN( g_Log, DBSTR "Cannot find gadgets!" );

	return 0;
}


/*---------------------------------------------------------------------------------------
	Scrollbar Manipulation functions.
---------------------------------------------------------------------------------------*/
static S32 SetSliderPosition( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetSliderPosition( id, position ) - incorrect arguments" );
		return 0;
	}

	S32 id = (S32)lua_tonumber( pState, 1 );
	F32 position = (F32)lua_tonumber( pState, 2 );

	C_GadgetBase* pGadget = gButtons.Find( id );

	if ( pGadget && pGadget->GetType() == E_UIButtonType_ScrollBar )
	{
		((UIScrollBar*)pGadget)->SetSliderPosition( position );
	}
	else
	{
		LOGWARN( g_Log, DBSTR "Gadget not found!" );
	}
	return 0;
}


static S32 SetScrollBounds( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER || lua_type( pState, 3 ) != LUA_TNUMBER)
	{
		g_pUILua->StackDump( pState, "SetSliderPosition( id, min, max ) - incorrect arguments" );
		return 0;
	}

	S32 id = (S32)lua_tonumber( pState, 1 );

	F32 min = (F32)lua_tonumber( pState, 2 );
	F32 max = (F32)lua_tonumber( pState, 3 );

	C_GadgetBase* pGadget = gButtons.Find( id );

	if ( pGadget && pGadget->GetType() == E_UIButtonType_ScrollBar )
	{
		((UIScrollBar*)pGadget)->SetBounds( min, max );
	}
	else
	{
		LOGWARN( g_Log, DBSTR "Gadget not found!" );
	}
	return 0;
}


static S32 SetScrollArea( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetSliderPosition( id, size ) - incorrect arguments" );
		return 0;
	}

	S32 id = (S32)lua_tonumber( pState, 1 );

	F32 size = (F32)lua_tonumber( pState, 2 );

	C_GadgetBase* pGadget = gButtons.Find( id );

	if ( pGadget && pGadget->GetType() == E_UIButtonType_ScrollBar )
	{
		((UIScrollBar*)pGadget)->SetScrollArea( size );
	}
	else
	{
		LOGWARN( g_Log, DBSTR "Gadget not found!" );
	}
	return 0;
}


// the library
static const luaL_reg game_windows_lib[] = 
{
	{ "AddItem",				AddItem },

	// Scrollbar functions.
	{ "SetSliderPosition",		SetSliderPosition },
	{ "SetScrollBounds",		SetScrollBounds },
	{ "SetScrollArea",			SetScrollArea },

	// end of list
	{ NULL, NULL }
};

LUALIB_API int luaopen_game_windows( lua_State* pState )
{
	luaL_openlib( pState, "UIWindows", game_windows_lib, 0 );
	return 1;
}

}; // namespace UI


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on)
#endif
