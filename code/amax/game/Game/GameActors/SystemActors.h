#ifndef GAME_SYSTEM_ACTORS_H
#define GAME_SYSTEM_ACTORS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorSys.h>
#include <shared_actors/XtActorBase.h>

#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>

namespace Game
{

namespace Actors
{

// Basic Node type actor
class C_NodeActorCreate : public ::Actors::C_PosActorCreate
{
public:
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_Node;}
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_Node;}

};

// Actor Info
class C_NodeActor : public C_GameActor
{
public:
	C_NodeActor(){}
	bool Create(const Actor::C_ActorCreationData *creationData)
	{
		const C_NodeActorCreate * pNodeCreate = (const C_NodeActorCreate *)creationData;

		m_WorldMatrix = pNodeCreate->GetCreationMatrix();

		return true;
	}

	const Maths::HMat43 &			GetMatrix() const {return m_WorldMatrix;}

	static const Actor::C_ActorTypeId		TypeId()	{ return E_ActorTypeId_Node; }
	static const Btl::String32	TypeName()	{ return "Node"; }

private:
	Maths::HMat43	m_WorldMatrix;
};

} // namespace Actors


} // namespace Game

#endif //  GAME_ACTOR_TYPES_H
