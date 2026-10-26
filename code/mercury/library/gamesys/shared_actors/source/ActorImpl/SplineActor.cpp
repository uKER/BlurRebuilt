#include "SharedActorsPch.h"
#include <Shared_Actors/ActorImpl/SplineActor.h>

#include <misc_shared/messaging/GameMessaging.h>
#include <Shared_Actors/TypeWrappers/SplineVec3Wrapper.h>

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize( "", off ) 
#endif

namespace Actors
{

C_SplineActor::C_SplineActor()
{
	m_pSplineWrapper = new SharedActors::C_SplineVec3Wrapper;
}
									
C_SplineActor::~C_SplineActor()
{
	SAFE_DELETE_PTR(m_pSplineWrapper);
}

bool C_SplineActor::PrimeImpl()
{
	bool primed = Actor::C_Actor::PrimeImpl();

	const C_SplineActorCreate* splineActorCreate(GetCreationData());
	const Maths::HMat43 createMat(GetCreationData()->GetCreationMatrix());
	
	if (splineActorCreate->m_SplineData.knots.count > 0)
		primed &= m_pSplineWrapper->Initialise(&splineActorCreate->m_SplineData, createMat);

	return primed;
}

void C_SplineActor::Update(const Maths::C_TimeS& timeStep)
{
}

bool C_SplineActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	switch(pMessage->Type())
	{
		case GameMessaging::E_GameMsg_Update:
			Update(Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue());
			break;
		default:
			return C_SharedActorBase::OnMessage(pMessage);
	}
	return true;
}

void C_SplineActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	m_pSplineWrapper->DebugRender(debugRenderPtr);
}

}

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize( "", on ) 
#endif

