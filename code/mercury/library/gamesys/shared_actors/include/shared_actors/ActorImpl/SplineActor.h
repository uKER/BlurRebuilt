#ifndef MERCURY_SPLINE_ACTOR_H
#define MERCURY_SPLINE_ACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorBase.h>
#include <shared_actors/ActorCreation/SplineActorCreate.h>


namespace Actors
{

//Shared spline actor class.
class C_SplineActor : public C_SharedActorBase
{

public:
	C_SplineActor();	
	virtual	~C_SplineActor();
	
	static const Actor::C_ActorTypeId	TypeId()			{ return E_ActorTypeId_Spline; }
	static const Actor::C_ActorTypeName	TypeName()			{ return "SplineActor"; }
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage );
	virtual void						DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
	const C_SplineActorCreate*			GetCreationData() const {return (const C_SplineActorCreate*)Actor::C_Actor::GetCreationData().GetData();}

	const SharedActors::C_SplineVec3Wrapper* GetSplineWrapper() const { return m_pSplineWrapper; }

protected:
	virtual bool						PrimeImpl();		// from Actor::C_Actor

private:
	void								Update( const Maths::C_TimeS& timeStep );

	SharedActors::C_SplineVec3Wrapper*	m_pSplineWrapper;
};

}

#endif
