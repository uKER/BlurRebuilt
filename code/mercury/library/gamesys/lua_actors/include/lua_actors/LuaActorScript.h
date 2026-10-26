#ifndef LUAACTORSCRIPT_2A5DA896_55C8_445E_A0EC_7D7C403CB3AA_H
#define LUAACTORSCRIPT_2A5DA896_55C8_445E_A0EC_7D7C403CB3AA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/Set.h>
#include <btl/utility/String.h>
#include <lua_actors/LuaActorDeclare.h>
#include <misc_shared/messaging/MessagingDeclare.h>


namespace LuaActor
{

enum E_LifetimeHook
{
	E_LifetimeHook_Initialise = 0, 
	E_LifetimeHook_Prime, 
	E_LifetimeHook_Create, 
	E_LifetimeHook_Kill, 
	E_LifetimeHook_Destruct, 
	E_LifetimeHook_Count
};

extern const char* GetLifetimeHookName(const E_LifetimeHook lifetimeHook);


class C_LuaActorScript
{
public:
	C_LuaActorScript();
	~C_LuaActorScript();

	bool Init(const I_LuaActorContext* pLuaActorContext, const Btl::String& scriptName);
	void Clear();
	
	const Btl::String& GetScriptName() const {return m_ScriptName;}

	bool DoLifetimeHook(C_LuaActor* pLuaActor, const E_LifetimeHook lifetimeHook) const;
	bool DoMessageHandler(C_LuaActor* pLuaActor, const Messaging::C_MessageBase* pMessage) const;

private:
	typedef Btl::Set<Messaging::TMsgType> C_Messages;

	const I_LuaActorContext*	m_pLuaActorContext;
	Btl::String					m_ScriptName;
	U32							m_LifetimeHooks;
	C_Messages					m_Messages;

	C_LuaActorScript(const C_LuaActorScript&); // can not copy
	C_LuaActorScript& operator = (const C_LuaActorScript&); // can not copy

	bool PushScriptTable(lua_State* pLuaState) const;

};

} // namespace LuaActor


#endif // LUAACTORSCRIPT_2A5DA896_55C8_445E_A0EC_7D7C403CB3AA_H
