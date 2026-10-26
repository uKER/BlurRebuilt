#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

#include "stdafx.h"
#include "GameInterface.h"

#include "ui/UiManager.h"
#include "ui/UiMessage.h"
#include "ui/UiMessageStack.h"

namespace UI
{
	static int AddMessage(lua_State *state)
	{
		// number of args
		int numArgs = lua_gettop(state);
		int i = 1;
				
		if(!lua_isstring(state, i))
		{
			g_pUILua->StackDump(state, "AddMessage(string, ...) - incorrect arguments\n");
			return 0;
		}
		// create a game message
		size_t len;
		const char* header = lua_tolstring(state, i, &len);
		UI::MessageRef message = UI::Message::create(header);
		
		for(i = 2; i <= numArgs; i++)
		{
			int type = lua_type(state, i);
			switch(type)
			{
			default:
				{
					LOGERROR(g_Log, "<GameInterface> Unknow lua type(%d).", type); 
					break;
				}
			case LUA_TLIGHTUSERDATA:
			case LUA_TTHREAD:
			case LUA_TUSERDATA:
			case LUA_TFUNCTION:
			case LUA_TTABLE:
				{
					LOGERROR(g_Log, "<GameInterface> unsupported type(%d).", type); 
					break;
				}
			case LUA_TNIL:
				{
					LOGERROR(g_Log, "<GameInterface> LUA_TNIL argument type."); 
					break;
				}
			case LUA_TBOOLEAN:
				{
					bool number = (bool) (lua_toboolean(state, i) != 0);
					message->pushBool(number);
					break;
				}
			case LUA_TNUMBER:
				{
					lua_Number number = lua_tonumber(state, i);
					message->pushReal((float)number);
					break;
				}
			case LUA_TSTRING:
				{
					size_t len;
					const char* string = lua_tolstring(state, i, &len);
					message->pushString(string);
					break;
				}
			}
		}

		UI::MessageStack::instance()->pushMessage(message);
		return 0;
	}

	static const luaL_reg game_lib[] = 
	{
		{"AddMessage",	AddMessage},
		{NULL,			NULL}
	};

	LUALIB_API int luaopen_game(lua_State *pState)
	{
		luaL_openlib(pState,"GameInterface", game_lib,0);
		return 1;
	}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

