#ifndef LUAACTORDECLARE_A4AB3A0E_BC79_4FA5_A6F3_CA5AE0EBCC9E_H
#define LUAACTORDECLARE_A4AB3A0E_BC79_4FA5_A6F3_CA5AE0EBCC9E_H

#ifdef _MSC_VER
#pragma once
#endif


#define LUA_ACTORS_CHECKS (!(CFG_GOLD))


struct lua_State;


namespace LuaActor
{

class C_LuaActor;
class C_LuaActorCreate;
class C_LuaActorMessageFactory;
class C_LuaActorScript;

class I_LuaActorContext;

} // namespace LuaActor


#endif // LUAACTORDECLARE_A4AB3A0E_BC79_4FA5_A6F3_CA5AE0EBCC9E_H
