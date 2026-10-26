#ifndef LUAACTORUTILS_646CE0C7_387E_41C7_B362_25AC0813AED9_H
#define LUAACTORUTILS_646CE0C7_387E_41C7_B362_25AC0813AED9_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <lua_actors/LuaActorDeclare.h>


namespace LuaActor
{

extern Debug::Logger& GetDebugLogger();

Actor::C_ActorHandle ToActorHandle(lua_State* pLuaState, const int index);
void PushActorHandle(lua_State* pLuaState, const Actor::C_ActorHandle& actorHandle);

Maths::HVec3 ToHVec3(lua_State* pLuaState, const int index);
void PushHVec3(lua_State* pLuaState, const Maths::HVec3 PCREF vector);

Maths::HMat43 ToHMat43(lua_State* pLuaState, const int index);
void PushHMat43(lua_State* pLuaState, const Maths::HMat43& matrix);

} // namespace LuaActor


#endif // LUAACTORUTILS_646CE0C7_387E_41C7_B362_25AC0813AED9_H
