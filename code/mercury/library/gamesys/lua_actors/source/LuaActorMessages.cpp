#include "LuaActorsPch.h"

#include <lua_actors/LuaActorMessages.h>

#include <lapi.h>


namespace LuaActorMessages_Impl
{

const char* g_MessageTypeName = "MessageType";

// boolean type info
class C_LuaActorMessageBooleanInfo : public LuaActor::I_LuaActorMessageDataTypeInfo
{
public:
	virtual U32 GetSize() const {return sizeof(bool);}

	virtual void PushValue(lua_State* pLuaState, const void* pMessageData) const
	{
		const bool* pValue = reinterpret_cast<const bool*>(pMessageData);
		lua_pushboolean(pLuaState, *pValue ? 1 : 0);
	}

	virtual void ToValue(lua_State* pLuaState, const int index, void* pMessageData) const
	{
		bool* pValue = reinterpret_cast<bool*>(pMessageData);

#if LUA_ACTORS_CHECKS
		if (abs(index) > lua_gettop(pLuaState))
		{
			LOGERROR(LuaActor::GetDebugLogger(), DBSTR "stack index %d out of range %d", index, lua_gettop(pLuaState));
			*pValue = false;
			return;
		}

		if (!lua_isboolean(pLuaState, index))
		{
			LOGERROR(LuaActor::GetDebugLogger(), DBSTR "expected boolean but got %s", lua_typename(pLuaState, lua_type(pLuaState, index)));
			*pValue = false;
			return;
		}
#endif // LUA_ACTORS_CHECKS

		*pValue = (lua_toboolean(pLuaState, index) != 0);
	}
};

const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const bool*)
{
	static C_LuaActorMessageBooleanInfo typeInfo;
	return &typeInfo;
}

// numeric type info
class C_LuaActorMessageNumberInfoBase : public LuaActor::I_LuaActorMessageDataTypeInfo
{
protected:
	void PushNumber(lua_State* pLuaState, const lua_Number value) const
	{
		lua_pushnumber(pLuaState, value);
	}

	lua_Number ToNumber(lua_State* pLuaState, const int index) const
	{
#if LUA_ACTORS_CHECKS
		if (abs(index) > lua_gettop(pLuaState))
		{
			LOGERROR(LuaActor::GetDebugLogger(), DBSTR "stack index %d out of range %d", index, lua_gettop(pLuaState));
			return 0;
		}

		if (!lua_isnumber(pLuaState, index))
		{
			LOGERROR(LuaActor::GetDebugLogger(), DBSTR "expected number but got %s", lua_typename(pLuaState, lua_type(pLuaState, index)));
			return 0;
		}
#endif // LUA_ACTORS_CHECKS

		return lua_tonumber(pLuaState, index);
	}

};

template <typename T_NumericType>
class C_LuaActorMessageNumberInfo : public C_LuaActorMessageNumberInfoBase
{
	virtual U32 GetSize() const {return sizeof(T_NumericType);}

	virtual void PushValue(lua_State* pLuaState, const void* pMessageData) const
	{
		const T_NumericType* pValue = reinterpret_cast<const T_NumericType*>(pMessageData);
		PushNumber(pLuaState, static_cast<lua_Number>(*pValue));
	}

	virtual void ToValue(lua_State* pLuaState, const int index, void* pMessageData) const
	{
		T_NumericType* pValue = reinterpret_cast<T_NumericType*>(pMessageData);
		*pValue = static_cast<T_NumericType>(ToNumber(pLuaState, index));
	}
};

const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const U8*)
{
	static C_LuaActorMessageNumberInfo<U8> typeInfo;
	return &typeInfo;
}

const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const U16*)
{
	static C_LuaActorMessageNumberInfo<U16> typeInfo;
	return &typeInfo;
}

const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const U32*)
{
	static C_LuaActorMessageNumberInfo<U32> typeInfo;
	return &typeInfo;
}

const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const S8*)
{
	static C_LuaActorMessageNumberInfo<S8> typeInfo;
	return &typeInfo;
}

const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const S16*)
{
	static C_LuaActorMessageNumberInfo<S16> typeInfo;
	return &typeInfo;
}

const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const S32*)
{
	static C_LuaActorMessageNumberInfo<S32> typeInfo;
	return &typeInfo;
}

const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const F32*)
{
	static C_LuaActorMessageNumberInfo<F32> typeInfo;
	return &typeInfo;
}

// string type info
void C_LuaActorMessageStringInfo::PushValue(lua_State* pLuaState, const void* pMessageData) const
{
	const char* pValue = reinterpret_cast<const char*>(pMessageData);
	lua_pushstring(pLuaState, pValue);
}

void C_LuaActorMessageStringInfo::ToValue(lua_State* pLuaState, const int index, void* pMessageData) const
{
	char* pValue = reinterpret_cast<char*>(pMessageData);

#if LUA_ACTORS_CHECKS
	if (abs(index) > lua_gettop(pLuaState))
	{
		LOGERROR(LuaActor::GetDebugLogger(), DBSTR "stack index %d out of range %d", index, lua_gettop(pLuaState));
		pValue[0] = 0;
		return;
	}

	if (!lua_isstring(pLuaState, index))
	{
		LOGERROR(LuaActor::GetDebugLogger(), DBSTR "expected string but got %s", lua_typename(pLuaState, lua_type(pLuaState, index)));
		pValue[0] = 0;
		return;
	}

	if (lua_strlen(pLuaState, index) >= m_BufferSize)
	{
		LOGWARN(LuaActor::GetDebugLogger(), DBSTR "string '%s' truncated for buffer of size %d", lua_tostring(pLuaState, index), m_BufferSize);
	}
#endif // LUA_ACTORS_CHECKS

	strncpy(pValue, lua_tostring(pLuaState, index), Maths::Min<size_t>(m_BufferSize - 1, lua_strlen(pLuaState, index)));
}

// vector type info
class C_LuaActorMessageVectorInfo : public LuaActor::I_LuaActorMessageDataTypeInfo
{
public:
	virtual U32 GetSize() const {return sizeof(Maths::HVec3);}

	virtual void PushValue(lua_State* pLuaState, const void* pMessageData) const
	{
		const Maths::HVec3* pValue = reinterpret_cast<const Maths::HVec3*>(pMessageData);
		LuaActor::PushHVec3(pLuaState, *pValue);
	}

	virtual void ToValue(lua_State* pLuaState, const int index, void* pMessageData) const
	{
		Maths::HVec3* pValue = reinterpret_cast<Maths::HVec3*>(pMessageData);
		*pValue = LuaActor::ToHVec3(pLuaState, index);
	}
};

const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const Maths::HVec3*)
{
	static C_LuaActorMessageVectorInfo typeInfo;
	return &typeInfo;
}

// matrix type info
class C_LuaActorMessageMatrixInfo : public LuaActor::I_LuaActorMessageDataTypeInfo
{
public:
	virtual U32 GetSize() const {return sizeof(Maths::HMat43);}

	virtual void PushValue(lua_State* pLuaState, const void* pMessageData) const
	{
		const Maths::HMat43* pValue = reinterpret_cast<const Maths::HMat43*>(pMessageData);
		LuaActor::PushHMat43(pLuaState, *pValue);
	}

	virtual void ToValue(lua_State* pLuaState, const int index, void* pMessageData) const
	{
		Maths::HMat43* pValue = reinterpret_cast<Maths::HMat43*>(pMessageData);
		*pValue = LuaActor::ToHMat43(pLuaState, index);
	}
};

const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const Maths::HMat43*)
{
	static C_LuaActorMessageMatrixInfo typeInfo;
	return &typeInfo;
}

// actor handle type info
class C_LuaActorMessageActorHandleInfo : public LuaActor::I_LuaActorMessageDataTypeInfo
{
public:
	virtual U32 GetSize() const {return sizeof(Actor::C_ActorHandle);}

	virtual void PushValue(lua_State* pLuaState, const void* pMessageData) const
	{
		const Actor::C_ActorHandle* pValue = reinterpret_cast<const Actor::C_ActorHandle*>(pMessageData);
		LuaActor::PushActorHandle(pLuaState, *pValue);
	}

	virtual void ToValue(lua_State* pLuaState, const int index, void* pMessageData) const
	{
		Actor::C_ActorHandle* pValue = reinterpret_cast<Actor::C_ActorHandle*>(pMessageData);
		*pValue = LuaActor::ToActorHandle(pLuaState, index);
	}

};

const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const Actor::C_ActorHandle*)
{
	static C_LuaActorMessageActorHandleInfo typeInfo;
	return &typeInfo;
}

// xt handle type info
void C_LuaActorMessageXtHandleInfoBase::PushRecord(lua_State* pLuaState, XtRecord* pRecord) const
{
	lua_pushlightuserdata(pLuaState, pRecord);
}

XtRecord* C_LuaActorMessageXtHandleInfoBase::ToRecord(lua_State* pLuaState, const int index) const
{
#if LUA_ACTORS_CHECKS
	if (abs(index) > lua_gettop(pLuaState))
	{
		LOGERROR(LuaActor::GetDebugLogger(), DBSTR "stack index %d out of range %d", index, lua_gettop(pLuaState));
		return NULL;
	}

	if (!lua_isuserdata(pLuaState, index))
	{
		LOGERROR(LuaActor::GetDebugLogger(), DBSTR "expected userdata but got %s", lua_typename(pLuaState, lua_type(pLuaState, index)));
		return NULL;
	}
#endif // LUA_ACTORS_CHECKS

	XtRecord* pRecord = reinterpret_cast<XtRecord*>(lua_touserdata(pLuaState, index));

#if LUA_ACTORS_CHECKS
	if ((pRecord != NULL) && (pRecord->type != m_pXtType))
	{
		LOGERROR(LuaActor::GetDebugLogger(), DBSTR "expected type %s but got %s", m_pXtType->name, pRecord->type->name);
		pRecord = NULL;
	}
#endif // LUA_ACTORS_CHECKS

	return pRecord;
}

void C_LuaActorMessageValueInfoBase::PushValueType(lua_State* pLuaState, const void* pValueData) const
{
	m_pValueTypeInfo->PushValue(pLuaState, pValueData);
}

void C_LuaActorMessageValueInfoBase::ToValueType(lua_State* pLuaState, const int index, void* pValueData) const
{
	m_pValueTypeInfo->ToValue(pLuaState, index, pValueData);
}

} // namespace LuaActorMessages_Impl


namespace LuaActor
{

C_LuaActorMessageMemberReflection::C_LuaActorMessageMemberReflection(const I_LuaActorMessageDataTypeInfo* pTypeInfo, const U32 offset, const char* name) :
	m_pTypeInfo(pTypeInfo),
	m_Offset(offset), 
	m_Name(name)
{
}


C_LuaActorMessageReflection::C_LuaActorMessageReflection(const Messaging::TMsgType messageType, const char* messageName, const U32 messageSize) :
	m_MessageType(messageType), 
	m_MessageName(messageName), 
	m_MessageSize(messageSize),
	m_Members()
{
}

C_LuaActorMessageReflection::~C_LuaActorMessageReflection()
{
	while (!m_Members.Empty())
	{
		C_LuaActorMessageMemberReflection* pMember = &(*m_Members.Begin());
		m_Members.PopFront();
		delete pMember;
	}
}

void C_LuaActorMessageReflection::ReflectMember(const I_LuaActorMessageDataTypeInfo* pTypeInfo, const U32 offset, const char* name)
{
#if LUA_ACTORS_CHECKS
	if (pTypeInfo == NULL)
	{
		LOGERROR(GetDebugLogger(), DBSTR "don't know how to reflect %s::%s", m_MessageName, name);
		return;
	}
#endif

	C_LuaActorMessageMemberReflection* pMember = new C_LuaActorMessageMemberReflection(pTypeInfo, offset, name);
	m_Members.PushBack(*pMember);
}

C_LuaActorMessageFactory::C_LuaActorMessageFactory() :
	m_Messages(),
	m_TypeMap(),
	m_NameMap()
{
}

C_LuaActorMessageFactory::~C_LuaActorMessageFactory()
{
	while (!m_Messages.Empty())
	{
		C_LuaActorMessageReflection* pReflection = &(*m_Messages.Begin());
		m_Messages.PopFront();
		delete pReflection;
	}
}

Messaging::TMsgType C_LuaActorMessageFactory::GetMessageType(const char* messageName) const
{
	const C_NameMap::ConstIterator it = m_NameMap.Find(messageName);
	return (it != m_NameMap.End()) ? it->GetValue()->m_MessageType : -1;
}

const char* C_LuaActorMessageFactory::GetMessageName(const Messaging::TMsgType messageType) const
{
	const C_TypeMap::ConstIterator it = m_TypeMap.Find(messageType);
	return (it != m_TypeMap.End()) ? it->GetValue()->m_MessageName : "";
}

void C_LuaActorMessageFactory::PushMessage(lua_State* pLuaState, const Messaging::C_MessageBase* pMessage) const
{
	assert(pMessage != NULL);

	const C_TypeMap::ConstIterator messageIt = m_TypeMap.Find(pMessage->Type());

	if (messageIt != m_TypeMap.End())
	{
		const C_LuaActorMessageReflection* pReflection = messageIt->GetValue();
		assert(pMessage->Size() == pReflection->m_MessageSize);

		// create the message table
		lua_newtable(pLuaState);

		// write the message name
		lua_pushstring(pLuaState, LuaActorMessages_Impl::g_MessageTypeName);
		lua_pushstring(pLuaState, pReflection->m_MessageName);
		lua_rawset(pLuaState, -2);

		// write the message data
		for (C_LuaActorMessageReflection::C_Members::ConstIterator memberIt = pReflection->m_Members.Begin(); memberIt != pReflection->m_Members.End(); ++memberIt)
		{
			lua_pushstring(pLuaState, memberIt->m_Name);
			memberIt->m_pTypeInfo->PushValue(pLuaState, reinterpret_cast<const U8*>(pMessage) + memberIt->m_Offset);
			lua_rawset(pLuaState, -2);
		}
	}
	else
	{
#if LUA_ACTORS_CHECKS
#ifndef CFG_SPEED
		const char* messageName = pMessage->m_pDebugInfo->m_MsgName;
#else
		const char* messageName = "<unknown>";
#endif
		LOGERROR(GetDebugLogger(), DBSTR "don't know how to convert message %s to Lua", messageName);
#endif // LUA_ACTORS_CHECKS

		lua_pushnil(pLuaState);
	}
}

Messaging::C_MessageBase* C_LuaActorMessageFactory::ToMessage(lua_State* pLuaState, const int index, Messaging::I_MessageAllocator& allocator) const
{
#if LUA_ACTORS_CHECKS
	if (abs(index) > lua_gettop(pLuaState))
	{
		LOGERROR(GetDebugLogger(), DBSTR "stack index %d out of range %d", index, lua_gettop(pLuaState));
		return NULL;
	}

	if (!lua_istable(pLuaState, index))
	{
		LOGERROR(GetDebugLogger(), DBSTR "expected table but got %s", lua_typename(pLuaState, lua_type(pLuaState, index)));
		return NULL;
	}
#endif // LUA_ACTORS_CHECKS

	// get the reflection data
	const C_LuaActorMessageReflection* pReflection = NULL;
	{
		lua_pushstring(pLuaState, LuaActorMessages_Impl::g_MessageTypeName);
		lua_rawget(pLuaState, index);

#if LUA_ACTORS_CHECKS
		if (!lua_isstring(pLuaState, -1))
		{
			LOGERROR(GetDebugLogger(), DBSTR "%s has type %s", LuaActorMessages_Impl::g_MessageTypeName, lua_typename(pLuaState, lua_type(pLuaState, -1)));
			lua_pop(pLuaState, 1);
			return NULL;
		}
#endif // LUA_ACTORS_CHECKS

		const char* messageName = lua_tostring(pLuaState, -1);
		const C_NameMap::ConstIterator it = m_NameMap.Find(messageName);

#if LUA_ACTORS_CHECKS
		if (it == m_NameMap.End())
		{
			LOGERROR(GetDebugLogger(), DBSTR "unknown message type %s", messageName);
			lua_pop(pLuaState, 1);
			return NULL;
		}
#endif // LUA_ACTORS_CHECKS

		pReflection = it->GetValue();
		lua_pop(pLuaState, 1);
	}

	// allocate the message object
	Messaging::C_MessageBase* pMessage = allocator.Create(pReflection->m_MessageType, pReflection->m_MessageSize);

#if LUA_ACTORS_CHECKS
	if (pMessage == NULL)
	{
		LOGERROR(GetDebugLogger(), "failed to allocate message %s of size %d", pReflection->m_MessageName, pReflection->m_MessageSize);
		return NULL;
	}
#endif // LUA_ACTORS_CHECKS

	// read the data
	for (C_LuaActorMessageReflection::C_Members::ConstIterator it = pReflection->m_Members.Begin(); it != pReflection->m_Members.End(); ++it)
	{
		lua_pushstring(pLuaState, it->m_Name);
		lua_rawget(pLuaState, index);

#if LUA_ACTORS_CHECKS
		if (lua_isnil(pLuaState, -1))
		{
			LOGERROR(GetDebugLogger(), DBSTR "member %s of message %s not found", it->m_Name, pReflection->m_MessageName);
		}
		else
#endif // LUA_ACTORS_CHECKS
		{
			it->m_pTypeInfo->ToValue(pLuaState, -1, reinterpret_cast<U8*>(pMessage) + it->m_Offset);
		}

		lua_pop(pLuaState, 1);
	}

#if LUA_ACTORS_CHECKS
	// check for unrecognised fields
	{
		lua_pushnil(pLuaState);
		while (lua_next(pLuaState, index) != 0)
		{
			if (lua_isstring(pLuaState, -2))
			{
				const char* name = lua_tostring(pLuaState, -2);

				if (strcmp(name, LuaActorMessages_Impl::g_MessageTypeName) != 0)
				{
					bool isMember = false;

					for (C_LuaActorMessageReflection::C_Members::ConstIterator it = pReflection->m_Members.Begin(); it != pReflection->m_Members.End(); ++it)
					{
						if (strcmp(name, it->m_Name) == 0)
						{
							isMember = true;
							break;
						}
					}

					if (!isMember)
					{
						LOGERROR(GetDebugLogger(), DBSTR "message %s contains unrecognised member %s of type %s", pReflection->m_MessageName, name, lua_typename(pLuaState, lua_type(pLuaState, -1)));
					}
				}
			}
			else
			{
				LOGERROR(GetDebugLogger(), DBSTR "message %s contains non-string key of type %s", pReflection->m_MessageName, lua_typename(pLuaState, lua_type(pLuaState, -2)));
			}

			lua_pop(pLuaState, 1);
		}
	}
#endif // LUA_ACTORS_CHECKS

	// done
	return pMessage;
}

#if LUA_ACTORS_CHECKS

bool C_LuaActorMessageFactory::IsMessageReflected(const Messaging::TMsgType messageType, const char* messageName) const
{
	// check if the type is already registered
	{
		const C_TypeMap::ConstIterator it = m_TypeMap.Find(messageType);
		if (it != m_TypeMap.End())
		{
			LOGERROR(GetDebugLogger(), DBSTR "message type %s already reflected as %s", messageName, it->GetValue()->m_MessageName);
			return true;
		}
	}

	// check if this name is already used by another type
	{
		const C_NameMap::ConstIterator it = m_NameMap.Find(messageName);
		if (it != m_NameMap.End())
		{
			LOGERROR(GetDebugLogger(), DBSTR "message name %s is already reflected", messageName);
			return true;
		}
	}

	return false;
}

#endif // LUA_ACTORS_CHECKS

void C_LuaActorMessageFactory::ReflectMessage(const Messaging::TMsgType messageType, const char* messageName)
{
#if LUA_ACTORS_CHECKS
	if (!IsMessageReflected(messageType, messageName))
#endif
	{
		C_LuaActorMessageReflection* pReflection = new C_LuaActorMessageReflection(messageType, messageName, sizeof(Messaging::C_MessageBase));
		ReflectMessage(pReflection);
	}
}

void C_LuaActorMessageFactory::ReflectMessage(C_LuaActorMessageReflection* pReflection)
{
	assert(pReflection != NULL);

#if LUA_ACTORS_CHECKS
	assert(!IsMessageReflected(pReflection->m_MessageType, pReflection->m_MessageName));

	// validate message size
	bool sizeAndOffsetsOk = true;
	{
		U32 totalSize = sizeof(Messaging::C_MessageBase);

		for (C_LuaActorMessageReflection::C_Members::ConstIterator it = pReflection->m_Members.Begin(); it != pReflection->m_Members.End(); ++it)
		{
			const U32 sizeOfMember = it->m_pTypeInfo->GetSize();

			if ((it->m_Offset + sizeOfMember) > pReflection->m_MessageSize)
			{
				LOGERROR(GetDebugLogger(), DBSTR "member %s with offset=%d and size=%d does not fit into message %s of size %d", it->m_Name, it->m_Offset, sizeOfMember, pReflection->m_MessageName, pReflection->m_MessageSize);
				sizeAndOffsetsOk = false;
			}

			totalSize += sizeOfMember;
		}

		if (totalSize != pReflection->m_MessageSize)
		{
			LOGERROR(GetDebugLogger(), DBSTR "calculated size %d does not match expected size %d of data for message %s", totalSize, pReflection->m_MessageSize, pReflection->m_MessageName);
			sizeAndOffsetsOk = false;
		}
	}

	if (!sizeAndOffsetsOk)
	{
		delete pReflection;
	}
	else
#endif
	{
		m_Messages.PushBack(*pReflection);
		m_TypeMap[pReflection->m_MessageType] = pReflection;
		m_NameMap[pReflection->m_MessageName] = pReflection;
	}
}

} // namespace LuaActor
