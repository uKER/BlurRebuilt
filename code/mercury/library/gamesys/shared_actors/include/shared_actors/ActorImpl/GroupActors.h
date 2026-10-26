#ifndef GROUPACTORS_41CC9C3B_4E1B_4C47_BE68_ED090F91C67C_H
#define GROUPACTORS_41CC9C3B_4E1B_4C47_BE68_ED090F91C67C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorBase.h>
#include <shared_actors/ActorCreation/GroupActorsCreate.h>


class C_LevelActorCreationList;


namespace Actors
{

/// Forwards messages to a number of other actors.
class C_GroupActor : public C_SharedActorBase
{
public:
	C_GroupActor();
	virtual ~C_GroupActor();

	static const Actor::C_ActorTypeId TypeId() {return E_ActorTypeId_Group;}
	static const Actor::C_ActorTypeName TypeName() {return "GroupActor";}

	virtual bool OnMessage(const Messaging::C_MessageBase* pMessage); // from Messaging::C_MessageHandler

	bool AddActor(Actor::C_ActorHandle actorHandle);
	bool RemoveActor(Actor::C_ActorHandle actorHandle);

	bool ContainsActor(const Actor::C_ActorHandle& actorHandle) const;

	virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void DeserialiseForSync(Sys::C_ContextualStream& stream);

	const C_GroupActorCreate* GetCreationData() const {return static_cast<const C_GroupActorCreate*>(Actor::C_Actor::GetCreationData().GetData());}

protected:
	virtual bool CreateImpl(); // from Actor::C_Actor

	void AddValidActors(const C_LevelActorCreationList& levelActorCreationList);

private:
	typedef Btl::Vector<Actor::C_ActorHandle> C_Members;

	C_Members				m_Members; ///< current members
	bool					m_bEnableMessages; ///< messages are forwarded to members
	Actor::C_ActorHandle	m_ForwardVolume;	///< Forward to restrict actor forwarding
#ifndef CFG_GOLD
	U32 m_DebugLockCount; ///< if non-zero then we're iterating through our members and really shouldn't be adding or remove them!
#endif

protected:
	S32 FindIndex(const Actor::C_ActorHandle& handle) const;

	void ForwardMessage(const Messaging::C_MessageBase* pMessage);
	void ForwardMessageToVolume(const Messaging::C_MessageBase* pMessage, Actor::C_ActorHandle volumeActor);
	void ForwardMessage(Messaging::TMsgType msgType);

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

};

} // namespace Actors


#endif // GROUPACTORS_41CC9C3B_4E1B_4C47_BE68_ED090F91C67C_H
