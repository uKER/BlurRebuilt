#ifdef _MSC_VER
#pragma once
#endif

#ifndef _CUT_SCENE_VEHICLE_SPLINE_ACTOR_H
#define _CUT_SCENE_VEHICLE_SPLINE_ACTOR_H

#include <actor_system/ActorDeclare.h>

#include <shared_actors/XtActorCreation.h>

#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>

#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

namespace SharedActors
{
	class C_SplineVec3Wrapper;
}

namespace Actors
{

//////////////////////////////////////////////////////////////////
// Cut Scene Vehicle Spline Actor class.
//


class C_NodeActorCreate;

class C_CutSceneVehicleSplineActorCreate : public C_PosActorCreate
{
public:
	const Actor::C_ActorTypeId	GetType() const {return Game::E_ActorTypeId_VehicleSplineActor;}

	XtSplineVec3								m_SplineData;
	Components::C_MovementAttacherCreateInfo	m_Attacher;	
	int											m_MPH;
};



class C_CutSceneVehicleSplineActor : public Game::Actors::C_GameActor
{

public:
	C_CutSceneVehicleSplineActor();
	virtual	~C_CutSceneVehicleSplineActor();
	
	static const Actor::C_ActorTypeId		TypeId()			{ return Game::E_ActorTypeId_VehicleSplineActor; }
	static const Actor::C_ActorTypeName		TypeName()			{ return "CutSceneVehicleSplineActor"; }

	virtual bool OnMessage(const Messaging::C_MessageBase *pMessage );
	
	const C_CutSceneVehicleSplineActorCreate*	GetCreationData() const { return static_cast<const C_CutSceneVehicleSplineActorCreate*>(C_GameActor::GetCreationData().GetData()); }
	const SharedActors::C_SplineVec3Wrapper*	GetSplineWrapper() const { return m_pSplineWrapper; }

protected:
	virtual bool					CreateImpl();		// from Actor::C_Actor

private:
	void							Update(const Maths::C_TimeS& timeStep);

	SharedActors::C_SplineVec3Wrapper*	m_pSplineWrapper;
};

}
#endif
