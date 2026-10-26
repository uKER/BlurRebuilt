#include "SharedActorsPch.h"
#include <Shared_Actors/Components/Impl/MovementSpline.h>
#include <Shared_actors/Components/Interfaces/I_MotionPredictor.h>

#include <xt/XtGame.h>
#include <xt/XtStd.h>

#include <gravity/narrowphase/PrimitiveTest.h>

#include <Misc_shared/NewCurveHermite.h>
#include <Misc_Shared/NewCurve/Actor/ActorSplineTypes.h>
#include <Misc_Shared/NewCurve/Actor/ActorHelpers.h>

#include <Gravity/NarrowPhase/Primitive.h>
#include <Gravity/NarrowPhase/CollisionResult.h>
#include <Gravity/NarrowPhase/PrimitiveTest.h>
#include <Gravity/NarrowPhase/Primitive.inl>

#include <Shared_Actors/Components/Creation/MovementComponentsCreate.h>
#include <Shared_Actors/ActorImpl/SplineActor.h>
#include <Shared_Actors/TypeWrappers/SplineVec3Wrapper.h>

#include <actor_system/ActorDebug.h>
#include <actor_system/ActorList.h>

#include <system/perf.h>

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

namespace Components
{

///////////////////////////////////////////////////////////////////////////////////////////////////
//Spline.
//
bool C_MovementSpline::InitialiseImpl()
{
	m_Rotation = 0.0f;
	m_ArcLengthProgression = 0.0f;
	m_AnimateTime = 0.0f;
	m_Loop = false;
	m_StartOffset = 0;

	if (GetCreationData()->m_pSplineActorCreate)
	{
		m_SplineActorHandle = GetCreationData()->m_pSplineActorCreate->GetActorHandle();
		m_MoveSpeed = GetCreationData()->m_MoveSpeed;
		m_Rotation = Maths::DegToRad(GetCreationData()->m_Rotation);
		m_Loop = GetCreationData()->m_Loop;
	}

	return C_MovementBase::InitialiseImpl();
}

bool C_MovementSpline::CreateImpl()
{
	if (m_SplineActorHandle)
	{
		const Actors::C_SplineActor* pSplineActor = 
			Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));
		if(pSplineActor)
			SetInitialPosition(m_SplineActorHandle);
	}

	return C_MovementBase::CreateImpl();
}

void C_MovementSpline::ExternalDependenciesCreatedImpl()
{
	C_MovementBase::ExternalDependenciesCreatedImpl();

	if( GetCreationData()->m_StartImmediately)
	{
		const Actors::C_SplineActor* pSplineActor = 
			Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));
		if(pSplineActor)
		{
			SetInitialPosition(m_SplineActorHandle);

			MoveAlongSpline(
				m_SplineActorHandle, 
				m_MoveSpeed, 
				m_Rotation,
//				ActorMsg::E_MovementFlag_Smooth);
				ActorMsg::E_MovementFlag_Linear);
		}
	}
}

void C_MovementSpline::Animate(const Maths::C_TimeS& timeStep)
{
	const Actors::C_SplineActor* pSplineActor = 
		Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));
	
	m_AnimateTime = Min(m_AnimateDuration.m_Scalar, m_AnimateTime.m_Scalar + timeStep.m_Scalar);

	if (!pSplineActor)
		return;

	bool clamped = false;
	if (m_Loop && pSplineActor->GetSplineWrapper()->IsClosed())
	{
		clamped = true;
		if (m_AnimateTime > m_AnimateDuration)
			m_AnimateTime -= m_AnimateDuration;
	}

	if (m_AnimateTime < m_AnimateDuration)
	{
		const F32 frac = CalcInterpolationFraction(
			m_AnimateTime.Get(), 
			m_AnimateDuration.Get(), 
			m_MotionFlags);

		m_ArcLengthProgression = pSplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(
			m_LocalTransform,
			frac, 
			m_StartOffset,
			m_MoveSpeed, 
			timeStep.Get(),
			clamped);
		// allow the body to be rotated by a specified amount
		const Maths::HVec3 axis(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY());
		const Maths::C_Quaternion rotQuat(axis, m_Rotation);
		const Maths::C_Quaternion newQuat = 
			Maths::C_Quaternion(m_LocalTransform.Quaternion() * rotQuat).Normalised();
		m_LocalTransform.Quaternion(newQuat);
	}
	else
	{
		//Note that if our spline is closed we won't get here so that we can seamlessly loop.
		SetIsAnimating(false);
		if (m_Loop)
		{
			MoveAlongSpline(
				m_SplineActorHandle, 
				m_MoveSpeed,
				m_Rotation, 
				m_MotionFlags);
		}
	}
}

void C_MovementSpline::SetInitialPosition(Actor::C_ActorHandle splineHandle)
{
	m_ArcLengthProgression = 0.0f;
	const Actors::C_SplineActor* pSplineActor = 
		Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));
	if (!pSplineActor)
		return;

	if (GetCreationData()->m_StartFromNearestPoint)
	{
		Maths::HVec3 closestPos;
		m_StartOffset = Curve::Actor::FindNearestPoint(
			*pSplineActor->GetSplineWrapper()->GetCurveEval().GetCurve(),
			pSplineActor->GetSplineWrapper()->GetCurveEval().GetReparam(),
			m_WorldTransform.Translation(),
			closestPos);
		m_ArcLengthProgression = m_StartOffset;
		//Snap translation to closest position.
		m_WorldTransform.Translation(closestPos);
	}
}

void C_MovementSpline::MoveAlongSpline(
	Actor::C_ActorHandle splineHandle,
	F32 moveSpeed,
	F32 rotation,
	ActorMsg::TMotionTypeFlags motionFlags)
{
	if (IsAnimating())
	{
#ifndef CFG_GOLD
		static int annoyingMessageCount = 0;

		annoyingMessageCount++;

		if ((annoyingMessageCount < 3) || ((annoyingMessageCount % 500) == 0))
		{
			LOGERROR(Actor::GetDebugLogLevelDesign(GetActor()), DBSTR"Move Spline Message to Actor already moving. Perhaps was set to \"Start Immediately\"? (hit %i times)", annoyingMessageCount);
		}
#endif

		return;
	}
	m_ParentOffset = Maths::C_QuatTrans();
	m_SplineActorHandle = splineHandle;
	m_Rotation = rotation;
	m_MoveSpeed = moveSpeed;
	m_AnimateTime = 0.0f;
	m_MotionFlags = motionFlags;
	const Actors::C_SplineActor* pSplineActor = 
		Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));
	
	if(pSplineActor)
	{
		const F32 length = pSplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();
		m_AnimateDuration = length / moveSpeed;
	}
	else
	{
		LOGERROR(Actor::GetDebugLogLevelDesign(GetActor()), DBSTR"Spline Movement component %s has invalid spline actor", GetActor()->GetActorName());
	}
	
	SetIsAnimating(true);
}

bool C_MovementSpline::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	PERF_EVENT("C_MovementSpline::MessageHandler");
	switch (pMessage->Type())
	{
		case ActorMsg::E_MovementMsg_Spline:
		{
			const ActorMsg::C_ActorMovementSplineMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementSplineMsg>(pMessage);
			m_NotificationHandle = msg->Notifier();
			m_Loop = msg->m_Loop;
			MoveAlongSpline(
				msg->m_ActorHandle,
				msg->m_MoveSpeed,
				Maths::DegToRad(msg->m_Rotation),
				ActorMsg::TMotionTypeFlags(msg->m_MovementFlags));
			break;
		}
			
	default:
		return C_MovementBase::OnMessage(pMessage);
		
	}

	return true;
}

void C_MovementSpline::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_SplineActorHandle;
	stream << m_Rotation;
	stream << m_MoveSpeed;	
	stream << m_ArcLengthProgression;
	stream << m_Loop;
	stream << m_StartOffset;
}

void C_MovementSpline::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_SplineActorHandle;
	stream >> m_Rotation;
	stream >> m_MoveSpeed;	
	stream >> m_ArcLengthProgression;
	stream >> m_Loop;
	stream >> m_StartOffset;
}

void C_MovementSpline::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementSpline::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementSpline::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementSpline::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Deserialise(stream);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//Spline aim movement component class.
//
C_MovementAimSpline::C_MovementAimSpline() : C_MovementSpline()
{
}

bool C_MovementAimSpline::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
		case ActorMsg::E_MovementMsg_AimAt:
		{
			const ActorMsg::C_ActorMovementAimAtMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementAimAtMsg>(pMessage);
			m_AimPos = msg->GetPosition();
			m_TrackingSpeed = 1.0f;
			m_LocalOffset = false;
			m_UseRetrackingSpeed = false;
		}
			break;
		case ActorMsg::E_MovementMsg_AimBetween:
		{

		}
			break;
		case ActorMsg::E_MovementMsg_Track:
		{
			const ActorMsg::C_ActorMovementTrackMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementTrackMsg>(pMessage);
			m_TargetHandle = msg->m_ActorHandle;
			m_IsAligned = false;
			m_OffsetPos = msg->GetOffsetPosition();
			m_LocalOffset = msg->m_LocalOffset;
			m_FutureTime = 0.0f;
			if (msg->m_UseRetrackingSpeed)
			{
				m_RetrackingSpeed = msg->m_RetrackingSpeed;
				m_UseRetrackingSpeed = true;
				m_InitialLock = false;

			}
			if (msg->m_ReplaceDefaultSpeed)
			{
				m_TrackingSpeed = msg->m_RetrackingSpeed;
			}
		}
			break;
		case ActorMsg::E_MovementMsg_TrackPrediction:
			{
				const ActorMsg::C_ActorMovementTrackPredictionMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementTrackPredictionMsg>(pMessage);
				m_TargetHandle = msg->m_ActorHandle;
				m_IsAligned = false;
				m_OffsetPos = msg->GetOffsetPosition();
				m_LocalOffset = msg->m_LocalOffset;
				m_FutureTime = msg->m_Time;
			}
			break;

		// does nothing but stop an error
		case ActorMsg::E_MovementNotificationMsg_AimerAligned:
		case ActorMsg::E_MovementNotificationMsg_AimerLostAlignment:
			break;

		default:
			return C_MovementSpline::OnMessage(pMessage);
	}

	return true;
}

void C_MovementAimSpline::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementSpline::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementAimSpline::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementSpline::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementAimSpline::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementSpline::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementAimSpline::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementSpline::DeserialiseForSync(stream);
	Deserialise(stream);
}

//void C_MovementAimSpline::Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, bool recalcTransform)
void C_MovementAimSpline::Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, const E_MovementTransform transform)
{
	const Actor::C_Actor* target = GetActorList()->GetActorPtr(m_TargetHandle);
	Maths::HVec3 targetPos = m_AimPos;

	if (target)
	{

		Maths::HVec3 pos = target->GetPosition();

		// This was changed because actor handles might be Mercury actors
		const Components::I_MotionPredictor* pMotionPredictor = target->QueryInterface<Components::I_MotionPredictor>();

		if (pMotionPredictor != NULL)
		{
			pos = pMotionPredictor->CalcFuturePosition(m_FutureTime);
		}

		Maths::HVec3 offset;
		m_LocalOffset ?  offset = m_OffsetPos * target->GetMatrix().Rotation() : offset = m_OffsetPos;

		targetPos = offset + pos;
	}
// 	else 
// 	{
// 		const Maths::AxisZ az = (m_AimPos - WorldTransform().Translation()).Normalised().Normal();
// 		const Maths::HMat33 aimRot(az);
// 		Maths::C_Quaternion targetQuatWs(aimRot);
// 		// Calc target quat in local space:
// 		m_TargetQuatLs = !(m_ParentOffset.Quaternion() * parentWorldTransform.Quaternion()) * targetQuatWs;
// 	}
// 
	const Maths::AxisZ az = (targetPos - WorldTransform().Translation()).Normalised().Normal();
	const Maths::AxisY ay = Maths::C_Normal(0.0f,1.0f,0.0f);	//[MarkC] Can we have a constant for this please?
	const Maths::HMat33 aimRot(az, ay);
	Maths::C_Quaternion targetQuatWs(aimRot);
	// Calc target quat in local space:
	m_TargetQuatLs = !(m_ParentOffset.Quaternion() * parentWorldTransform.Quaternion()) * targetQuatWs;

	C_MovementSpline::Update(timeStep, parentWorldTransform, transform);
}


void C_MovementAimSpline::Animate(const Maths::C_TimeS& timeStep)
{
	//Spline movement
	{
		const Actors::C_SplineActor* pSplineActor = 
			Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));
		
		m_AnimateTime = Min(m_AnimateDuration.m_Scalar, m_AnimateTime.m_Scalar + timeStep.m_Scalar);

		if (pSplineActor)
		{
			if (m_Loop && pSplineActor->GetSplineWrapper()->IsClosed())
			{
				if (m_AnimateTime > m_AnimateDuration)
					m_AnimateTime -= m_AnimateDuration;
			}

			if (m_AnimateTime < m_AnimateDuration)
			{
				const F32 frac = CalcInterpolationFraction(
					m_AnimateTime.Get(), 
					m_AnimateDuration.Get(), 
					m_MotionFlags);

				m_ArcLengthProgression = pSplineActor->GetSplineWrapper()->GetSplinePos(
					m_LocalTransform.m_Translation,
					frac, 
					m_StartOffset,
					m_MoveSpeed, 
					timeStep.Get());
			}
			else
			{
				//Note that if our spline is closed we won't get here so that we can seamlessly loop.
				SetIsAnimating(false);
				if (m_Loop)
				{
					MoveAlongSpline(
						m_SplineActorHandle, 
						m_MoveSpeed,
						m_Rotation, 
						m_MotionFlags);
				}
			}
		}
	}

	//Aimer
	{
		const Actor::C_Actor* target = GetActorList()->GetActorPtr(m_TargetHandle);
		if (target)
		{
			F32 trackSpeed = m_TrackingSpeed;

			if (m_InitialLock==false&& m_UseRetrackingSpeed)
			{
				trackSpeed = m_RetrackingSpeed;
			}
			m_LocalTransform.Quaternion(m_LocalTransform.Quaternion().ShortestSlerp(m_TargetQuatLs, trackSpeed));
			m_LocalTransform.m_Quaternion.Normalise();

			// Notify actor of alignment events:
			const F32 diff = fabsf(m_LocalTransform.Quaternion() | m_TargetQuatLs);
			const bool alignedToTgt = fabsf(diff-1.0f) < 0.0001f;
			if (alignedToTgt)
			{
				if (!m_IsAligned)
				{
					const ActorMsg::C_ActorMovementNotificationAimerAligned aimerAlignedMsg(m_TargetHandle);
					SendMessageToActor(&aimerAlignedMsg);
					m_InitialLock = true;
				}
				m_IsAligned = true;
				
			}
			else
			{
				if (m_IsAligned)
				{
					const ActorMsg::C_ActorMovementNotificationAimerLostAlignment aimerLostAlignmentMsg(m_TargetHandle);
					SendMessageToActor(&aimerLostAlignmentMsg);
				}
				m_IsAligned = false;
			}
			
		}
		else
		{
			m_LocalTransform.Quaternion(m_TargetQuatLs);
			m_LocalTransform.m_Quaternion.Normalise();
		}

		// allow the body to be rotated by a specified amount
		const Maths::HVec3 axis(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY());
		const Maths::C_Quaternion rotQuat(axis, m_Rotation);
		const Maths::C_Quaternion newQuat = 
			Maths::C_Quaternion(m_LocalTransform.Quaternion() * rotQuat).Normalised();
		m_LocalTransform.Quaternion(newQuat);
	}
}

bool C_MovementAimSpline::InitialiseImpl()
{
	bool success = C_MovementSpline::InitialiseImpl();
	if (!success) return success;

	m_IsAligned = false;
	m_LocalOffset = false;

	if (GetCreationData()->m_pTargetActor)
		m_TargetHandle = GetCreationData()->m_pTargetActor->GetActorHandle();

	//If we've a valid handle here then potentially we're not driven by a message script.
	if (m_TargetHandle)
	{
		m_TrackingSpeed = 1.f;
		m_UseRetrackingSpeed = false;
	}
	else
	{
		m_TrackingSpeed = Maths::Clamped(GetCreationData()->m_TrackingSpeed, 0.0f, 1.0f);
	}
	
	return true;
}

void C_MovementAimSpline::Serialise(Sys::C_ContextualStream& stream) const
{
	C_MovementSpline::Serialise(stream);

	stream << m_TargetHandle;
	stream << m_TrackingSpeed;
	stream << m_TargetQuatLs;
	stream << m_OffsetPos;
	stream << m_LocalOffset;
	stream << m_IsAligned;
	stream << m_FutureTime;
	stream << m_UseRetrackingSpeed;
	stream << m_RetrackingSpeed;
	stream << m_InitialLock;
	stream << m_AimPos;
}

void C_MovementAimSpline::Deserialise(Sys::C_ContextualStream& stream)
{
	C_MovementSpline::Deserialise(stream);

	stream >> m_TargetHandle;
	stream >> m_TrackingSpeed;
	stream >> m_TargetQuatLs;	
	stream >> m_OffsetPos;
	stream >> m_LocalOffset;
	stream >> m_IsAligned;
	stream >> m_FutureTime;
	stream >> m_UseRetrackingSpeed;
	stream >> m_RetrackingSpeed;
	stream >> m_InitialLock;
	stream >> m_AimPos;
}

}

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif
