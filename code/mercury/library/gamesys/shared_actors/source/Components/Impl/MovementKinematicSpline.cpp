#include "SharedActorsPch.h"
#include <Shared_Actors/Components/Impl/MovementKinematicSpline.h>

#include <btl\Containers\algorithm.h>

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
#include <shared_actors/Components/Interfaces/I_PhysicsBodyComponents.h>
#include <Shared_Actors/ActorImpl/SplineActor.h>
#include <Shared_Actors/TypeWrappers/SplineVec3Wrapper.h>

#include <actor_system/ActorDebug.h>
#include <actor_system/ActorList.h>
#include <actor_system/Actor.h>


#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

Debug::Bool g_MovementKinematicSplineDebugRender("ActorDebug.Components.Movement.KinematicSpline.DebugRenderEnable", false);
Debug::Bool g_MovementKinematicSplineTrackerDebugRender("ActorDebug.Components.Movement.KinematicSplineTracker.DebugRenderEnable", false);
Debug::Float g_MovementKinematicSplineReactionScale("ActorDebug.Components.Movement.KinematicSplineTracker.ReactionScale", 2.0f, Debug::Range(0.0f, 30.0f));

namespace Components
{

///////////////////////////////////////////////////////////////////////////////////////////////////
//Kinematic Spline.
//
bool C_MovementKinematicSpline::InitialiseImpl()
{
	m_ArcLengthProgression = 0.0f;
	m_Loop = false;
	m_StartOffset = 0.0f;
	m_StartSpeed = m_Velocity = GetCreationData()->m_StartSpeed;
	m_MinSpeed = GetCreationData()->m_MinSpeed;
	m_MaxSpeed = GetCreationData()->m_MaxSpeed;
	m_Loop = GetCreationData()->m_Loop;
	m_MaxAcceleration = m_Acceleration = GetCreationData()->m_Acceleration;
	m_BrakeAtEnd = GetCreationData()->m_BrakeAtEnd;
	m_MinArcProgression = 0.0f;
	
	if (GetCreationData()->m_pSplineActorCreate)
	{
		m_SplineActorHandle = GetCreationData()->m_pSplineActorCreate->GetActorHandle();

	}

	return C_MovementBase::InitialiseImpl();
}

bool C_MovementKinematicSpline::CreateImpl()
{
	if (m_SplineActorHandle)
	{
		const Actors::C_SplineActor* pSplineActor = 
			Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));
		if(pSplineActor)
			SetInitialPosition(m_SplineActorHandle, GetCreationData()->m_StartFromNearestPoint);
	}

	return C_MovementBase::CreateImpl();
}

void C_MovementKinematicSpline::ExternalDependenciesCreatedImpl()
{
	C_MovementBase::ExternalDependenciesCreatedImpl();

	if( GetCreationData()->m_StartImmediately)
	{
		const Actors::C_SplineActor* pSplineActor = 
			Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));
		if(pSplineActor)
		{
			SetInitialPosition(m_SplineActorHandle, GetCreationData()->m_StartFromNearestPoint);
			MoveAlongSpline(m_SplineActorHandle, m_StartSpeed, m_MinSpeed, m_MaxSpeed, m_MaxAcceleration);
		}
	}
}

void C_MovementKinematicSpline::Animate(const Maths::C_TimeS& timeStep)
{
	const Actors::C_SplineActor* pSplineActor = 
		Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));
	
	if (!pSplineActor)
		return;

	F32 oldVel = m_Velocity;

	UpdateEndOfSplineBraking(pSplineActor);
	

	// calculate new speed
	m_Velocity = m_Velocity + (m_Acceleration * timeStep.Get());
	m_Velocity = Maths::Clamp(m_Velocity, m_MinSpeed, m_MaxSpeed);

	// displacement 
	
	F32 t = timeStep.Get();
	F32 disp;
	

	if (fabs(m_Velocity) < FLT_EPSILON )
		disp = 0.0f;
	else
		disp = (oldVel*t) + (0.5f*m_Acceleration*t*t);
	

	UpdateSplineProgression(pSplineActor, disp);
	UpdateSlaves(timeStep);

	
}

void C_MovementKinematicSpline::UpdateSlaves(const Maths::C_TimeS& timeStep)
{
	const int count = m_Slaves.Size();
	if (count > 0)
	{
		ActorMsg::C_ActorMovementUpdateSlaveMsg msg(timeStep.Get(), m_ArcLengthProgression);
		for (int c=0 ; c<count ; c++)
		{
			SendMessageToActor(m_Slaves[c], &msg);
		}
	}
}

bool C_MovementKinematicSpline::UpdateEndOfSplineBraking(const Actors::C_SplineActor* pSpline)
{
	if (m_BrakeAtEnd)
	{
		const F32 length = pSpline->GetSplineWrapper()->GetCurveEval().GetArcLength();
		F32 vel = fabs(m_Velocity);
		
		if (vel > 0.1f)
		{
			F32 remainingDist;
			m_Velocity > 0.0f ? remainingDist = length - m_ArcLengthProgression : remainingDist = m_ArcLengthProgression - m_MinArcProgression;
			F32 timeToEnd = remainingDist / vel;
			F32 bestTimeToStop = m_Velocity / (2.0f*m_MaxAcceleration);
			
			if (timeToEnd < bestTimeToStop)
			{
				m_Acceleration = -(2.0f*m_MaxAcceleration);
				return true;
			}
			else
			{
				m_Acceleration = m_MaxAcceleration;

			}
		}

	}
	return false;
}

void C_MovementKinematicSpline::UpdateSplineProgression(const Actors::C_SplineActor* pSpline, F32 displacement)
{
	m_ArcLengthProgression += displacement;
	const F32 length = pSpline->GetSplineWrapper()->GetCurveEval().GetArcLength();
	if (m_ArcLengthProgression < length && m_ArcLengthProgression > m_MinArcProgression)
	{
		pSpline->GetSplineWrapper()->GetForwardFacingSplinePos(m_LocalTransform, m_ArcLengthProgression);
	}
	else
	{
		m_ArcLengthProgression > length ? m_ArcLengthProgression = length : m_ArcLengthProgression = m_MinArcProgression;
		pSpline->GetSplineWrapper()->GetForwardFacingSplinePos(m_LocalTransform, m_ArcLengthProgression);
		
		SetIsAnimating(false);
		if (m_Loop)
		{
			m_ArcLengthProgression = 0.0f;
			MoveAlongSpline(m_SplineActorHandle, m_StartSpeed, m_MinSpeed, m_MaxSpeed, m_MaxAcceleration);
		}
	}

	
}

void C_MovementKinematicSpline::SetInitialPosition(Actor::C_ActorHandle splineHandle, bool moveToNearest)
{
	m_ArcLengthProgression = 0.0f;
	const Actors::C_SplineActor* pSplineActor = 
		Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(splineHandle));
	if (!pSplineActor)
		return;

	if (moveToNearest)
	{
		Maths::HVec3 closestPos;
		const F32 length = pSplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();

		m_StartOffset = Curve::Actor::FindNearestPoint(
			*pSplineActor->GetSplineWrapper()->GetCurveEval().GetCurve(),
			pSplineActor->GetSplineWrapper()->GetCurveEval().GetReparam(),
			m_WorldTransform.Translation(),
			closestPos);
		m_ArcLengthProgression = m_StartOffset;
		pSplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(m_WorldTransform, m_ArcLengthProgression);
				
		const int count = m_Slaves.Size();
		if (count > 0)
		{
			ActorMsg::C_ActorMovementSetSlaveMasterOffset msg;
			msg.SetValue(m_ArcLengthProgression);
			for (int c=0 ; c<count ; c++)
			{
				SendMessageToActor(m_Slaves[c], &msg);
			}
		}
	}

	
	
}

void C_MovementKinematicSpline::MoveAlongSpline(Actor::C_ActorHandle splineHandle, F32 startSpeed, F32 minSpeed, F32 maxSpeed, F32 accel)
{
	m_ParentOffset = Maths::C_QuatTrans();
	m_SplineActorHandle = splineHandle;
	
	m_MinSpeed = minSpeed;
	m_StartSpeed = m_Velocity = startSpeed;
	m_MaxSpeed = maxSpeed;
	m_MaxAcceleration = m_Acceleration = accel;
		
	const Actors::C_SplineActor* pSplineActor = 
		Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));

	if(pSplineActor)
	{
		SetIsAnimating(true);	
	}
	else
	{
		LOGERROR(Actor::GetDebugLogLevelDesign(GetActor()), DBSTR"Spline Movement component %s has invalid spline actor", GetActor()->GetActorName());
	}
}



bool C_MovementKinematicSpline::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
		case ActorMsg::E_MovementMsg_KinematicSpline:
		{
			const ActorMsg::C_ActorMovementKinematicSplineMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementKinematicSplineMsg>(pMessage);
			m_NotificationHandle = msg->Notifier();
			m_Loop = msg->m_Loop;
			m_BrakeAtEnd = msg->m_BrakeAtEnd;
			SetInitialPosition(msg->m_ActorHandle, msg->m_StartFromNearestPosition);
			MoveAlongSpline(msg->m_ActorHandle, msg->m_StartSpeed, msg->m_MinSpeed, msg->m_MaxSpeed, msg->m_Acceleration);
			break;
			
		}
		case ActorMsg::E_MovementMsg_KinematicSplineCommand:
		{
			const ActorMsg::C_ActorMovementKinematicSplineCommandMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementKinematicSplineCommandMsg>(pMessage);
			m_MinSpeed = msg->m_MinSpeed;
			m_MaxSpeed = msg->m_MaxSpeed;
			m_MaxAcceleration = msg->m_Acceleration;
			break;
		}

		case ActorMsg::E_MovementMsg_AddSplineSlave:
		{
			const ActorMsg::C_ActorMovementAddSplineSlaveMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementAddSplineSlaveMsg>(pMessage);
			assert(Btl::Find(m_Slaves, msg->GetActorHandle()) == m_Slaves.End());	// Already in list!
			m_Slaves.PushBack(msg->GetActorHandle());
			break;
		}

		case ActorMsg::E_MovementMsg_RemoveSplineSlave:
		{
			const ActorMsg::C_ActorMovementRemoveSplineSlaveMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementRemoveSplineSlaveMsg>(pMessage);
			Btl::Vector<Actor::C_ActorHandle>::Iterator it = Btl::Find(m_Slaves, msg->GetActorHandle());
			if (it != m_Slaves.End())
				m_Slaves.Erase(it);
			break;
		}

		case ActorMsg::E_MovementMsg_SetLastSlaveOffset:
		{
			m_MinArcProgression = Messaging::MessageCast<ActorMsg::C_ActorMovementSetLastSlaveOffset>(pMessage)->GetValue();
			break;
		}
			
	default:
		return C_MovementBase::OnMessage(pMessage);
		
	}

	return true;
}

void C_MovementKinematicSpline::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_SplineActorHandle;
	stream << m_MaxAcceleration;		
	stream << m_Acceleration;		
	stream << m_MinSpeed;
	stream << m_MaxSpeed;
	stream << m_StartSpeed;
	stream << m_Velocity;
	stream << m_ArcLengthProgression;
	stream << m_MinArcProgression;
	stream << m_StartOffset;
	stream << m_BrakeAtEnd;
	stream << m_Loop;
}

void C_MovementKinematicSpline::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_SplineActorHandle;
	stream >> m_MaxAcceleration;		
	stream >> m_Acceleration;		
	stream >> m_MinSpeed;
	stream >> m_MaxSpeed;
	stream >> m_StartSpeed;
	stream >> m_Velocity;
	stream >> m_ArcLengthProgression;
	stream >> m_MinArcProgression;
	stream >> m_StartOffset;
	stream >> m_BrakeAtEnd;
	stream >> m_Loop;
}

void C_MovementKinematicSpline::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementKinematicSpline::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementKinematicSpline::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementKinematicSpline::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Deserialise(stream);
}

#ifndef CFG_GOLD
void C_MovementKinematicSpline::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!g_MovementKinematicSplineDebugRender)
		return;

	
	Util::C_String text;
	
	text.Printf("vel%0.1f acc%0.1f ", m_Velocity, m_Acceleration);
	Maths::HVec3 pos = GetActor()->GetPosition();
	debugRenderPtr->Draw3dText(pos + Maths::HVec3(0.0f, 2.0f, 0.0f), 0.50f, text);

	
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//Kinematic Spline. Tracker
//
bool C_MovementKinematicSplineTracker::InitialiseImpl()
{
	return C_MovementKinematicSpline::InitialiseImpl();
}

bool C_MovementKinematicSplineTracker::CreateImpl()
{
	return C_MovementKinematicSpline::CreateImpl();
}


void C_MovementKinematicSplineTracker::Animate(const Maths::C_TimeS& timeStep)
{
	const Actors::C_SplineActor* pSplineActor = 
		Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));

	if (!pSplineActor)
		return;

	F32 oldVel = m_Velocity;
	
	if (!UpdateEndOfSplineBraking(pSplineActor))
	{
		// only track if we are not near the end of spline and need to brake.
		const Actor::C_Actor* pTrackerActor = GetActorList()->GetActorPtr(m_TrackerActorHandle);
		if (pTrackerActor)
		{
			
			m_TrackerNearestIndex = Curve::Actor::FollowerFindNearestReparamIndexForwardsAndBackwards(*pSplineActor->GetSplineWrapper()->GetCurveEval().GetCurve(),
																				pSplineActor->GetSplineWrapper()->GetCurveEval().GetReparam(),	
																					pTrackerActor->GetPosition(), m_TrackerNearestIndex);
			F32 trackerPos = Curve::Actor::FollowerGetArcDistance(*pSplineActor->GetSplineWrapper()->GetCurveEval().GetCurve(),
																	pSplineActor->GetSplineWrapper()->GetCurveEval().GetReparam(),	
																		pTrackerActor->GetPosition(), m_TrackerNearestIndex);

			

			m_TrackerActorPos = trackerPos;

			F32 relPos = m_ArcLengthProgression - trackerPos;
			F32 relDiff = m_IdealDistance - relPos;

			// if the tracker has a body then use it's velocity to improve the tracking ability

			const I_PhysicsBody* pBody = pTrackerActor->QueryInterface<I_PhysicsBody>();
			if (pBody)
			{
				Maths::HVec3 linearVel = pBody->LinearVelocity();
				linearVel = linearVel * GetActor()->GetMatrix().InverseRotTrans().Rotation();
				F32 localSpeed = linearVel.Z();
				m_TrackerVel = localSpeed;
			}
			else 
			{
				m_TrackerVel = 0.0f;
			}

			// relative velocity in spline space;
			F32 relVel = m_Velocity - m_TrackerVel;

			// how far away can we react to change
			F32 reactionDistance = (fabs(relVel) / m_MaxAcceleration) * g_MovementKinematicSplineReactionScale;
			m_ReactionDistance = reactionDistance;

			// outside the reaction range then either full accel or brake applied
			if (relDiff > reactionDistance)
			{
				m_Acceleration = m_MaxAcceleration;
			}
			else if (relDiff < -reactionDistance)
			{
				m_Acceleration = -m_MaxAcceleration;

			}
			else 
			{
				if (pBody)
				{
					if (m_TrackerVel > m_Velocity)
						m_Acceleration = m_MaxAcceleration*0.5f;
					else
						m_Acceleration = -m_MaxAcceleration*0.5f;
				}
				else
				{
					// for Actors that don't have a body then just attempt to keep position ideal.
					m_TrackerVel = 0.0f;

					// are we heading towards ideal position
					if (Maths::Sign(m_Velocity) == Maths::Sign(relDiff))
					{
						// can we stop in time
						F32 timeToIdealPos = relDiff / m_Velocity;
						F32 bestTimeToStop = fabs(m_Velocity) / (2.0f*m_MaxAcceleration);

						if (timeToIdealPos < bestTimeToStop)
						{
							m_Acceleration = Maths::Sign(relDiff) * -(2.0f*m_MaxAcceleration);
							
						}
						else 
						{
							// move towards slowly
							m_Acceleration = Maths::Sign(relDiff) * 0.5f * m_MaxAcceleration;
						}

					}
					else
					{
						// heading away
						if (relDiff > 0)
						{
							m_Acceleration = m_MaxAcceleration;
						}
						else
						{
							if (m_Velocity > m_MinSpeed)
							{
								
								m_Acceleration = -m_MaxAcceleration;
							}
							else 
							{
								m_Acceleration = 0.0f;
							}
						}
					}

					

					
					
				}

			}

		}

		// calculate new speed
		m_Velocity = m_Velocity + (m_Acceleration * timeStep.Get());
		m_Velocity = Maths::Clamp(m_Velocity, m_MinSpeed, m_MaxSpeed);


	}
	else
	{
		// calculate new speed
		m_Velocity = m_Velocity + (m_Acceleration * timeStep.Get());
		m_Velocity = Maths::Clamp(m_Velocity, m_MinSpeed, m_MaxSpeed);

	}
	
	
	// displacement 
	//F32 disp = m_Velocity * timeStep.Get();
	F32 t = timeStep.Get();
	F32 disp = (oldVel*t) + (0.5f*m_Acceleration*t*t);

	if (m_ArcLengthProgression + disp > 0.0f)
	{
		UpdateSplineProgression(pSplineActor, disp);
		UpdateSlaves(timeStep);
	}


}





bool C_MovementKinematicSplineTracker::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
	case ActorMsg::E_MovementMsg_KinematicSplineTracker:
		{
			const ActorMsg::C_ActorMovementKinematicSplineTrackerMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementKinematicSplineTrackerMsg>(pMessage);
			m_NotificationHandle = msg->Notifier();
			m_TrackerActorHandle = msg->m_TrackerActorHandle;
			m_MinSpeed = msg->m_MinSpeed;
			m_MaxSpeed = msg->m_MaxSpeed;
			m_MaxAcceleration = msg->m_Acceleration;
			m_IdealDistance = msg->m_IdealDistance;
			break;

		}

	default:
		return C_MovementKinematicSpline::OnMessage(pMessage);

	}

	return true;
}

void C_MovementKinematicSplineTracker::Serialise(Sys::C_ContextualStream& stream) const
{
	C_MovementKinematicSpline::Serialise(stream);
	stream << m_TrackerActorHandle;
	stream << m_IdealDistance;
	stream << m_TrackerActorPos;
	stream << m_TrackerNearestIndex;
	
}

void C_MovementKinematicSplineTracker::Deserialise(Sys::C_ContextualStream& stream)
{
	C_MovementKinematicSpline::Deserialise(stream);
	stream >> m_TrackerActorHandle;
	stream >> m_IdealDistance;
	stream >> m_TrackerActorPos;
	stream >> m_TrackerNearestIndex;
}

void C_MovementKinematicSplineTracker::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementKinematicSpline::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementKinematicSplineTracker::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementKinematicSpline::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementKinematicSplineTracker::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementKinematicSpline::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementKinematicSplineTracker::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementKinematicSpline::DeserialiseForSync(stream);
	Deserialise(stream);
}

#ifndef CFG_GOLD
void C_MovementKinematicSplineTracker::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!g_MovementKinematicSplineTrackerDebugRender)
		return;

	const Actors::C_SplineActor* pSplineActor = 
		Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));

	if (!pSplineActor)
		return;

	C_MovementKinematicSpline::DebugRender(debugRenderPtr);

	if (m_TrackerActorHandle)
	{
		Maths::HVec3 trackerPos = pSplineActor->GetSplineWrapper()->GetCurveEval().EvaluateArcLength(m_TrackerActorPos);
		debugRenderPtr->Draw3dPoint(trackerPos, 1.0f, Maths::RGBA(0xff, 0xff, 0, 0xff));
		Util::C_String text;
		F32 relDist = m_ArcLengthProgression - m_TrackerActorPos;
		text.Printf("rel%0.1f des%0.1f\n", relDist, m_IdealDistance);
		text += Util::C_String().Printf("vel%0.1f rd%0.1f", m_TrackerVel, m_ReactionDistance);
		debugRenderPtr->Draw3dText(trackerPos+ Maths::HVec3(0.0f, 0.5f, 0.0f), 0.50f, text);
	}
	

	
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////
//Kinematic Spline Slave.
//
bool C_MovementKinematicSplineSlave::InitialiseImpl()
{
	m_ArcLengthProgression = 0.0f;
	m_StartOffset = 0.0f;
	m_SlaveMasterOffset = 0.0f;

	if (GetCreationData()->m_pSplineActorCreate)
	{
		m_SplineActorHandle = GetCreationData()->m_pSplineActorCreate->GetActorHandle();
	}

	if (GetCreationData()->m_pSlaveMaster)
	{
		m_SlaveMasterHandle = GetCreationData()->m_pSlaveMaster->GetActorHandle();
	}



	return C_MovementBase::InitialiseImpl();
}

bool C_MovementKinematicSplineSlave::CreateImpl()
{
	if (m_SplineActorHandle && m_SlaveMasterHandle)
	{
		ActorMsg::C_ActorMovementAddSplineSlaveMsg msg;
		msg.SetActorHandle(GetActorHandle());
		SendMessageToActor(m_SlaveMasterHandle, &msg);
	}
	else
	{
		return false;
	}
	

	return C_MovementBase::CreateImpl();
}

void C_MovementKinematicSplineSlave::SetInitialPosition(Actor::C_ActorHandle splineHandle, F32 slaveMasterPosition)
{
	m_ArcLengthProgression = 0.0f;
	m_SlaveMasterOffset = 0.0f;
	m_ParentOffset = Maths::C_QuatTrans();
	const Actors::C_SplineActor* pSplineActor = 
		Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(splineHandle));
	if (!pSplineActor)
		return;
	
	{
		Maths::HVec3 closestPos;
		const F32 length = pSplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();

		m_StartOffset = Curve::Actor::FindNearestPoint(
			*pSplineActor->GetSplineWrapper()->GetCurveEval().GetCurve(),
			pSplineActor->GetSplineWrapper()->GetCurveEval().GetReparam(),
			m_WorldTransform.Translation(),
			closestPos);
		m_ArcLengthProgression = m_StartOffset;
		pSplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(m_WorldTransform, m_ArcLengthProgression);

		m_SlaveMasterOffset = slaveMasterPosition - m_ArcLengthProgression;

		if (GetCreationData()->m_LastSlave)
		{
			ActorMsg::C_ActorMovementSetLastSlaveOffset msg;
			// add on half a metre for safety
			msg.SetValue(m_SlaveMasterOffset+0.5f);
			SendMessageToActor(m_SlaveMasterHandle, &msg);
		}
		
	}
}

void C_MovementKinematicSplineSlave::Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, const E_MovementTransform transform)
{
	// do nothing as we get a slave update
}

void C_MovementKinematicSplineSlave::SlaveUpdate(F32 timeStep, F32 slaveMasterPosition)
{
	m_ArcLengthProgression = slaveMasterPosition - m_SlaveMasterOffset;

	const Actors::C_SplineActor* pSplineActor = 
		Actor::ActorCast_Safe<Actors::C_SplineActor>(GetActorList()->GetActorPtr(m_SplineActorHandle));
	if (!pSplineActor)
		return;

	const F32 length = pSplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();
	if (m_ArcLengthProgression > 0.0f)
	{
		pSplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(m_LocalTransform, m_ArcLengthProgression);
	}
	else
	{
		F32 prog = length - m_ArcLengthProgression;
		pSplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(m_LocalTransform, prog);
	}

	m_WorldTransform = m_LocalTransform;
	Maths::C_TimeS time(timeStep);
	AnimateActorTransform(m_WorldTransform, time);

	ActorMsg::C_ActorMovementUpdateMsg msg(time, m_WorldTransform, E_MovementTransform_Update);
	const int count = m_Children.Size();
	for (int c=0 ; c<count ; c++)
	{
		SendMessageToActor(m_Children[c], &msg);
	}
}



bool C_MovementKinematicSplineSlave::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
		case ActorMsg::E_MovementMsg_SetSlaveMasterOffset:
		{
			SetInitialPosition(m_SplineActorHandle, Messaging::MessageCast<ActorMsg::C_ActorMovementSetSlaveMasterOffset>(pMessage)->GetValue());
			break;
		}

		case ActorMsg::E_MovementMsg_UpdateSlave:
		{
			const ActorMsg::C_ActorMovementUpdateSlaveMsg *pMsg = Messaging::MessageCast<ActorMsg::C_ActorMovementUpdateSlaveMsg>(pMessage);
			SlaveUpdate(pMsg->m_TimeStep, pMsg->m_SlaveMasterProgression);
			break;
		}

	default:
		return C_MovementBase::OnMessage(pMessage);

	}

	return true;
}

void C_MovementKinematicSplineSlave::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_SplineActorHandle;
	stream << m_SlaveMasterHandle;
	
}

void C_MovementKinematicSplineSlave::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_SplineActorHandle;
	stream >> m_SlaveMasterHandle;
}

void C_MovementKinematicSplineSlave::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementKinematicSplineSlave::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementKinematicSplineSlave::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementKinematicSplineSlave::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Deserialise(stream);
}

#ifndef CFG_GOLD
void C_MovementKinematicSplineSlave::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	
}
#endif

}

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif
