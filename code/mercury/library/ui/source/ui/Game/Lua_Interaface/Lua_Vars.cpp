#include "stdafx.h"

#include "ui/Game/UILua.h"
#include "ui/Game/UIVariableStore.h"

namespace UI
{

int UIVars_Set(lua_State *pState)
{
	if(!lua_isstring(pState,1) || !lua_isnumber(pState,2))
	{
		g_pUILua->StackDump(pState,"UIVars_Set(str(string),var(num)) - incorrect arguments\n");
		return 0;
	}

	const char *str = lua_tostring(pState,1);
	int var=(int)lua_tonumber(pState,2);

	if(lua_isnumber(pState,3))
		GetVariableStore()->Set(str,var,(S32)(lua_tonumber(pState,3)-1.0f));
	else
		GetVariableStore()->Set(str,var);

	return 0;
}

int UIVars_Get(lua_State *pState)
{
	if(!lua_isstring(pState,1))
	{
		g_pUILua->StackDump(pState,"UIVars_Get(str(string)) - incorrect arguments\n");
		return 0;
	}
	
	const char *str = lua_tostring(pState,1);

	if(lua_isnumber(pState,2))
		lua_pushnumber(pState, GetVariableStore()->Get(str, (S32)(lua_tonumber(pState,2)-1.0f) ));
	else
		lua_pushnumber(pState, GetVariableStore()->Get(str));
	return 1;
}



// defaults all variables stored within GetVariableStore()->
int UIVars_Reset(lua_State* pState)
{
	GetVariableStore()->Default();
	return 0;
}

static const luaL_reg variables_lib[] = {
	{"Set",					UIVars_Set},
	{"Get",					UIVars_Get},
	{"Reset",				UIVars_Reset},

	{NULL,			NULL}		// end of list
};

LUALIB_API int luaopen_vars (lua_State *pState)
{
	luaL_openlib(pState,"UIVars",variables_lib,0);

	return 1;
}

}; // namespace UI
