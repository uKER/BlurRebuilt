#ifndef UPDATE_MANAGER_MESSAGES_H
#define UPDATE_MANAGER_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorMessages.h>


namespace ActorMsg
{
// Update Manager related messages
// --------------------------------
// Seed sends its position to actors
class C_UpdateSeedPositionMsg : public Messaging::C_MessageDerived<C_UpdateSeedPositionMsg, ActorMsg::E_GameActorMsg_UpdateSeedPosition>
{
public:
	explicit C_UpdateSeedPositionMsg(Maths::C_Vertex position)
		:m_Position(position){}

	Maths::C_Vertex			GetPosition() const {return Maths::C_Vertex(m_Position.x,m_Position.y,m_Position.z);}

	MathsStorage::Float3	m_Position;
};

// Seed sends message to request position
class C_SeedRequestPositionMsg : public Messaging::C_MessageDerived<C_SeedRequestPositionMsg, ActorMsg::E_GameActorMsg_SeedRequestPosition>
{
public:
	C_SeedRequestPositionMsg(Actor::C_ActorHandle seedActor)
		:m_SeedActor(seedActor){}


	Actor::C_ActorHandle	m_SeedActor;
};

// counted actor sends position to seed
class C_CountedActorPositionMsg : public Messaging::C_MessageDerived<C_CountedActorPositionMsg, ActorMsg::E_GameActorMsg_CountedActorPosition>
{
public:
	C_CountedActorPositionMsg(U32 groupID,Maths::C_Vertex position, Actor::C_ActorHandle actorHandle)
		:m_GroupId(groupID),m_Position(position),m_ActorHandle(actorHandle){}

	Maths::C_Vertex			GetPosition() const {return Maths::C_Vertex(m_Position.x,m_Position.y,m_Position.z);}

	S32						m_GroupId;
	MathsStorage::Float3	m_Position;
	Actor::C_ActorHandle			m_ActorHandle;
};

// counted actor gets its update order
class C_CountedActorOrderMsg : public Messaging::C_MessageDerived<C_CountedActorOrderMsg, ActorMsg::E_GameActorMsg_CountedActorOrder>
{
public:
	explicit C_CountedActorOrderMsg(U32 order)
		:m_Order(order){}

	U32 m_Order;
};

}

#endif
