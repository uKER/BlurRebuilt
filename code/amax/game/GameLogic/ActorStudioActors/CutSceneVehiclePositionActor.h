#ifdef _MSC_VER
#pragma once
#endif

#ifndef _CUT_SCENE_VEHICLE_POSITION_ACTOR_H
#define _CUT_SCENE_VEHICLE_POSITION_ACTOR_H

#include <actor_system/ActorDeclare.h>

#include <shared_actors/XtActorCreation.h>

#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>

#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

namespace Actors
{

//////////////////////////////////////////////////////////////////
// Cut Scene Vehicle Position Actor class.
//


class C_NodeActorCreate;

class C_CutSceneVehiclePositionActorCreate : public C_PosActorCreate
{
public:
	const Actor::C_ActorTypeId				GetType() const {return Game::E_ActorTypeId_VehiclePositionActor;}
//	int										m_VehicleIndex;
};



class C_CutSceneVehiclePositionActor : public Game::Actors::C_GameActor
{

public:
	C_CutSceneVehiclePositionActor() {}
									
	virtual							~C_CutSceneVehiclePositionActor() {}

	virtual const Maths::C_Vertex		GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}


	
	static const Actor::C_ActorTypeId		TypeId()			{ return Game::E_ActorTypeId_VehiclePositionActor; }
	static const Actor::C_ActorTypeName		TypeName()			{ return "CutSceneVehiclePositionActor"; }
	virtual bool					OnMessage(const Messaging::C_MessageBase *pMessage );

	const C_CutSceneVehiclePositionActorCreate*		GetCreationData() const {return static_cast<const C_CutSceneVehiclePositionActorCreate*>(C_GameActor::GetCreationData().GetData());}

protected:
	virtual bool					CreateImpl();		// from Actor::C_Actor

private:
	Maths::HMat43						m_WorldMatrix;

};

}
#endif
