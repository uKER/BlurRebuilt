#include "stdafx.h"

#include <Core/UtMaths.h>
#include <misc_shared/messaging/MessageTypeList.h>
#include <Misc_shared/NewCurveHermite.h>
#include <Misc_Shared/NewCurve/Actor/ActorSplineTypes.h>

#pragma warning (push, 4)
#include "MovementDualSpline.h"
#include <camera2/camera.h>
#pragma warning (push, 3)
#include <Shared_Actors/ActorImpl/SplineActor.h>
#include <Shared_Actors/TypeWrappers/SplineVec3Wrapper.h>
#include <actor_system/ActorSys.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>
#pragma warning (pop)
#include <game/AmaxCam/Helper.h>

namespace Components
{
using namespace Maths;

C_MovementDualSpline::C_MovementDualSpline()
{
	m_Counter = 0.f;
	m_AnimationTime = 0.f;
	m_Active = false;
}

bool C_MovementDualSpline::CreateImpl()
{
	// Get handles to the SplineActors for position and target that have been created:
	const C_MovementDualSplineCreate* create = GetCreationData();
	if (create && create->m_Position)
		m_PositionHandle = create->m_Position->GetActorHandle();
	if (create && create->m_Target)
		m_TargetHandle = create->m_Target->GetActorHandle();

	m_AnimationTime = create->m_AnimationTime;
	SetIsAnimating(true);
	m_Counter = 0.f;
	m_Active = false;

	return C_MovementBase::CreateImpl();
}

ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(C_MovementDualSpline)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_GameActorMsg_Activate, ActorMsg::E_GameActorMsg_Deactivate)
ACTOR_COMPONENT_MESSAGE_TYPES_END(C_MovementDualSpline, C_MovementBase)

bool C_MovementDualSpline::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
		case ActorMsg::E_GameActorMsg_Activate:
			m_Counter = 0.f;
			m_Active = true;
		return true;

		case ActorMsg::E_GameActorMsg_Deactivate:
			m_Active = false;
		return true;
	}
	
	return C_MovementBase::OnMessage(pMessage);
}

void C_MovementDualSpline::Animate(const C_TimeS& timeStep)
{
	if (m_Active)
		m_Counter += timeStep.Get();

	F32 x = 0.f;
	if (m_AnimationTime > 1e-5f)
	{
		if (m_Counter >= m_AnimationTime)
			m_Counter  = m_AnimationTime;

		x = m_Counter / m_AnimationTime;
	}

	const Actors::C_SplineActor* positionSpline = HandleToSpline(m_PositionHandle);
	const Actors::C_SplineActor* targetSpline   = HandleToSpline(m_TargetHandle);

	if (!positionSpline)
		return;

	// Evaluate position and target from splines:
	HVec3 pos = positionSpline->GetSplineWrapper()->GetCurveEval().Evaluate(x);

	m_LocalTransform.m_Translation = pos;

	if (targetSpline)
	{
		HVec3 target = targetSpline->GetSplineWrapper()->GetCurveEval().Evaluate(x);
		m_LocalTransform.m_Quaternion = Cam2::C_Camera::GetLookQuat(target - pos);
	}
	else
	{
		// Use the gradient of the curve:
		F32 b = x + 0.1f;
		if (b > 1.f) b = 1.f;
		F32 a = b - 0.1f;
		HVec3 pa = positionSpline->GetSplineWrapper()->GetCurveEval().Evaluate(a);
		HVec3 pb = positionSpline->GetSplineWrapper()->GetCurveEval().Evaluate(b);
		m_LocalTransform.m_Quaternion = Cam2::C_Camera::GetLookQuat(pb - pa);
	}
	
	m_ParentOffset = C_QuatTrans();
}

const Actors::C_SplineActor* C_MovementDualSpline::HandleToSpline(const Actor::C_ActorHandle& handle)
{
	Actor::C_ActorList* gal = GetActorList();
	if (!gal)
		return 0;

	const Actor::C_Actor* actor = gal->GetActorPtr(handle);
	if (!actor)
		return 0;

	return Actor::ActorCast_Safe<Actors::C_SplineActor>(actor);
}

}

XT_TYPE(Components::C_MovementDualSplineCreate, "DualSplineMovementComponent")
	XT_BASE(Components::C_MovementCreateInfo)
	XT_FIELD(m_Position, "position")
	XT_FIELD(m_Target, "target")
	XT_FIELD(m_AnimationTime, "animationTime")
XT_END_TYPE

#pragma warning (pop)
