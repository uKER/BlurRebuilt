
#ifndef _LUA_DEBUGGER_H_
#define _LUA_DEBUGGER_H_

#ifdef _MSC_VER
#pragma once
#endif

struct lua_State;

namespace LuaDebugger
{

int InitialiseNetworkConnection(const Util::C_String& scriptRoot);

void Update(lua_State *L);

}

#endif
