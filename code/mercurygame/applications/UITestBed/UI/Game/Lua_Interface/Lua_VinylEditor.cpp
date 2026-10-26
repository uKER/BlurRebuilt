#include "Stdafx.h"

#include <ui/UIDefines.h>
#include <ui/Game/UILUA.h>

#include "ui/Game/Gadgets/VinylManager.h"
#include "ui/Game/Gadgets/UIVinylScene.h"

#include "ui/Game/Gadgets/UIColourPicker.h"

namespace UI
{

static int Init( lua_State* pState )
{
	C_VinylManager::Instance().Init( 1000 );
	return 0;
}

static int End( lua_State* pState )
{
	C_VinylManager::Instance().End();
	return 0;
}

static int ClearScene( lua_State* pState )
{
	C_VinylManager::Instance().ClearScene();
	return 0;
}

static int SelectedSetColour( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SelectedSetColour( number, number ) - incorrect arguments" );
		return 0;
	}
	const U32 edge_index = (U32)lua_tonumber( pState, 1 );
	const U16 col_index = (U16)lua_tonumber( pState, 2 );

	C_VinylManager::Instance().SelectedColourIndex( edge_index, col_index );

	return 0;
}

static int SelectedGetUseGradient( lua_State* pState )
{
	const bool useGradient = C_VinylManager::Instance().SelectedUseGradient();

	lua_pushboolean( pState, useGradient ? 1 : 0 );
	return 1;
}

static int SelectedSetUseGradient( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TBOOLEAN )
	{
		g_pUILua->StackDump( pState, "SelectedSetUseGradient( bool ) - incorrect arguments" );
		return 0;
	}

	const bool useGradient = (lua_toboolean( pState, 1 ) != 0 );

	C_VinylManager::Instance().SelectedUseGradient( useGradient );

	return 0;
}

static int SelectedGetColour( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SelectedGetColour( number ) - incorrect arguments" );
		return 0;
	}

	const U32 edge_index = ((U32)lua_tonumber( pState, 1 ) == 0) ? 0 : 1;

	U16 colour_index = C_VinylManager::Instance().SelectedColourIndex( edge_index );

	lua_pushnumber( pState, (lua_Number)colour_index );
	return 1;
}

static int ResolveColourARGB( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "ResolveColourARGB( number, number ) - incorrect arguments" );
		return 0;
	}

	U16 index = (U16)lua_tonumber( pState, 1 );
	F32 alpha = (F32)lua_tonumber( pState, 2 );

	C_UIColour col = C_VinylManager::Instance().ResolveColour( index, alpha );

	lua_pushnumber( pState, (F32)col.A() );
	lua_pushnumber( pState, (F32)col.R() );
	lua_pushnumber( pState, (F32)col.G() );
	lua_pushnumber( pState, (F32)col.B() );

	return 4;
}

static int SelectedGetAlpha( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "GetAlpha( number ) - incorrect arguments" );
		return 0;
	}

	const U32 edge_index = ((U32)lua_tonumber( pState, 1 ) == 0) ? 0 : 1;

	F32 alpha = C_VinylManager::Instance().SelectedAlpha( edge_index );

	lua_pushnumber( pState, (lua_Number)alpha );

	return 1;
}

static int SelectedSetAlpha( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetAlpha( number, number ) - incorrect arguments" );
		return 0;
	}

	const U32 edge_index = ((U32)lua_tonumber( pState, 1 ) == 0) ? 0 : 1;
	const F32 alpha = (F32)lua_tonumber( pState, 2 );

	C_VinylManager::Instance().SelectedAlpha( edge_index, alpha );

	return 0;
}

static int SelectedSetShape( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SelectedSetShape( number ) - incorrect arguments" );
		return 0;
	}

	U16 shapeIndex = (U16)lua_tonumber(pState,1);
	C_VinylManager::Instance().SelectedShapeIndex( shapeIndex );

	return 0;
}

static int SelectedResetItem( lua_State* pState )
{
	C_VinylManager::Instance().SelectedResetItem();

	return 0;
}

static int InsertNewItem( lua_State* pState )
{
	C_VinylManager::Instance().InsertNewItem();

	return 0;
}

static int SelectedEnterGroup( lua_State* pState )
{
	C_VinylManager::Instance().SelectedEnterGroup();

	return 0;
}

static int SelectedExitGroup( lua_State* pState )
{
	C_VinylManager::Instance().SelectedExitGroup();

	return 0;
}

static int SelectedExplodeGroup( lua_State* pState )
{
	C_VinylManager::Instance().SelectedExplodeGroup();
	return 0;
}

static int SelectedGetShape( lua_State* pState )
{
	U16 shapeIndex = C_VinylManager::Instance().SelectedShapeIndex();
	lua_pushnumber( pState, shapeIndex );

	return 1;
}

static int Cut( lua_State* pState )
{
	C_VinylManager::Instance().Cut();
	
	return 0;
}

static int Copy( lua_State* pState )
{
	C_VinylManager::Instance().Copy();

	return 0;
}

static int Paste( lua_State* pState )
{
	C_VinylManager::Instance().Paste();

	return 0;
}

static int Stamp( lua_State* pState )
{
	C_VinylManager::Instance().Stamp();

	return 0;
}

static int SetEditMode( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "SetEditMode( number ) - incorrect arguments" );
		return 0;
	}

	C_VinylManager::E_EditMode editMode = (C_VinylManager::E_EditMode)(S32)lua_tonumber( pState, 1 );
	// Set
	C_VinylManager::Instance().EditMode( editMode );

	return 0;
}

static int GetEditMode( lua_State* pState )
{
	C_VinylManager::E_EditMode editMode = C_VinylManager::Instance().EditMode();

	lua_pushnumber( pState, (lua_Number)editMode );
	return 1;
}

static int IsEmptySlot( lua_State* pState )
{
	VinylItem* p = C_VinylManager::Instance().scList().At( C_VinylManager::Instance().scCurrent() );

	lua_pushboolean( pState, ( p->Shape() == 0xffff && p->child < 0 ) ? 1 : 0 );

	return 1;
}

static int IsGroupSlot( lua_State* pState )
{
	VinylItem* p = C_VinylManager::Instance().scList().At( C_VinylManager::Instance().scCurrent() );
	lua_pushboolean( pState, ( p && p->child >= 0 ) ? 1 : 0 );

	return 1;
}

static int IsRoot( lua_State* pState )
{
	VinylItem* p = C_VinylManager::Instance().scList().At( C_VinylManager::Instance().scCurrent() );

	lua_pushboolean( pState, ( p && p->parent == 0 ) ? 1 : 0 );

	return 1;
}


static int PickerGetColour( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "PickerGetColour( number ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if ( !pObject )
		return 0;

	// Type check
	if( pObject->GetType() != E_UIGameButtonType_ColourPicker )
		return 0;

	UIColourPicker* pPicker = (UIColourPicker*)pObject;

	const S32 col_index = pPicker->GetColourIndex();
	const S32 int_index = pPicker->GetIntensityIndex();

	if( col_index < 0 || col_index > 0xff || int_index < 0 || int_index > 0xff )
		LOGWARN( g_Log, DBSTR "Colour index %d or %d is out of range", col_index, int_index );

	U16 colour_index = (col_index<<8) | int_index;
		
	lua_pushnumber( pState, (lua_Number)colour_index );
	return 1;
}

static int PickerSetColour( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "PickerSetColour( number, number ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if ( !pObject )
		return 0;

	// Type check
	if( pObject->GetType() != E_UIGameButtonType_ColourPicker )
		return 0;

	UIColourPicker* pPicker = (UIColourPicker*)pObject;

	const S32 index = (S32)lua_tonumber( pState, 2 );

	const S32 col_index = index >> 8;
	const S32 int_index = index & 0xff;

	pPicker->SetColourIndex( col_index );
	pPicker->SetIntensityIndex( int_index );

	return 0;
}

static int PickerSetAlpha( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "PickerSetAlpha( number, number ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if ( !pObject )
		return 0;

	// Type check
	if( pObject->GetType() != E_UIGameButtonType_ColourPicker )
		return 0;

	UIColourPicker* pPicker = (UIColourPicker*)pObject;

	pPicker->SetAlpha( (F32)lua_tonumber( pState, 2 ) );

	return 0;
}

static int PickerGetAlpha( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "PickerGetAlpha( number ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if ( !pObject )
		return 0;

	// Type check
	if( pObject->GetType() != E_UIGameButtonType_ColourPicker )
		return 0;

	UIColourPicker* pPicker = (UIColourPicker*)pObject;

	lua_pushnumber( pState, pPicker->GetAlpha() );

	return 1;
}

static int PickerGetShape( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "PickerGetShape( number ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if ( !pObject )
		return 0;

	// TODO

	return 0;
}

static int PickerSetShape( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "PickerSetShape( number, number ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );
	if ( !pObject )
		return 0;

	// TODO
	return 0;
}

static int FileToClipboard( lua_State* pState )
{
	VinylList& sl = C_VinylManager::Instance().scScratchPadList();
	VinylList& cl = C_VinylManager::Instance().scClipboardList();

	cl.Clear();
	CopySub( &cl, cl.Root(), &sl, sl.Root() );

	return 0;
}

static int ClipboardToFile( lua_State* pState )
{
	VinylList& sl = C_VinylManager::Instance().scScratchPadList();
	VinylList& cl = C_VinylManager::Instance().scClipboardList();

	sl.Clear();
	CopySub( &sl, sl.Root(), &cl, cl.Root() );

	return 0;
}

static int FileToScene( lua_State* pState )
{
	VinylList& sl = C_VinylManager::Instance().scScratchPadList();
	VinylList& rl = C_VinylManager::Instance().scList();

	rl.Clear();
	CopySub( &rl, rl.Root(), &sl, sl.Root() );
	return 0;
}

static int SceneToFile( lua_State* pState )
{
	VinylList& sl = C_VinylManager::Instance().scScratchPadList();
	VinylList& rl = C_VinylManager::Instance().scList();

	sl.Clear();
	CopySub( &sl, sl.Root(), &rl, rl.Root() );

	return 0;
}

static int SceneToAvatar( lua_State* pState )
{
	VinylList& al = C_VinylManager::Instance().scAvatarList();
	VinylList& rl = C_VinylManager::Instance().scList();

	al.Clear();
	CopySub( &al, al.Root(), &rl, rl.Root() );

	return 0;
}

static int AvatarToScene( lua_State* pState )
{
	VinylList& rl = C_VinylManager::Instance().scList();
	VinylList& al = C_VinylManager::Instance().scAvatarList();

	rl.Clear();
	CopySub( &rl, rl.Root(), &al, al.Root() );

	return 0;
}

static int SceneType( lua_State* pState )
{
	if( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		// Set
		C_VinylManager::Instance().SceneType( (C_VinylManager::E_SceneType)(S32)lua_tonumber( pState, 1 ) );
		return 0;
	}

	if( lua_type( pState, 1 ) == LUA_TNONE )
	{
		// Get
		lua_pushinteger( pState, (S32)C_VinylManager::Instance().SceneType() );
		return 1;

	}

	g_pUILua->StackDump( pState, "SceneType( number | none ) - incorrect arguments" );
	return 0;
}

static int CurrentList( lua_State* pState )
{
	if( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		// Set
		C_VinylManager::Instance().CurrentList( (C_VinylManager::E_List)(S32)lua_tonumber( pState, 1 ) );
		return 0;
	}

	if( lua_type( pState, 1 ) == LUA_TNONE )
	{
		// Get
		lua_pushinteger( pState, (S32)C_VinylManager::Instance().CurrentList() );
		return 1;

	}

	g_pUILua->StackDump( pState, "CurrentList( number | none ) - incorrect arguments" );
	return 0;
}

static int SelectedToRoot( lua_State* pState )
{
	C_VinylManager::Instance().SelectedToRoot();

	return 0;
}

static int RenderNow( lua_State* pState )
{
	C_VinylManager::Instance().ShouldRender( C_VinylManager::E_ShouldRender_Render );

	return 0;
}

// the library
static const luaL_reg vinyl_editor[] = 
{
	{ "Init",						Init },
	{ "End",						End },
	{ "ClearScene",					ClearScene },

	{ "SelectedToRoot",				SelectedToRoot },
	{ "SelectedSetUseGradient",		SelectedSetUseGradient },
	{ "SelectedGetUseGradient",		SelectedGetUseGradient },
	{ "SelectedSetColour",			SelectedSetColour },
	{ "SelectedGetColour",			SelectedGetColour },
	{ "SelectedSetAlpha",			SelectedSetAlpha },
	{ "SelectedGetAlpha",			SelectedGetAlpha },
	{ "SelectedSetShape",			SelectedSetShape },
	{ "SelectedGetShape",			SelectedGetShape },
	{ "SelectedResetItem",			SelectedResetItem },
	{ "SelectedEnterGroup",			SelectedEnterGroup },
	{ "SelectedExitGroup",			SelectedExitGroup },
	{ "SelectedExplodeGroup",		SelectedExplodeGroup },

	{ "InsertNewItem",				InsertNewItem },

	{ "Cut",						Cut },
	{ "Copy",						Copy },
	{ "Paste",						Paste },
	{ "Stamp",						Stamp },

	{ "GetEditMode",				GetEditMode },
	{ "SetEditMode",				SetEditMode },

	{ "SceneType",					SceneType },
	{ "CurrentList",				CurrentList },
	
	{ "IsEmptySlot",				IsEmptySlot },
	{ "IsGroupSlot",				IsGroupSlot },
	{ "IsRoot",						IsRoot },

	{ "PickerGetColour",			PickerGetColour },
	{ "PickerSetColour",			PickerSetColour },
	{ "PickerSetAlpha",				PickerSetAlpha },
	{ "PickerGetAlpha",				PickerGetAlpha },
	{ "PickerGetShape",				PickerGetShape },
	{ "PickerSetShape",				PickerSetShape },

	{ "FileToClipboard",			FileToClipboard },
	{ "FileToScene",				FileToScene },
	{ "ClipboardToFile",			ClipboardToFile },
	{ "SceneToFile",				SceneToFile },
	{ "SceneToAvatar",				SceneToAvatar },
	{ "AvatarToScene",				AvatarToScene },

	{ "RenderNow",					RenderNow },
	{ "ResolveColourARGB",			ResolveColourARGB },

	// end of list
	{ NULL, NULL }
};

LUALIB_API int luaopen_vinyl_editor( lua_State* pState )
{
	luaL_openlib( pState, "VinylEditor", vinyl_editor, 0 );
	return 1;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on)
#endif
