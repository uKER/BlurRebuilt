#ifndef LUAACTOR_54D386A8_1269_417B_816C_5A95689C7E5D_H
#define LUAACTOR_54D386A8_1269_417B_816C_5A95689C7E5D_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/Actor.h>
#include <lua_actors/LuaActorCreate.h>
#include <lua_actors/LuaActorDeclare.h>


namespace LuaActor
{

class C_LuaActor : public Actor::C_Actor
{
public:
	C_LuaActor();
	virtual ~C_LuaActor();

	virtual bool MessageHandler(const Messaging::C_MessageBase* pMessage); // from Messaging::C_MessageHandler

	virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const; // from Actor::C_Actor
	virtual void DeserialiseForSave(Sys::C_ContextualStream& stream); // from Actor::C_Actor

	virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const; // from Actor::C_Actor
	virtual void DeserialiseForSync(Sys::C_ContextualStream& stream); // from Actor::C_Actor

	const C_LuaActorCreate* GetCreationData() const {return static_cast<const C_LuaActorCreate*>(Actor::C_Actor::GetCreationData().GetData());}

	void PushInstance(lua_State* pLuaState);

protected:
	virtual bool InitialiseImpl(); // from Actor::C_Actor
	virtual bool PrimeImpl(); // from Actor::C_Actor
	virtual bool CreateImpl(); // from Actor::C_Actor
	virtual void KillImpl(); // from Actor::C_Actor

private:
	const C_LuaActorScript* m_pLuaActorScript;

	bool AcquireInstance(lua_State* pLuaState);
	bool ReleaseInstance(lua_State* pLuaState);

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

	lua_State* GetLuaState() const;
	const C_LuaActorMessageFactory* GetLuaActorMessageFactory() const;

	static C_LuaActor* GetFromInstance(lua_State* pLuaState, const int index);

	static bool SerialiseTable(Sys::C_ContextualStream& stream, lua_State* pLuaState, const int index);
	static bool DeserialiseTable(Sys::C_ContextualStream& stream, lua_State* pLuaState, const int index);

	static bool SerialiseLua(Sys::C_ContextualStream& stream, lua_State* pLuaState, const int index);
	static bool DeserialiseLua(Sys::C_ContextualStream& stream, lua_State* pLuaState, const int luaType);

public:
	static int LuaAddSubscriber(lua_State* pLuaState);
	static int LuaBroadcastMessage(lua_State* pLuaState);
	static int LuaGetActorHandle(lua_State* pLuaState);
	static int LuaGetActorName(lua_State* pLuaState);
	static int LuaGetCreationData(lua_State* pLuaState);
	static int LuaInitialiseComponent(lua_State* pLuaState);
	static int LuaRemoveSubscriber(lua_State* pLuaState);
	static int LuaSendMessageToActor(lua_State* pLuaState);

};

template <Actor::C_ActorTypeId t_TypeId, const char* t_TypeName>
class C_LuaActorImpl : public C_LuaActor
{
public:
	static const Actor::C_ActorTypeId TypeId() {return t_TypeId;}
	static const Actor::C_ActorTypeName TypeName() {return t_TypeName;}

};

} // namespace LuaActor


#endif // LUAACTOR_54D386A8_1269_417B_816C_5A95689C7E5D_H
