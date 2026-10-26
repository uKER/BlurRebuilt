#ifndef ACTORMESSAGEDATA_H
#define ACTORMESSAGEDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <actor_system/ActorSerialisation.h>
#include <misc_shared/context/ContextualStream.h>
#include <misc_shared/messaging/MessageBase.h>


namespace Actor
{

/**
@brief Base helper class for messages carrying an actor handle.
*/
class C_MessageActorData : public Messaging::C_MessageBase
{
public:
	C_MessageActorData(const Messaging::TMsgType typeId, const U32 size) : Messaging::C_MessageBase(typeId, size), m_ActorHandle() {}
	explicit C_MessageActorData(const Messaging::TMsgType typeId, const U32 size, const C_ActorHandle actorHandle) : Messaging::C_MessageBase(typeId, size), m_ActorHandle(actorHandle) {}
	// auto: C_MessageActorData(const C_MessageActorData& copy);
	// auto: ~C_MessageActorData();
	// auto: C_MessageActorData& operator=(const C_MessageActorData& copy);

	C_ActorHandle GetActorHandle() const {return m_ActorHandle;} ///< @returns actor handle for this message.
	void SetActorHandle(const C_ActorHandle& actorHandle) {m_ActorHandle = actorHandle;} ///< @param actorHandle actor handle for this message.

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage) {stream << static_cast<const C_MessageActorData*>(pMessage)->m_ActorHandle;}
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage) {stream >> static_cast<C_MessageActorData*>(pMessage)->m_ActorHandle;}

private:
	C_ActorHandle m_ActorHandle; ///< handle of actor

};

} // namespace Actor


#endif // ACTORMESSAGEDATA_H
