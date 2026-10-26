#ifndef GROUPACTORSCREATE_2BC66FEF_26C8_4EB7_818E_432486F61B16_H
#define GROUPACTORSCREATE_2BC66FEF_26C8_4EB7_818E_432486F61B16_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorTypes.h>
#include <shared_actors/XtActorBase.h>


namespace Actors
{

/// Base class for creating C_GroupActor.
class C_GroupActorCreate : public C_GameActorCreate
{
public:
	static const Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_Group;}
	virtual const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_Group;}

	virtual U32 GetNumMembers() const = 0;
	virtual Actor::C_ActorHandle GetMember(const U32 index) const = 0;

};

/// Creates C_GroupActor from ActorStudio data.
class C_GroupActorCreateXt : public C_GroupActorCreate
{
public:
	XtArray<C_GameActorCreate*> m_Members;

	virtual U32 GetNumMembers() const; // from C_GroupActorCreate
	virtual Actor::C_ActorHandle GetMember(const U32 index) const; // from C_GroupActorCreate

};

/// Creates C_GroupActor from procedural data.
class C_GroupActorCreateRuntime : public C_GroupActorCreate
{
public:
	Btl::Vector<Actor::C_ActorHandle> m_Members;

	virtual U32 GetNumMembers() const; // from C_GroupActorCreate
	virtual Actor::C_ActorHandle GetMember(const U32 index) const; // from C_GroupActorCreate

	virtual void Serialise(Sys::C_ContextualStream& stream) const; // from Actor::C_ActorCreationData
	virtual void Deserialise(Sys::C_ContextualStream& stream); // from Actor::C_ActorCreationData

};

} // namespace Actors


#endif // GROUPACTORSCREATE_2BC66FEF_26C8_4EB7_818E_432486F61B16_H
