#include "LuaActorsPch.h"

#include <lua_actors/LuaActor.h>

#include <actor_system/ActorSerialisation.h>
#include <actor_system/GameComponent.h>
#include <lapi.h>
#include <lauxlib.h>
#include <lua_actors/LuaActorMessages.h>
#include <lua_actors/LuaActorScript.h>
#include <lua_actors/LuaActorUtils.h>
#include <misc_shared/dataref/Sorcery.h>
#include <misc_shared/messaging/MessageSerialisation.h>


namespace LuaActor_Impl
{

const char* g_InstanceTableName = "g_LuaActorInstances";
const char* g_MetaTableName = "s_LuaActorMetaTable";
const char* g_LuaActorName = "m_pLuaActor";
const char* g_DataTableName = "m_Data";

const luaL_Reg g_MetaTableRegistry [] = 
{
	{ "AddSubscriber",			LuaActor::C_LuaActor::LuaAddSubscriber }, 
	{ "BroadcastMessage",		LuaActor::C_LuaActor::LuaBroadcastMessage }, 
	{ "GetActorHandle",			LuaActor::C_LuaActor::LuaGetActorHandle }, 
	{ "GetActorName",			LuaActor::C_LuaActor::LuaGetActorName }, 
	{ "GetCreationData",		LuaActor::C_LuaActor::LuaGetCreationData }, 
	{ "InitialiseComponent",	LuaActor::C_LuaActor::LuaInitialiseComponent }, 
	{ "RemoveSubscriber",		LuaActor::C_LuaActor::LuaRemoveSubscriber }, 
	{ "SendMessageToActor", 	LuaActor::C_LuaActor::LuaSendMessageToActor }, 
	{ NULL,						NULL }
};

} // namespace LuaActor_Impl


namespace LuaActor
{

C_LuaActor::C_LuaActor() :
	Actor::C_Actor(), 
	m_pLuaActorScript(NULL)
{
}

C_LuaActor::~C_LuaActor()
{
	if (m_pLuaActorScript != NULL)
	{
		m_pLuaActorScript->DoLifetimeHook(this, E_LifetimeHook_Destruct);
		m_pLuaActorScript = NULL;
	}
}

lua_State* C_LuaActor::GetLuaState() const
{
	// TODO: [Oscar] rip value from actor context
	return NULL;
}

const C_LuaActorMessageFactory* C_LuaActor::GetLuaActorMessageFactory() const
{
	// TODO: [Oscar] rip value from actor context
	return NULL;
}

bool C_LuaActor::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	if (m_pLuaActorScript->DoMessageHandler(this, pMessage))
		return true;

	return Actor::C_Actor::MessageHandler(pMessage);
}

bool C_LuaActor::SerialiseLua(Sys::C_ContextualStream& stream, lua_State* pLuaState, const int index)
{
#if LUA_ACTORS_CHECKS
	if (abs(index) > lua_gettop(pLuaState))
	{
		LOGERROR(GetDebugLogger(), DBSTR "stack index %d out of range %d", index, lua_gettop(pLuaState));
		return false;
	}
#endif // LUA_ACTORS_CHECKS

	const int luaType = lua_type(pLuaState, index);

	// write contents
	switch (luaType)
	{
	case LUA_TNUMBER:
		{
			const lua_Number value = lua_tonumber(pLuaState, index);
			stream << value;
		}
		break;

	case LUA_TBOOLEAN:
		{
			const bool value = (lua_toboolean(pLuaState, index) != 0);
			stream << value;
		}
		break;

	case LUA_TSTRING:
		{
			const Btl::String value = lua_tostring(pLuaState, index);
			stream << value;
		}
		break;

	case LUA_TTABLE:
		if (!SerialiseTable(stream, pLuaState, index))
		{
			return false;
		}
		break;

	case LUA_TLIGHTUSERDATA:
		{
			// assume this must be an actor handle... we shouldn't be pushing any other userdata that scripts can use
			const Actor::C_ActorHandle value = ToActorHandle(pLuaState, index);
			stream << value;
		}
		break;

	default:
#if LUA_ACTORS_CHECKS
		LOGERROR(GetDebugLogger(), DBSTR "don't know how to to serialise type %s", lua_typename(pLuaState, luaType));
#endif // LUA_ACTORS_CHECKS
		return false;
	}

	// done
	return true;
}

bool C_LuaActor::DeserialiseLua(Sys::C_ContextualStream& stream, lua_State* pLuaState, const int luaType)
{
	// read contents
	switch (luaType)
	{
	case LUA_TNUMBER:
		{
			lua_Number value;
			stream >> value;
			lua_pushnumber(pLuaState, value);
		}
		break;

	case LUA_TBOOLEAN:
		{
			bool value;
			stream >> value;
			lua_pushboolean(pLuaState, value ? 1 : 0);
		}
		break;

	case LUA_TSTRING:
		{
			Btl::String value;
			stream >> value;
			lua_pushlstring(pLuaState, value.Chars(), value.Length());
		}
		break;

	case LUA_TTABLE:
		{
			lua_newtable(pLuaState);
			if (!DeserialiseTable(stream, pLuaState, -1))
			{
				return false;
			}
		}
		break;

	case LUA_TLIGHTUSERDATA:
		{
			// assume this must be an actor handle... we shouldn't be pushing any other userdata that scripts can use
			Actor::C_ActorHandle value;
			stream >> value;
			PushActorHandle(pLuaState, value);
		}
		break;

	default:
#if LUA_ACTORS_CHECKS
		LOGERROR(GetDebugLogger(), DBSTR "don't know how to to deserialise type %s", lua_typename(pLuaState, luaType));
#endif // LUA_ACTORS_CHECKS
		return false;
	}

	// done
	return true;
}

bool C_LuaActor::SerialiseTable(Sys::C_ContextualStream& stream, lua_State* pLuaState, const int index)
{
	const int tableIndex = (index > 0) ? index : lua_gettop(pLuaState) + index + 1;

	// write table members
	for (lua_pushnil(pLuaState); lua_next(pLuaState, tableIndex) != 0; )
	{
		// write type
		const int luaType = lua_type(pLuaState, -1);
		stream << luaType;

		// write name
#if LUA_ACTORS_CHECKS
		if (!lua_isstring(pLuaState, -2))
		{
			LOGERROR(GetDebugLogger(), DBSTR "data table key should be string, got %s", lua_typename(pLuaState, lua_type(pLuaState, -2)));
			lua_pop(pLuaState, 2);
			return false;
		}
#endif // LUA_ACTORS_CHECKS
		
		const char* memberName = lua_tostring(pLuaState, -2);
		stream << Btl::String(memberName);

		// write data
		if (!SerialiseLua(stream, pLuaState, -1))
		{
#if LUA_ACTORS_CHECKS
			LOGERROR(GetDebugLogger(), DBSTR "failed to write table member %s", memberName);
#endif // LUA_ACTORS_CHECKS
			lua_pop(pLuaState, 2);
			return false;
		}

		// next
		lua_pop(pLuaState, 1);
	}

	// terminate with special type
	stream.WriteAs<int>(LUA_TNONE);
	return true;
}

bool C_LuaActor::DeserialiseTable(Sys::C_ContextualStream& stream, lua_State* pLuaState, const int index)
{
	const int tableIndex = (index > 0) ? index : lua_gettop(pLuaState) + index + 1;

	// read table members
	for (;;)
	{
		// read type
		int luaType;
		stream >> luaType;

		// look for terminator
		if (luaType == LUA_TNONE)
		{
			break;
		}

		// read name
		Btl::String memberName;
		stream >> memberName;
		lua_pushlstring(pLuaState, memberName.Chars(), memberName.Length());

		// read value
		if (!DeserialiseLua(stream, pLuaState, luaType))
		{
#if LUA_ACTORS_CHECKS
			LOGERROR(GetDebugLogger(), DBSTR "failed to deserialise table element %s", memberName.Chars());
#endif // LUA_ACTORS_CHECKS
			lua_pop(pLuaState, 1);
			return false;
		}

		// add to table
		lua_rawset(pLuaState, tableIndex);
	}

	// done 
	return true;
}

void C_LuaActor::Serialise(Sys::C_ContextualStream& stream) const
{
	lua_State* pLuaState = GetLuaState();

	// put the instance table onto the stack
	lua_pushstring(pLuaState, LuaActor_Impl::g_InstanceTableName);
	lua_rawget(pLuaState, LUA_GLOBALSINDEX);

	// get our instance
	const char* actorName = GetActorName();
	lua_pushstring(pLuaState, actorName);
	lua_rawget(pLuaState, -2);

	// get the data table
	lua_pushstring(pLuaState, LuaActor_Impl::g_DataTableName);
	lua_rawget(pLuaState, -2);

	// serialise the data table
	if (!SerialiseTable(stream, pLuaState, -1))
	{
#if LUA_ACTORS_CHECKS
		LOGERROR(GetDebugLogger(), DBSTR "failed to write data table for actor %s", GetActorName());
#endif // LUA_ACTORS_CHECKS
	}

	// remove stuff from stack
	lua_pop(pLuaState, 3);
}

void C_LuaActor::Deserialise(Sys::C_ContextualStream& stream)
{
	lua_State* pLuaState = GetLuaState();

	// put the instance table onto the stack
	lua_pushstring(pLuaState, LuaActor_Impl::g_InstanceTableName);
	lua_rawget(pLuaState, LUA_GLOBALSINDEX);

	// get our instance
	const char* actorName = GetActorName();
	lua_pushstring(pLuaState, actorName);
	lua_rawget(pLuaState, -2);

	// get the data table
	lua_pushstring(pLuaState, LuaActor_Impl::g_DataTableName);
	lua_rawget(pLuaState, -2);

	//de serialise the data table
	if (!DeserialiseTable(stream, pLuaState, -1))
	{
#if LUA_ACTORS_CHECKS
		LOGERROR(GetDebugLogger(), DBSTR "failed to read data table for actor %s", GetActorName());
#endif // LUA_ACTORS_CHECKS
	}

	// remove stuff from stack
	lua_pop(pLuaState, 3);
}

void C_LuaActor::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	Actor::C_Actor::SerialiseForSave(stream);
	Serialise(stream);
}

void C_LuaActor::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	Actor::C_Actor::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_LuaActor::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	Actor::C_Actor::SerialiseForSync(stream);
	Serialise(stream);
}

void C_LuaActor::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	Actor::C_Actor::DeserialiseForSync(stream);
	Deserialise(stream);
}

bool C_LuaActor::InitialiseImpl()
{
	if (!Actor::C_Actor::InitialiseImpl())
		return false;

	// TODO: [Oscar] set m_pLuaActorScript

	if (!m_pLuaActorScript->DoLifetimeHook(this, E_LifetimeHook_Initialise))
		return false;

	return true;
}

bool C_LuaActor::PrimeImpl()
{
	bool primed = Actor::C_Actor::PrimeImpl();

	primed &= m_pLuaActorScript->DoLifetimeHook(this, E_LifetimeHook_Prime);

	return primed;
}

bool C_LuaActor::CreateImpl()
{
	if (!Actor::C_Actor::CreateImpl())
		return false;

	if (!m_pLuaActorScript->DoLifetimeHook(this, E_LifetimeHook_Create))
		return false;

	return true;
}

void C_LuaActor::KillImpl()
{
	m_pLuaActorScript->DoLifetimeHook(this, E_LifetimeHook_Kill);

	Actor::C_Actor::KillImpl();
}

void C_LuaActor::PushInstance(lua_State* pLuaState)
{
	// get global instance table
	lua_pushstring(pLuaState, LuaActor_Impl::g_InstanceTableName);
	lua_rawget(pLuaState, LUA_GLOBALSINDEX);

	// get actor instance
	const char* actorName = GetActorName();
	lua_pushstring(pLuaState, GetActorName());
	lua_rawget(pLuaState, -2);

#if LUA_ACTORS_CHECKS
	if (!lua_istable(pLuaState, -1))
	{
		LOGERROR(GetDebugLogger(), DBSTR "instance table for actor %s has type %s", actorName, lua_typename(pLuaState, lua_type(pLuaState, -1)));
		lua_pop(pLuaState, 1);
		lua_pushnil(pLuaState);
	}
#endif // LUA_ACTORS_CHECKS

	// remove global instance table
	lua_remove(pLuaState, -2);
}

bool C_LuaActor::AcquireInstance(lua_State* pLuaState)
{
	// put the instance table onto the stack
	lua_pushstring(pLuaState, LuaActor_Impl::g_InstanceTableName);
	lua_rawget(pLuaState, LUA_GLOBALSINDEX);

	if (lua_isnil(pLuaState, -1))
	{
		lua_pop(pLuaState, 1);

		// create the instance table
		lua_newtable(pLuaState);

		lua_pushstring(pLuaState, LuaActor_Impl::g_InstanceTableName);
		lua_pushvalue(pLuaState, -2);
		lua_rawset(pLuaState, LUA_GLOBALSINDEX);

		// create the meta-table
		lua_newtable(pLuaState);

		for (const luaL_Reg* pDef = LuaActor_Impl::g_MetaTableRegistry; pDef->name != NULL; ++pDef)
		{
			lua_pushstring(pLuaState, pDef->name);
			lua_pushcfunction(pLuaState, pDef->func);
			lua_rawset(pLuaState, -3);
		}

		lua_setmetatable(pLuaState, -2);
	}
#if LUA_ACTORS_CHECKS
	else if (!lua_istable(pLuaState, -1))
	{
		LOGERROR(GetDebugLogger(), DBSTR "global name %s already used as type %s", LuaActor_Impl::g_InstanceTableName, lua_typename(pLuaState, lua_type(pLuaState, -1)));
		lua_pop(pLuaState, 1);
		return false;
	}
#endif // LUA_ACTORS_CHECKS

	// create the instance
	const char* actorName = GetActorName();

#if LUA_ACTORS_CHECKS
	lua_pushstring(pLuaState, actorName);
	lua_rawget(pLuaState, -2);

	if (!lua_isnil(pLuaState, -1))
	{
		LOGERROR(GetDebugLogger(), DBSTR "instance table already contains an actor with name %s", actorName);
		lua_pop(pLuaState, 2);
		return false;
	}

	lua_pop(pLuaState, 1);
#endif // LUA_ACTORS_CHECKS

	lua_pushstring(pLuaState, actorName);
	lua_newtable(pLuaState);
	
	// copy the meta-table from the global table to this instance
	lua_getmetatable(pLuaState, -3);
	lua_setmetatable(pLuaState, -2);

	// store the actor
	lua_pushstring(pLuaState, LuaActor_Impl::g_LuaActorName);
	lua_pushlightuserdata(pLuaState, this);
	lua_rawset(pLuaState, -3);

	// create the data table
	lua_pushstring(pLuaState, LuaActor_Impl::g_DataTableName);
	lua_newtable(pLuaState);
	lua_rawset(pLuaState, -3);

	// add to instance table
	lua_rawset(pLuaState, -3);
	lua_pop(pLuaState, 1);

	return true;
}

bool C_LuaActor::ReleaseInstance(lua_State* pLuaState)
{
	// put the instance table onto the stack
	lua_pushstring(pLuaState, LuaActor_Impl::g_InstanceTableName);
	lua_rawget(pLuaState, LUA_GLOBALSINDEX);

#if LUA_ACTORS_CHECKS
	if (!lua_istable(pLuaState, -1))
	{
		LOGERROR(GetDebugLogger(), DBSTR "global %s has type %s", LuaActor_Impl::g_InstanceTableName, lua_typename(pLuaState, lua_type(pLuaState, -1)));
		lua_pop(pLuaState, 1);
		return false;
	}
#endif // LUA_ACTORS_CHECKS

	// remove the instance
	const char* actorName = GetActorName();
	lua_pushstring(pLuaState, actorName);
	lua_pushnil(pLuaState);
	lua_rawset(pLuaState, -3);

	// check if the instance table is empty
	lua_pushnil(pLuaState);
	if (lua_next(pLuaState, -2) == 0)
	{
		// remove the global
		lua_pushstring(pLuaState, LuaActor_Impl::g_InstanceTableName);
		lua_pushnil(pLuaState);
		lua_rawset(pLuaState, LUA_GLOBALSINDEX);
	}
	else
	{
		lua_pop(pLuaState, 1);
	}

	// remove instance table from stack
	lua_pop(pLuaState, 1);
	
	return true;
}

C_LuaActor* C_LuaActor::GetFromInstance(lua_State* pLuaState, const int index)
{
#if LUA_ACTORS_CHECKS
	// check that this is a valid stack index
	if ((index == 0) || (abs(index) > lua_gettop(pLuaState)))
	{
		LOGERROR(GetDebugLogger(), DBSTR "stack index %d out of range %d", index, lua_gettop(pLuaState));
		return NULL;
	}

	// check that it's a table
	if (!lua_istable(pLuaState, index))
	{
		LOGERROR(GetDebugLogger(), DBSTR "expected table, got %s", lua_typename(pLuaState, lua_type(pLuaState, index)));
		return NULL;
	}
#endif // LUA_ACTORS_CHECKS

	const int instanceIndex = (index > 0) ? index : lua_gettop(pLuaState) + index + 1;

	// get the C_LuaActor pointer from this table
	lua_pushstring(pLuaState, LuaActor_Impl::g_LuaActorName);
	lua_rawget(pLuaState, instanceIndex);

#if LUA_ACTORS_CHECKS
	// check that it contains userdata
	if (!lua_isuserdata(pLuaState, -1))
	{
		LOGERROR(GetDebugLogger(), DBSTR "%s has type %s", LuaActor_Impl::g_LuaActorName, lua_typename(pLuaState, lua_type(pLuaState, -1)));
		lua_pop(pLuaState, 1);
		return NULL;
	}
#endif

	C_LuaActor* pLuaActor = reinterpret_cast<C_LuaActor*>(lua_touserdata(pLuaState, -1));
	lua_pop(pLuaState, 1);
	return pLuaActor;
}

int C_LuaActor::LuaAddSubscriber(lua_State* pLuaState)
{
	C_LuaActor* pLuaActor = GetFromInstance(pLuaState, 1);

#if LUA_ACTORS_CHECKS
	if (lua_gettop(pLuaState) < 2)
	{
		LOGERROR(GetDebugLogger(), DBSTR "no message specified");
		return 0;
	}

	if (!lua_isstring(pLuaState, 2))
	{
		LOGERROR(GetDebugLogger(), DBSTR "message type must be string, got %s", lua_typename(pLuaState, lua_type(pLuaState, 2)));
		return 0;
	}
#endif // LUA_ACTORS_CHECKS

	const char* messageName = lua_tostring(pLuaState, 2);
	const Messaging::TMsgType messageType = pLuaActor->GetLuaActorMessageFactory()->GetMessageType(messageName);

#if LUA_ACTORS_CHECKS
	if (static_cast<int>(messageType) == -1)
	{
		LOGERROR(GetDebugLogger(), DBSTR "actor %s subscribed to unknown message %s", pLuaActor->GetActorName(), messageName);
	}
	else
#endif // LUA_ACTORS_CHECKS
	{
		pLuaActor->AddSubscriber(messageType);
	}

	return 0;
}

int C_LuaActor::LuaBroadcastMessage(lua_State* pLuaState)
{
	C_LuaActor* pLuaActor = GetFromInstance(pLuaState, 1);
	
	Messaging::C_DefaultMessageAllocator allocator;
	Messaging::C_MessageBase* pMessage = pLuaActor->GetLuaActorMessageFactory()->ToMessage(pLuaState, 3, allocator);

	if (pMessage != NULL)
	{
		pLuaActor->BroadcastMessage(pMessage);
		allocator.Destroy(pMessage);
	}

	return 0;
}

int C_LuaActor::LuaGetActorHandle(lua_State* pLuaState)
{
	C_LuaActor* pLuaActor = GetFromInstance(pLuaState, 1);

	Actor::C_ActorHandle actorHandle;

	if (pLuaActor != NULL)
		actorHandle = pLuaActor->GetHandle();

	PushActorHandle(pLuaState, actorHandle);
	return 1;
}

int C_LuaActor::LuaGetActorName(lua_State* pLuaState)
{
	C_LuaActor* pLuaActor = GetFromInstance(pLuaState, 1);

	lua_pushstring(pLuaState, pLuaActor->GetActorName());
	return 1;
}

int C_LuaActor::LuaGetCreationData(lua_State* pLuaState)
{
	C_LuaActor* pLuaActor = GetFromInstance(pLuaState, 1);

#if LUA_ACTORS_CHECKS
	if (lua_gettop(pLuaState) < 2)
	{
		LOGERROR(GetDebugLogger(), DBSTR "no creation data name specified");
		lua_pushnil(pLuaState);
		return 1;
	}

	if (!lua_isstring(pLuaState, 2))
	{
		LOGERROR(GetDebugLogger(), DBSTR "creation data name must be string, got %s", lua_typename(pLuaState, lua_type(pLuaState, 2)));
		lua_pushnil(pLuaState);
		return 1;
	}
#endif // LUA_ACTORS_CHECKS

	const char* memberName = lua_tostring(pLuaState, 2);

	pLuaActor->GetCreationData()->PushNamedData(pLuaState, pLuaActor, memberName);
	return 1;
}

int C_LuaActor::LuaInitialiseComponent(lua_State* pLuaState)
{
	C_LuaActor* pLuaActor = GetFromInstance(pLuaState, 1);

#if LUA_ACTORS_CHECKS
	if (lua_gettop(pLuaState) < 2)
	{
		LOGERROR(GetDebugLogger(), DBSTR "no component creation data specified");
		lua_pushboolean(pLuaState, FALSE);
		return 1;
	}

	if (!lua_isuserdata(pLuaState, 2))
	{
		LOGERROR(GetDebugLogger(), DBSTR "component creation data must be userdata, got %s", lua_typename(pLuaState, lua_type(pLuaState, 2)));
		lua_pushboolean(pLuaState, FALSE);
		return 1;
	}
#endif // LUA_ACTORS_CHECKS

	const Component::I_ComponentCreator* pCreationData = reinterpret_cast<const Component::I_ComponentCreator*>(lua_touserdata(pLuaState, 2));
	const DataRef::C_Ptr<Component::I_ComponentCreator> dataRef = DataRef::SorceryLookup(pCreationData);

#if LUA_ACTORS_CHECKS
	if (dataRef.IsNull())
	{
		LOGERROR(GetDebugLogger(), DBSTR "component create data for actor %s is not in Xt database", pLuaActor->GetActorName());
		lua_pushboolean(pLuaState, FALSE);
		return 1;
	}
#endif // LUA_ACTORS_CHECKS

	const Actor::I_ActorComponent* pComponent = pLuaActor->InitialiseComponent(dataRef);
	lua_pushboolean(pLuaState, (pComponent == NULL) ? FALSE : TRUE);
	return 1;
}

int C_LuaActor::LuaRemoveSubscriber(lua_State* pLuaState)
{
	C_LuaActor* pLuaActor = GetFromInstance(pLuaState, 1);

#if LUA_ACTORS_CHECKS
	if (lua_gettop(pLuaState) < 2)
	{
		LOGERROR(GetDebugLogger(), DBSTR "no message specified");
		return 0;
	}

	if (!lua_isstring(pLuaState, 2))
	{
		LOGERROR(GetDebugLogger(), DBSTR "message type must be string, got %s", lua_typename(pLuaState, lua_type(pLuaState, 2)));
		return 0;
	}
#endif // LUA_ACTORS_CHECKS

	const char* messageName = lua_tostring(pLuaState, 2);
	const Messaging::TMsgType messageType = pLuaActor->GetLuaActorMessageFactory()->GetMessageType(messageName);

#if LUA_ACTORS_CHECKS
	if (static_cast<int>(messageType) == -1)
	{
		LOGERROR(GetDebugLogger(), DBSTR "actor %s unsubscribed from unknown message %s", pLuaActor->GetActorName(), messageName);
	}
	else
#endif // LUA_ACTORS_CHECKS
	{
		pLuaActor->RemoveSubscriber(messageType);
	}

	return 0;
}

int C_LuaActor::LuaSendMessageToActor(lua_State* pLuaState)
{
	C_LuaActor* pLuaActor = GetFromInstance(pLuaState, 1);
	
	const Actor::C_ActorHandle destination = ToActorHandle(pLuaState, 2);

	Messaging::C_DefaultMessageAllocator allocator;
	Messaging::C_MessageBase* pMessage = pLuaActor->GetLuaActorMessageFactory()->ToMessage(pLuaState, 3, allocator);

	if (pMessage != NULL)
	{
		pLuaActor->SendMessageToActor(destination, pMessage);
		allocator.Destroy(pMessage);
	}

	return 0;
}

} // namespace LuaActor
