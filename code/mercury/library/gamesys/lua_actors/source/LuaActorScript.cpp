#include "LuaActorsPch.h"

#include <lua_actors/LuaActorScript.h>

#include <lapi.h>
#include <lua_actors/LuaActor.h>
#include <lua_actors/LuaActorContext.h>
#include <lua_actors/LuaActorMessages.h>
#include <lua_actors/LuaActorUtils.h>


namespace LuaActorScript_Impl
{

const char* g_pLifetimeHookNames [] = 
{
	"Initialise", 
	"Prime", 
	"Create", 
	"Kill", 
	"Destruct", 
};

} // namespace LuaActorScript_Impl


namespace LuaActor
{

const char* GetLifetimeHookName(const E_LifetimeHook lifetimeHook)
{
	StaticAssert(lengthof(LuaActorScript_Impl::g_pLifetimeHookNames) == E_LifetimeHook_Count, array_size_must_match_enum_count);
	return ((lifetimeHook >= 0) && (lifetimeHook < E_LifetimeHook_Count)) ? LuaActorScript_Impl::g_pLifetimeHookNames[lifetimeHook] : "<unknown>";
}


C_LuaActorScript::C_LuaActorScript() :
	m_pLuaActorContext(NULL), 
	m_ScriptName(), 
	m_LifetimeHooks(0), 
	m_Messages()
{
}

C_LuaActorScript::~C_LuaActorScript()
{
}

bool C_LuaActorScript::PushScriptTable(lua_State* pLuaState) const
{
	lua_pushlstring(pLuaState, m_ScriptName.Chars(), m_ScriptName.Length());
	lua_rawget(pLuaState, LUA_GLOBALSINDEX);

#if LUA_ACTORS_CHECKS
	if (!lua_istable(pLuaState, -1))
	{
		LOGERROR(GetDebugLogger(), DBSTR "script '%s' references global of type %s", m_ScriptName.Chars(), lua_typename(pLuaState, lua_type(pLuaState, -1)));
		lua_pop(pLuaState, 1);
		return false;
	}
#endif // LUA_ACTORS_CHECKS

	return true;
}

bool C_LuaActorScript::Init(const I_LuaActorContext* pLuaActorContext, const Btl::String& scriptName)
{
	assert(pLuaActorContext != NULL);

	Clear();

	m_pLuaActorContext = pLuaActorContext;
	m_ScriptName = scriptName;

	// attempt to find the named script in the globals
	lua_State* pLuaState = pLuaActorContext->GetLuaState();

	if (!PushScriptTable(pLuaState))
	{
		return false;
	}

	const C_LuaActorMessageFactory& messageFactory = pLuaActorContext->GetLuaActorMessageFactory();

	// enumerate the script functions
	for (lua_pushnil(pLuaState); lua_next(pLuaState, -2) != 0; )
	{
#if LUA_ACTORS_CHECKS
		if (!lua_isstring(pLuaState, -2))
		{
			LOGERROR(GetDebugLogger(), DBSTR "script '%s' contains key of type %s, should be a string", scriptName.Chars(), lua_typename(pLuaState, lua_type(pLuaState, -2)));
			lua_pop(pLuaState, 1);
			continue;
		}
#endif // LUA_ACTORS_CHECKS

		// get the function name
		const char* functionName = lua_tostring(pLuaState, -2);

#if LUA_ACTORS_CHECKS
		if (!lua_isfunction(pLuaState, -1))
		{
			LOGERROR(GetDebugLogger(), DBSTR "script '%s' contains key %s with value of type %s, should be a function", scriptName.Chars(), functionName, lua_typename(pLuaState, lua_type(pLuaState, -1)));
			lua_pop(pLuaState, 1);
			continue;
		}
#endif // LUA_ACTORS_CHECKS

		// check if this is a message handler or a lifetime hook, otherwise ignore it
		const Messaging::TMsgType messageType = messageFactory.GetMessageType(functionName);

		if (messageType != -1)
		{
			m_Messages.Insert(messageType);
		}
		else
		{
			for (U32 lifetimeHook = 0; lifetimeHook < E_LifetimeHook_Count; ++lifetimeHook)
			{
				if (stricmp(functionName, LuaActorScript_Impl::g_pLifetimeHookNames[lifetimeHook]) == 0)
				{
					m_LifetimeHooks |= (1 << lifetimeHook);
					break;
				}
			}
		}

		// next function
		lua_pop(pLuaState, 1);
	}

	// done
	lua_pop(pLuaState, 1);
	return true;
}

void C_LuaActorScript::Clear()
{
	m_pLuaActorContext = NULL;
	m_ScriptName = Btl::String();
	m_LifetimeHooks = 0;
	m_Messages.Clear();
}

bool C_LuaActorScript::DoLifetimeHook(C_LuaActor* pLuaActor, const E_LifetimeHook lifetimeHook) const
{
	assert(pLuaActor != NULL);

	bool ok = true;

	if ((m_LifetimeHooks & (1 << lifetimeHook)) != 0)
	{
		// get the script
		lua_State* pLuaState = m_pLuaActorContext->GetLuaState();
		PushScriptTable(pLuaState);

		// push the function
		const char* functionName = GetLifetimeHookName(lifetimeHook);
		lua_pushstring(pLuaState, functionName);
		lua_rawget(pLuaState, -2);

		// push the parameters
		pLuaActor->PushInstance(pLuaState);

		// call the function
		const int numParams = 1;
		const int numReturn = 1;
#if LUA_ACTORS_CHECKS
		if (lua_pcall(pLuaState, numParams, numReturn, 0) != 0)
		{
			LOGERROR(GetDebugLogger(), DBSTR "script '%s' encounted error in %s : %s", m_ScriptName.Chars(), functionName, lua_tostring(pLuaState, -1));
			ok = false;
		}
		else
#else
		lua_call(pLuaState, numParams, numReturn);
#endif LUA_ACTORS_CHECKS
		{
			// handle the return value
			ok = (lua_toboolean(pLuaState, -1) != 0);
		}

		// pop the return values and remove the script
		lua_pop(pLuaState, 1 + numReturn);
	}

	return ok;
}

bool C_LuaActorScript::DoMessageHandler(C_LuaActor* pLuaActor, const Messaging::C_MessageBase* pMessage) const
{
	assert(pLuaActor != NULL);

	bool handled = false;

	if (m_Messages.Find(pMessage->Type()) != m_Messages.End())
	{
		const C_LuaActorMessageFactory& messageFactory = m_pLuaActorContext->GetLuaActorMessageFactory();

		// get the script
		lua_State* pLuaState = m_pLuaActorContext->GetLuaState();
		PushScriptTable(pLuaState);

		// push the function
		const char* functionName = messageFactory.GetMessageName(pMessage->Type());
		lua_pushstring(pLuaState, functionName);
		lua_rawget(pLuaState, -2);

		// push the parameters
		pLuaActor->PushInstance(pLuaState);
		messageFactory.PushMessage(pLuaState, pMessage);

		// call the function
		const int numParams = 2;
		const int numReturn = 0;
#if LUA_ACTORS_CHECKS
		if (lua_pcall(pLuaState, numParams, numReturn, 0) != 0)
		{
			LOGERROR(GetDebugLogger(), DBSTR "script '%s' encounted error in %s : %s", m_ScriptName.Chars(), functionName, lua_tostring(pLuaState, -1));
		}
		else
#else
		lua_call(pLuaState, numParams, numReturn);
#endif LUA_ACTORS_CHECKS
		{
			handled = true;
		}

		// pop the return values and remove the script
		lua_pop(pLuaState, 1 + numReturn);
	}

	return handled;
}

} // namespace LuaActor
