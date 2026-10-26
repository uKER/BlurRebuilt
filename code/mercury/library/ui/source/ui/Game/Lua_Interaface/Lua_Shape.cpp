#include "stdafx.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

#include "Ui/Illustrator/UIAdobeSceneManager.h"
#include "Ui/Illustrator/UIAdobeScene.h"

// Gadgets
#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/Gadgets/BaseUIShape.h"

namespace UI
{

/*---------------------------------------------------------------------------------------
	Name : ChangeSceneName

	Desc : Sets the scene name which the given static shape object references
---------------------------------------------------------------------------------------*/
static int ChangeSceneName( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TSTRING )
	{
		g_pUILua->StackDump( pState, "ChangeSceneName( num(id), string(name) ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pGadget = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if( !pGadget )
		return 0;

	// need a way of type checking this.
	bool result = ((BaseUIShape*)pGadget)->ChangeSceneName( (const char*)lua_tostring( pState, 2 ) );

	lua_pushboolean( pState, result ? 1 : 0 );
	return 1;
}


/*---------------------------------------------------------------------------------------
	Name : ChangeObjectName

	Desc : Sets the object name which the given static shape object references
---------------------------------------------------------------------------------------*/
static int ChangeObjectName( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TSTRING )
	{
		g_pUILua->StackDump( pState, "ChangeObjectName( num(id), string(name) ) - incorrect arguments" );
		return 0;
	}

	C_GadgetBase *pGadget = gButtons.Find( (U32)lua_tonumber( pState, 1 ) );
	if( !pGadget )
		return 0;

	// need a way of type checking this.
	bool result = ((BaseUIShape*)pGadget)->ChangeObjectName( (const char*)lua_tostring( pState, 2 ) );
	
	lua_pushboolean( pState, result ? 1 : 0 );
	return 1;
}


/*---------------------------------------------------------------------------------------
	Name : NumberOfSceneShapes

	Desc : Returns the number of shapes that exist in a loaded scene
---------------------------------------------------------------------------------------*/
static int NumberOfSceneShapes( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TSTRING )
	{
		g_pUILua->StackDump( pState, "NumberOfShapes( name ) - incorrect arguments\n" );
		return 0;
	}

	const C_AdobeHandle* pHandle = C_AdobeSceneManager::Instance().GetHandleByName( lua_tostring( pState, 1 ) );
	U32 count = 0;

	if ( pHandle )
		count = pHandle->ScenePtr()->GetTotalNumberOfShapes();
	
	lua_pushnumber( pState, count );
	return 1;
}


/*---------------------------------------------------------------------------------------
	Name : SetVertexParams

	Desc : Sets the min and max vertex count for a shape outline
---------------------------------------------------------------------------------------*/
static S32 SetVertexParams( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER || lua_type( pState, 2 ) != LUA_TNUMBER || lua_type( pState, 3 ) != LUA_TNUMBER )
		return 0;

	C_GadgetBase* pObject = gButtons.Find( (S32)lua_tonumber( pState, 1 ) );

	if ( pObject && pObject->GetType() == E_UIButtonType_StaticShape )
		((BaseUIShape*)pObject)->SetVertexParams( (F32)lua_tonumber( pState, 2 ), (F32)lua_tonumber( pState, 3 ) );

	return 0;
}


static const luaL_reg shape_lib[] = 
{
	{"ChangeObjectName",		ChangeObjectName },
	{"ChangeSceneName",			ChangeSceneName },
	{"NumberOfSceneShapes",		NumberOfSceneShapes },
	{"SetVertexParams",			SetVertexParams },

	{NULL,			NULL}		// end of list
};

LUALIB_API int luaopen_shape (lua_State *pState)
{
	luaL_openlib( pState, "UIShape", shape_lib, 0 );

	return 1;
}

}; // namespace UI


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on)
#endif