#include "stdafx.h"

#include <Misc_shared/NewCurveHermite.h>
#include <Misc_Shared/NewCurve/Actor/ActorSplineTypes.h>

#include <Shared_Actors/TypeWrappers/SplineVec3Wrapper.h>

#include <Game/Game.h>

#include <GameLogic/ActorStudioActors/CutSceneVehicleSplineActor.h>

#include <AI/AI_Messages.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off ) 
#endif


// Gameplay Editor Create
XT_TYPE(Actors::C_CutSceneVehicleSplineActorCreate, "CutSceneVehicleSplineActor")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(m_SplineData, "SplineData")
	XT_FIELD(m_Attacher, "Attacher")	
	XT_FIELD(m_MPH, "MilesPerHour")
XT_END_TYPE

namespace Actors
{

C_CutSceneVehicleSplineActor::C_CutSceneVehicleSplineActor()
{
	m_pSplineWrapper = new SharedActors::C_SplineVec3Wrapper;
}

C_CutSceneVehicleSplineActor::~C_CutSceneVehicleSplineActor()
{
	SAFE_DELETE_PTR(m_pSplineWrapper);
}

bool C_CutSceneVehicleSplineActor::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
		case GameMessaging::E_GameMsg_Update:
			Update(Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue());
			break;
		default:
			return C_GameActor::OnMessage(pMessage);
	}
	return true;
}

bool C_CutSceneVehicleSplineActor::CreateImpl()
{
	if(!C_GameActor::CreateImpl())
		return false;

	const C_CutSceneVehicleSplineActorCreate* pSplineActorCreate(GetCreationData());
	const Maths::HMat43 createMat(pSplineActorCreate->GetCreationMatrix());
	
	bool created = true;
	if (pSplineActorCreate->m_SplineData.knots.count > 0)
		created &= m_pSplineWrapper->Initialise(&pSplineActorCreate->m_SplineData, createMat);

	//Copy knot positions to AI system where a NURBS curve will be generated.
	AIAmax::AICreateSplineMsg create_spline_msg;
	create_spline_msg.m_SplineName=Btl::String32(pSplineActorCreate->m_Name);
	create_spline_msg.m_ReSample=true;
	U32 nKnots = m_pSplineWrapper->GetCurveData().GetNumKnots();
	//Allocate sample point storage.
	create_spline_msg.m_samplePoints.Resize(nKnots);
	//Copy knot positions only.
	for(U32 i = 0; i < nKnots; ++i)
	{
		Curve::Hermite3DActorKnot& knot = m_pSplineWrapper->GetCurveData().AcquireKnot(i);
		create_spline_msg.m_samplePoints[i] = knot.value;
		m_pSplineWrapper->GetCurveData().ReleaseKnot();
	}

	Game::C_Game::GetInstance()->GetSubscriberSystem().BroadcastMessage( &create_spline_msg );
	return created;
}

void C_CutSceneVehicleSplineActor::Update(const Maths::C_TimeS& timeStep)
{
}

}	//namespace Actors

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on ) 
#endif