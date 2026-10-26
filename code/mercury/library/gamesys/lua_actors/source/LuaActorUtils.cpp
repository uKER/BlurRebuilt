#include "LuaActorsPch.h"

#include <lua_actors/LuaActorUtils.h>

#include <lapi.h>


namespace LuaActorUtils_Impl
{

const char* g_VectorComponentNames [] = { "x", "y", "z" };
const char* g_MatrixComponentNames [] = { "ax", "ay", "az", "aw" };

};


namespace LuaActor
{

Debug::Logger& GetDebugLogger()
{
	static Debug::Logger& logger = Debug::GetLogger("mercury.gamesys.LuaActors");
	return logger;
}

Actor::C_ActorHandle ToActorHandle(lua_State* pLuaState, const int index)
{
#if LUA_ACTORS_CHECKS
	// check that this is a valid stack index
	if ((index == 0) || (abs(index) > lua_gettop(pLuaState)))
	{
		LOGERROR(GetDebugLogger(), DBSTR "stack index %d out of range %d", index, lua_gettop(pLuaState));
		return Actor::C_ActorHandle();
	}

	// check that it contains userdata
	if (!lua_isuserdata(pLuaState, index))
	{
		LOGERROR(GetDebugLogger(), DBSTR "stack index %d has type %s", index, lua_typename(pLuaState, lua_type(pLuaState, index)));
		return Actor::C_ActorHandle();
	}
#endif // LUA_ACTORS_CHECKS

	void* pUserData = lua_touserdata(pLuaState, index);
	return *reinterpret_cast<const Actor::C_ActorHandle*>(&pUserData);
}

void PushActorHandle(lua_State* pLuaState, const Actor::C_ActorHandle& actorHandle)
{
	lua_pushlightuserdata(pLuaState, *reinterpret_cast<void* const*>(&actorHandle));
}

Maths::HVec3 ToHVec3(lua_State* pLuaState, const int index)
{
#if LUA_ACTORS_CHECKS
	// check that this is a valid stack index
	if ((index == 0) || (abs(index) > lua_gettop(pLuaState)))
	{
		LOGERROR(GetDebugLogger(), DBSTR "stack index %d out of range %d", index, lua_gettop(pLuaState));
		return Maths::Constants<Maths::HVec3>::Zero();
	}

	// check that it contains a table
	if (!lua_istable(pLuaState, index))
	{
		LOGERROR(GetDebugLogger(), DBSTR "expected table but got type %s", index, lua_typename(pLuaState, lua_type(pLuaState, index)));
		return Maths::Constants<Maths::HVec3>::Zero();
	}
#endif // LUA_ACTORS_CHECKS

	const int vectorIndex = (index >= 0) ? index : lua_gettop(pLuaState) + index + 1;

	// get the components
	const U32 numComponents = lengthof(LuaActorUtils_Impl::g_VectorComponentNames);
	Maths::HF32 values [numComponents];

	for (U32 i = 0; i < numComponents; ++i)
	{
		lua_pushstring(pLuaState, LuaActorUtils_Impl::g_VectorComponentNames[i]);
		lua_rawget(pLuaState, vectorIndex);

#if LUA_ACTORS_CHECKS
		if (!lua_isnumber(pLuaState, -1))
		{
			LOGERROR(GetDebugLogger(), "vector component %s has type %s", LuaActorUtils_Impl::g_VectorComponentNames[i], lua_typename(pLuaState, lua_type(pLuaState, -1)));
			values[i] = Maths::Constants<Maths::HF32>::Zero();
		}
		else
#endif // LUA_ACTORS_CHECKS
		{
			values[i] = static_cast<Maths::HF32>(lua_tonumber(pLuaState, -1));
		}
	}	
	
	lua_pop(pLuaState, numComponents);

	// set the vector
	StaticAssert(numComponents == 3, maths_hvec3_has_3_components);
	return Maths::HVec3(values[0], values[1], values[2]);
}

void PushHVec3(lua_State* pLuaState, const Maths::HVec3 PCREF vector)
{
	lua_newtable(pLuaState);

	// set the components
	const U32 numComponents = lengthof(LuaActorUtils_Impl::g_VectorComponentNames);
	StaticAssert(numComponents == 3, maths_hvec3_has_3_components);
	const Maths::HF32 values [numComponents] = {vector.X(), vector.Y(), vector.Z()};

	// write the components
	for (U32 i = 0; i < numComponents; ++i)
	{
		lua_pushstring(pLuaState, LuaActorUtils_Impl::g_VectorComponentNames[i]);
		lua_pushnumber(pLuaState, static_cast<lua_Number>(values[i]));
		lua_rawset(pLuaState, -3);
	}
}

Maths::HMat43 ToHMat43(lua_State* pLuaState, const int index)
{
#if LUA_ACTORS_CHECKS
	// check that this is a valid stack index
	if ((index == 0) || (abs(index) > lua_gettop(pLuaState)))
	{
		LOGERROR(GetDebugLogger(), DBSTR "stack index %d out of range %d", index, lua_gettop(pLuaState));
		return Maths::Constants<Maths::HMat43>::Identity();
	}

	// check that it contains a table
	if (!lua_istable(pLuaState, index))
	{
		LOGERROR(GetDebugLogger(), DBSTR "expected table but got type %s", index, lua_typename(pLuaState, lua_type(pLuaState, index)));
		return Maths::Constants<Maths::HMat43>::Identity();
	}
#endif // LUA_ACTORS_CHECKS

	const int matrixIndex = (index >= 0) ? index : lua_gettop(pLuaState) + index + 1;

	// get the components
	const U32 numComponents = lengthof(LuaActorUtils_Impl::g_MatrixComponentNames);
	Maths::HVec3 values [numComponents];

	for (U32 i = 0; i < numComponents; ++i)
	{
		lua_pushstring(pLuaState, LuaActorUtils_Impl::g_MatrixComponentNames[i]);
		lua_rawget(pLuaState, matrixIndex);
		values[i] = ToHVec3(pLuaState, -1);
	}	
	
	lua_pop(pLuaState, numComponents);

	// set the matrix
	StaticAssert(numComponents == 4, maths_hmat43_has_4_components);
	return Maths::HMat43(values[0], values[1], values[2], values[3]);
}

void PushHMat43(lua_State* pLuaState, const Maths::HMat43& matrix)
{
	lua_newtable(pLuaState);

	// set the components
	const U32 numComponents = lengthof(LuaActorUtils_Impl::g_MatrixComponentNames);
	StaticAssert(numComponents == 4, maths_hmat43_has_4_components);
	const Maths::HVec3 values [numComponents] = {matrix.AX(), matrix.AY(), matrix.AW(), matrix.AZ()};

	// write the components
	for (U32 i = 0; i < numComponents; ++i)
	{
		lua_pushstring(pLuaState, LuaActorUtils_Impl::g_MatrixComponentNames[i]);
		PushHVec3(pLuaState, values[i]);
		lua_rawset(pLuaState, -3);
	}
}

} // namespace LuaActor
