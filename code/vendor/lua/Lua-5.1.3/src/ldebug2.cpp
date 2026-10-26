
#include "stdafx.h"

extern "C"
{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lbaselib_c

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
}

static Debug::Logger &g_luaLogger = Debug::GetLogger("mercury.lua");

extern "C" int luaB_print (lua_State *L)
{
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	Util::C_String str;
	for (i=1; i<=n; i++)
	{
		const char *s;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  /* get result */
		if (s == NULL)
			return luaL_error(L, "`tostring' must return a string to `print'");

#if !defined(CFG_GOLD)
		//if (i>1) LOGINFO(g_luaLogger, "\t"); // fputs("\t", stdout);
		//fputs(s, stdout);
		//LOGINFO(g_luaLogger, s);
		//Debug::ConsoleLn("UiLua",s);
		if (i>1) str+="\t";
		str+=s;
#endif

		lua_pop(L, 1);  /* pop result */
	}


#if !defined(CFG_GOLD)
	LOGINFO(g_luaLogger, str.Chars());
	Debug::ConsoleLn("UiLua",str.Chars());
	//fputs("\n", stdout);
	//LOGINFO(g_luaLogger, "\n");	//removed because LOGINFO puts a carriage return in as well
#endif
	return 0;
}
