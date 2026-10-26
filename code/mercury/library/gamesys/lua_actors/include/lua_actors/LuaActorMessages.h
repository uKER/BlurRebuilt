#ifndef LUAACTORMESSAGES_31B33BF6_A87A_44DC_86B5_B2FA39000351_H
#define LUAACTORMESSAGES_31B33BF6_A87A_44DC_86B5_B2FA39000351_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/list.h>
#include <btl/containers/map.h>
#include <btl/utility/string.h>
#include <lua_actors/LuaActorDeclare.h>
#include <misc_shared/messaging/MessagingDeclare.h>


namespace LuaActor
{

class I_LuaActorMessageDataTypeInfo
{
public:
	virtual ~I_LuaActorMessageDataTypeInfo() {}

	virtual U32 GetSize() const = 0;

	virtual void PushValue(lua_State* pLuaState, const void* pMessageData) const = 0;
	virtual void ToValue(lua_State* pLuaState, const int index, void* pMessageData) const = 0;

};

struct C_LuaActorMessageMemberReflection : public Btl::ListElementSingle<C_LuaActorMessageMemberReflection>
{
	const I_LuaActorMessageDataTypeInfo*	m_pTypeInfo;
	U32										m_Offset;
	const char*								m_Name;

	C_LuaActorMessageMemberReflection(const I_LuaActorMessageDataTypeInfo* pTypeInfo, const U32 offset, const char* name);
};

struct C_LuaActorMessageReflection : public Btl::ListElementSingle<C_LuaActorMessageReflection>
{
	typedef Btl::List<C_LuaActorMessageMemberReflection> C_Members;

	Messaging::TMsgType	m_MessageType;
	const char*			m_MessageName;
	U32					m_MessageSize;
	C_Members			m_Members;
	
	C_LuaActorMessageReflection(const Messaging::TMsgType messageType, const char* messageName, const U32 messageSize);
	~C_LuaActorMessageReflection();

	template <class T_DataType>
	inline const I_LuaActorMessageDataTypeInfo* MakeTypeInfo(const T_DataType&);
	
	void ReflectMember(const I_LuaActorMessageDataTypeInfo* pTypeInfo, const U32 offset, const char* name);
};

class C_LuaActorMessageFactory
{
public:
	C_LuaActorMessageFactory();
	~C_LuaActorMessageFactory();

	void ReflectMessage(const Messaging::TMsgType messageType, const char* messageName);

	template <class T_MessageType>
	void ReflectMessage(const char* messageName);

	Messaging::TMsgType GetMessageType(const char* messageName) const;
	const char* GetMessageName(const Messaging::TMsgType messageType) const;

	void PushMessage(lua_State* pLuaState, const Messaging::C_MessageBase* pMessage) const;
	Messaging::C_MessageBase* ToMessage(lua_State* pLuaState, const int index, Messaging::I_MessageAllocator& allocator) const;

private:
	typedef Btl::List<C_LuaActorMessageReflection> C_Messages;
	typedef Btl::Map<Messaging::TMsgType, const C_LuaActorMessageReflection*> C_TypeMap;
	typedef Btl::Map<Btl::String, const C_LuaActorMessageReflection*> C_NameMap;

	C_Messages	m_Messages;
	C_TypeMap	m_TypeMap;
	C_NameMap	m_NameMap;

#if LUA_ACTORS_CHECKS
	bool IsMessageReflected(const Messaging::TMsgType messageType, const char* messageName) const;
#endif // LUA_ACTORS_CHECKS

	void ReflectMessage(C_LuaActorMessageReflection* pReflection);

};

} // namespace LuaActor


#include <lua_actors/LuaActorMessages.inl>

#endif // LUAACTORMESSAGES_31B33BF6_A87A_44DC_86B5_B2FA39000351_H
