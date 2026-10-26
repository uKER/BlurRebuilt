#ifndef SYSTEM_ACTORS_CREATE_H
#define SYSTEM_ACTORS_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorTypes.h>
#include <shared_actors/XtActorBase.h>


namespace Components
{
	class C_MovementCreateInfo;
}


namespace Actors
{

// Null Actor
// This actor has a position but no behaviour
class C_NullActorCreate : public C_PosActorCreate
{
public:
	const Actor::C_ActorTypeId					GetType() const {return E_ActorTypeId_NullActor;}

};

// Proxy Actor
// This actor will be assigned an actor it is proxy to by a message

class C_ProxyActorCreate : public C_GameActorCreate
{
public:
	const Actor::C_ActorTypeId					GetType() const {return E_ActorTypeId_ProxyActor;}
	C_GameActorCreate *							m_InitialActor;
};

// Basic Node type actor
class C_NodeActorCreate : public C_PosActorCreate
{
public:
	const Actor::C_ActorTypeId					GetType() const {return E_ActorTypeId_Node;}

	Components::C_MovementCreateInfo*	m_pSorceryMovementComponentCreate;

	// from Actor::C_ActorCreationData
	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};

}

#endif


