#ifndef MERCURY_SPLINE_ACTOR_CREATE_H
#define MERCURY_SPLINE_ACTOR_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorTypes.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

namespace SharedActors
{
	class C_SplineVec3Wrapper;
}

namespace Actors
{

//////////////////////////////////////////////////////////////////
// Spline Actor class.
//
class C_NodeActorCreate;

class C_SplineActorCreate : public C_PosActorCreate
{
public:
	const Actor::C_ActorTypeId	GetType() const {return E_ActorTypeId_Spline;}
	XtSplineVec3								m_SplineData;
	Components::C_MovementAttacherCreateInfo	m_Attacher;	
};

}

#endif
