#include "SharedActorsPch.h"
#include <shared_actors/Components/Impl/MovementComponents.h>

#include <btl\containers\algorithm.h>
#include <display.h>

#include <Display.h>	// To make it compile

//GCC wants these includes here, TODO: sort include Hell.
#include <Gravity/NarrowPhase/Primitive.h>
#include <Gravity/NarrowPhase/CollisionResult.h>
#include <Gravity/NarrowPhase/PrimitiveTest.h>
#include <Gravity/NarrowPhase/Primitive.inl>

#include <actor_system/Actor.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/ActorList.h>
#include <animation/pose/PoseFormat.h>
#include <animation/pose/SkeletonPose.h>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>
#include <animation/skeleton/Skeleton.h>
#include <animation/task/EvaluationState.h>
#include <misc_shared/messaging/GameMessaging.h>
#include <misc_shared/messaging/MessageTypeList.h>
#include <shared_actors/Components/Messages/AnimationMessages.h>
#include <shared_actors/Components/Interfaces/I_AnimationController.h>
#include <shared_actors/Components/Interfaces/I_AnimatedObjectComponents.h>
#include <shared_actors/Components/Interfaces/I_MotionPredictor.h>
#include <system/Perf.h>

#include <framework/application/ApplicationBase.h>

Debug::Bool g_MovementDebugRender("ActorDebug.Components.Movement.DebugRender.Enable", false);

// Helper fns:


NOALIAS __forceinline const F32 WrapAngle( const F32 angleRadians )
{
	const F32 ar_div_2PI = angleRadians * Maths::Rcp2Pi;
	return ( ar_div_2PI - Maths::Floor( ar_div_2PI ) ) * Maths::Pi2;
}

NOALIAS __forceinline const Maths::HF32 WrapAngle( const Maths::HF32 PCREF angleRadians )
{
	const Maths::HF32 ar_div_2PI = angleRadians * Maths::Load<Maths::HF32>( &Maths::Rcp2Pi );
	return ( ar_div_2PI - RoundToIntMinusInf( ar_div_2PI ) ) * Maths::Load<Maths::HF32>( &Maths::Pi2 );
}


namespace Components
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Movement component base.
//
C_MovementBase::C_MovementBase()
	: m_IsAnimating(false),
	m_MovementCount(0),
	m_IsEnabled(true),
	m_DisableSlowMotion(false),
	m_MotionFlags(ActorMsg::E_MovementFlag_Smooth),
	m_AnimateTime(0),
	m_AnimateDuration(0),
	m_LinearVelocity(Maths::ZERO_INIT)
{
}


bool C_MovementBase::InitialiseImpl()
{
	if(!Actor::I_ActorComponent::InitialiseImpl())
		return false;

	if (GetCreationData()->IsParented())
	{
		if (GetActorList()->ActorExists(GetCreationData()->ParentHandle()))
			m_AttachHandle = GetCreationData()->ParentHandle();
		else
			LOGWARN(Actor::GetDebugLogGeneral(), DBSTR"movement component parent doesn't exist");
	}
	m_WorldTransform = Maths::C_QuatTrans(GetActor()->GetMatrix()); // World space until parent set.
	m_ParentOffset = m_WorldTransform;

	//printf("MovementBase init: worldPos = (%f, %f, %f)\n", (F32)m_WorldTransform.Translation().X(), (F32)m_WorldTransform.Translation().Y(), (F32)m_WorldTransform.Translation().Z());

	return true;
}


bool C_MovementBase::CreateImpl()
{
	if(!Actor::I_ActorComponent::CreateImpl())
		return false;

	return true;
}


bool C_MovementBase::HasValidParent() const
{
	return GetActorList()->GetActorPtr(m_ParentHandle)!=NULL;
}


F32 C_MovementBase::CalcInterpolationFraction(F32 v1, F32 v2, ActorMsg::TMotionTypeFlags motionFlags)
{
	BIZ_ASSERT(motionFlags != ActorMsg::E_MovementFlag_Invalid);

	if (v2==0 || Maths::Equal(v1, v2, FLT_EPSILON))
		return 1.0f;

	F32 frac = v1/v2;
	// Smooth interpolation fraction if required:
	if ((motionFlags & ActorMsg::E_MovementFlag_EaseIn) &&  frac<0.5f || (motionFlags & ActorMsg::E_MovementFlag_EaseOut) &&  frac>=0.5f)
		frac = F32((1 - cosf(frac*Maths::Pi)) * 0.5f);

	return frac;
}


ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(C_MovementBase)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_MovementMsg_Begin, ActorMsg::E_MovementMsg_End)
	ACTOR_COMPONENT_MESSAGE_TYPES_RANGE(ActorMsg::E_MovementNotificationMsg_Begin, ActorMsg::E_MovementNotificationMsg_End)
ACTOR_COMPONENT_MESSAGE_TYPES_END(C_MovementBase, Actor::I_ActorComponent)


bool C_MovementBase::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_MovementMsg_SetParent:
		{
			const ActorMsg::C_ActorMovementSetParentMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementSetParentMsg>(pMessage);
			SetParent(msg->GetActorHandle());
			break;
		}

	case ActorMsg::E_MovementMsg_ClearParent:
		ClearParent();
		break;

	case ActorMsg::E_MovementMsg_IncMovementCount:
		IncMovementCount();
		break;

	case ActorMsg::E_MovementMsg_DecMovementCount:
		DecMovementCount();
		break;

	case ActorMsg::E_MovementMsg_AddChild:
		{
			const ActorMsg::C_ActorMovementAddChildMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementAddChildMsg>(pMessage);
			const Actor::C_Actor *pActor = GetActorList()->GetActorPtr(msg->GetActorHandle());

			if(pActor)
			{
				const I_Movement *pChildMovementComp = (const I_Movement *)pActor->QueryInterface(E_IF_Movement);

				if(pChildMovementComp && pChildMovementComp->MovementCount() > 0)
					IncMovementCount();
			}

			//assert(Btl::Find(m_Children, msg->GetActorHandle()) == m_Children.End());	// Already in list!
			m_Children.PushBack(msg->GetActorHandle());

			break;
		}

	case ActorMsg::E_MovementMsg_RemoveChild:
		{
			const ActorMsg::C_ActorMovementRemoveChildMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementRemoveChildMsg>(pMessage);

			const Actor::C_Actor *pActor = GetActorList()->GetActorPtr(msg->GetActorHandle());

			if(pActor)
			{
				const I_Movement *pChildMovementComp = (const I_Movement *)pActor->QueryInterface(E_IF_Movement);

				if(pChildMovementComp && pChildMovementComp->MovementCount() > 0)
					DecMovementCount();
			}


			Btl::Vector<Actor::C_ActorHandle>::Iterator it = Btl::Find(m_Children, msg->GetActorHandle());
			if (it != m_Children.End())
				m_Children.Erase(it);
			break;
		}

	case ActorMsg::E_GameActorMsg_DisableSlowMotion:
		{
			m_DisableSlowMotion = Messaging::MessageCast<ActorMsg::C_DisableSlowMotionMsg>(pMessage)->GetValue();
		}
		break;

	case GameMessaging::E_GameMsg_Update:
		{
			const Actor::C_Actor* parent = GetActorList()->GetActorPtr(m_ParentHandle);
			Maths::C_QuatTrans parentWorldTransform;
			E_MovementTransform transform = E_MovementTransform_None;
			F32 timeStep = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
			// If getting E_GameMsg_Update and has a parent, then by implication the parent has no movement component.
			if (parent)
			{
				parentWorldTransform = Maths::C_QuatTrans(parent->GetMatrix());
				transform = E_MovementTransform_Update;
			}
			const ActorMsg::C_ActorMovementUpdateMsg moverMsg(timeStep, parentWorldTransform, transform);
			SendMessageToComponent(Components::E_IF_Movement, &moverMsg);

			if(m_MovementCount == 0)
				RemoveSubscriber(GameMessaging::E_GameMsg_Update);
			break;
		}

	case ActorMsg::E_MovementMsg_Update:
		{
			const ActorMsg::C_ActorMovementUpdateMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementUpdateMsg>(pMessage);
			Update(
				m_DisableSlowMotion ? (F32)Framework::C_ApplicationBase::GetInstance()->GetTimer().GetFrameTime() : (F32)msg->TimeStep().Get(), 
				msg->ParentWorldTransform(), msg->m_Transform
				);

			break;
		}

	case ActorMsg::E_MovementMsg_Enable:
		if((m_MovementCount > 0) && (GetActorList()->ActorExists(m_ParentHandle) == false))
			AddSubscriber(GameMessaging::E_GameMsg_Update);
		m_IsEnabled = true;
		break;

	case ActorMsg::E_MovementMsg_Disable:
		RemoveSubscriber(GameMessaging::E_GameMsg_Update);
		m_IsEnabled = false;
		break;

		// empty handler to stop warnings
	case ActorMsg::E_MovementNotificationMsg_MovementComplete:
		break;

	default:
		return Actor::I_ActorComponent::OnMessage(pMessage);
	}

	return true;
}


void C_MovementBase::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_LocalTransform;	// Current local animation transform.
	stream << m_ParentOffset;		// Offset relative to parent.  This is the object's local-to-world transform if no parent (i.e. world is parent).
	stream << m_WorldTransform;	// Current world transform.  Keep this here so we can have an animator without a body (e.g. for compositing animation types).

	stream << m_IsAnimating;
	stream << m_MovementCount;
	stream << m_IsEnabled;
	stream << m_DisableSlowMotion;
	stream << m_LinearVelocity;
	stream.WriteAs<U32>(m_MotionFlags);
	stream << m_AnimateTime.Get();			// Time spent animating so far.
	stream << m_AnimateDuration.Get();		// Target duration of animation.

	stream << m_NotificationHandle;	// Actor to notify that movement has been completed

	stream << m_ParentHandle;
	//stream << m_AttachHandle;	// Actor to be attached to on construction.
	stream << m_Children.Size();
	for(S32 childNo = 0;childNo < m_Children.Size(); childNo++)
	{
		stream << m_Children[childNo];
	}
}

void C_MovementBase::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_LocalTransform;	// Current local animation transform.
	stream >> m_ParentOffset;		// Offset relative to parent.  This is the object's local-to-world transform if no parent (i.e. world is parent).
	stream >> m_WorldTransform;	// Current world transform.  Keep this here so we can have an animator without a body (e.g. for compositing animation types).

	stream >> m_IsAnimating;
	stream >> m_MovementCount;
	stream >> m_IsEnabled;
	stream >> m_DisableSlowMotion;
	stream >> m_LinearVelocity;

	stream.ReadAs<U32>(m_MotionFlags);

	m_AnimateTime = stream.ReadAs<F32>();			// Time spent animating so far.
	m_AnimateDuration = stream.ReadAs<F32>();		// Target duration of animation.

	stream >> m_NotificationHandle;	// Actor to notify that movement has been completed

	stream >> m_ParentHandle;
	//stream >> m_AttachHandle;	// Actor to be attached to on construction.

	U32 noChildren;
	stream >> noChildren;
	for(U32 childNo = 0;childNo < noChildren; childNo++)
		m_Children.PushBack(stream.ReadAs<Actor::C_ActorHandle>());

	// if we have a movement count and no parent then we must update
	if((m_MovementCount > 0) && !GetActorList()->ActorExists(m_ParentHandle))
		AddSubscriber(GameMessaging::E_GameMsg_Update);

}

void C_MovementBase::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	Actor::I_ActorComponent::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementBase::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	Actor::I_ActorComponent::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementBase::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	Actor::I_ActorComponent::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementBase::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	Actor::I_ActorComponent::DeserialiseForSync(stream);
	Deserialise(stream);
	m_ParentHandle = m_AttachHandle;

	// Determine if we need to subscribe to update
	if(!GetActorList()->ActorExists(m_ParentHandle) && (m_MovementCount > 0) && m_IsEnabled)
		AddSubscriber(GameMessaging::E_GameMsg_Update);
	else
		RemoveSubscriber(GameMessaging::E_GameMsg_Update);

	//EnableSubscriber(GameMessaging::E_GameMsg_Update, m_IsEnabled);

}

void C_MovementBase::ExternalDependenciesCreatedImpl()
{
	I_Movement::ExternalDependenciesCreatedImpl();

 	if (GetActorList()->ActorExists(m_AttachHandle))
	{
		SetParent(m_AttachHandle);
		m_AttachHandle = Actor::C_ActorHandle();
	}
}

// Tell an object that it is moving
void C_MovementBase::SetIsAnimating(bool set) 
{
	BIZ_ASSERT(GetLifetimeState() != ActorSys::E_LifetimeState_Initialised);

	if(m_IsAnimating != set)
	{
		m_IsAnimating = set;

		// do movement count behaviour
		if(set)
			IncMovementCount();
		else
			DecMovementCount();
	}
}


// Increment the components movement count
// this will get called when the component starts moving 
// and when it receives a E_MovementMsg_IncMovementCount message
void C_MovementBase::IncMovementCount(void)
{
	m_MovementCount++;
	if(m_MovementCount == 1)	// 0 -> 1
	{
		// If the component has a parent then send an inc message to it
		if(GetActorList()->ActorExists(m_ParentHandle))
		{
			SendMessageToActor(m_ParentHandle,ActorMsg::E_MovementMsg_IncMovementCount);
		}
		else
		{
			AddSubscriber(GameMessaging::E_GameMsg_Update);
		}
	}
}

// Deccrement the components movement count
// this will get called when the component stops moving 
// and when it receives a E_MovementMsg_DecMovementCount message
void C_MovementBase::DecMovementCount(void)
{
	BIZ_ASSERT(m_MovementCount > 0);
	m_MovementCount--;

	if(m_MovementCount == 0) // 1 -> 0
	{
		// If the component has a parent then send an inc message to it
		if(GetActorList()->ActorExists(m_ParentHandle))
		{
			SendMessageToActor(m_ParentHandle,ActorMsg::E_MovementMsg_DecMovementCount);
		}
	}
}


// Param recalcTransform - whether we need to recalculate this mover's transform (e.g. a parent's transform has changed).
void C_MovementBase::Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, const E_MovementTransform transform)
{
	BIZ_ASSERT(m_AnimateTime <= m_AnimateDuration);
	BIZ_ASSERT(m_IsEnabled);

	const bool animatingThisUpdate = IsAnimating();
	const bool recalcTransform = (transform == E_MovementTransform_Update) || (transform == E_MovementTransform_Finalise);
	const bool recalcNeeded = (animatingThisUpdate || recalcTransform);
	if (recalcNeeded)
	{
		if (animatingThisUpdate)
			Animate(timeStep);	// May not be animating after this call.

		CalcWorldTransform(parentWorldTransform);	// Needs to be called so non-animating children inherit parent movements.

		AnimateActorTransform(m_WorldTransform, timeStep);
	}

	const bool finishedAnimatingThisUpdate = (animatingThisUpdate && !IsAnimating());
	if (finishedAnimatingThisUpdate)
	{
		//SetActorFinalTransform(m_WorldTransform);
		if (animatingThisUpdate)
		{
			SendMessageToActor(ActorMsg::E_MovementNotificationMsg_MovementComplete);	// audio component will want this, oh yes
			GetActorList()->SendMessageToActor(m_NotificationHandle, ActorMsg::E_MovementNotificationMsg_MovementComplete);
		}
	}

	// If we've stopped animating this frame and our parent is still
	// Or our parent has stopped moving and we're not animating
	const bool stoppedMovingThisUpdate = (finishedAnimatingThisUpdate && (transform != E_MovementTransform_Update))
		|| (!IsAnimating() && (transform == E_MovementTransform_Finalise));

	if(stoppedMovingThisUpdate)
	{
		SetActorFinalTransform(m_WorldTransform);
	}

	// Now update all children:
	E_MovementTransform newTransform = E_MovementTransform_None;

	if(stoppedMovingThisUpdate)	// we've stopped moving
		newTransform = E_MovementTransform_Finalise;
	else if((transform == E_MovementTransform_Update) || IsAnimating())	// we're still moving
		newTransform = E_MovementTransform_Update;

	//newTransform = recalcNeeded ? E_MovementTransform_Update : E_MovementTransform_None;

	ActorMsg::C_ActorMovementUpdateMsg msg(timeStep, WorldTransform(), newTransform);
	const int count = m_Children.Size();
	for (int c=0 ; c<count ; c++)
	{
		SendMessageToActor(m_Children[c], &msg);
	}
}


// Gets the parent's transform matrix from either its movement component if present, or its actor matrix.
bool C_MovementBase::GetActorWorldMatrix(Actor::C_ActorHandle handle, Maths::HMat43& matrix) const
{
	const Components::C_MovementBase* parentMover = ActorMovementComponent(handle);
	if (parentMover)
	{
		matrix = parentMover->WorldTransform().Matrix();
		return true;
	}
	else
	{
		const Actor::C_Actor* parentActor = GetActorList()->GetActorPtr(handle);
		if (parentActor)
		{
			matrix = parentActor->GetMatrix();
			return true;
		}
	}

	//LOGWARN(Actors::g_Log, DBSTR"empty parent handle");
	return false;
}


void C_MovementBase::CalcWorldTransform(const Maths::C_QuatTrans& parentWorldTransform)
{
	m_WorldTransform = m_LocalTransform * m_ParentOffset * parentWorldTransform;
}


// Use when animating an actor's transform.
void C_MovementBase::AnimateActorTransform(const Maths::C_QuatTrans& transform, const Maths::C_TimeS& timeStep)
{
	m_LinearVelocity = (transform.Translation() - GetActor()->GetPosition()) / timeStep.Get();

	const ActorMsg::C_ActorAnimateTransformMsg animateTransformMsg(transform, timeStep.Get());
	SendMessageToActor(&animateTransformMsg);
}


// Use when an animation ends, to finalise the actor's position.
void C_MovementBase::SetActorFinalTransform(const Maths::C_QuatTrans& transform)
{
	const ActorMsg::C_ActorSetTransformMsg setTransformMsg(transform);
	SendMessageToActor(&setTransformMsg);
	m_LinearVelocity = Maths::HVec3(0,0,0);
}


const Components::C_MovementBase* C_MovementBase::ActorMovementComponent(Actor::C_ActorHandle handle) const
{
	const Actor::C_Actor* actor = GetActorList()->GetActorPtr(handle);
	if (actor)
	{
		const I_ActorComponent* component = actor->QueryInterface(Components::E_IF_Movement);
		if (component)
		{
			const C_MovementBase* movementComponent = static_cast<const C_MovementBase*>(component);
			return movementComponent;
		}
	}
	return NULL;
}


void C_MovementBase::AttachToParent(Actor::C_ActorHandle parentHandle, const Maths::C_QuatTrans& parentWorldTransform)
{
	BIZ_ASSERT(!HasValidParent()); // Already got a valid parent!

	m_ParentOffset = m_WorldTransform * Inverse(parentWorldTransform);
	CalcWorldTransform(parentWorldTransform);

	const bool parentHasMoveComp = ActorMovementComponent(parentHandle)!=NULL;
	// commented out for now as vehicles use this message even though they have no movement component [PhilH]

	//if (parentHasMoveComp)
	{
		const ActorMsg::C_ActorMovementAddChildMsg addChildMsg(GetActorHandle());
		SendMessageToActor(parentHandle, &addChildMsg);
	}
	if(m_MovementCount > 0)
		RemoveSubscriber(GameMessaging::E_GameMsg_Update);

	m_ParentHandle = parentHandle;
}


void C_MovementBase::SetParent(Actor::C_ActorHandle parentHandle)
{
	ClearParent();

	const Components::C_MovementBase* parentMover = ActorMovementComponent(parentHandle);
	if (parentMover)
	{
		AttachToParent(parentHandle, parentMover->WorldTransform());
	}
	else
	{
		const Actor::C_Actor* newParent = GetActorList()->GetActorPtr(parentHandle);
		if (newParent)
		{
			AttachToParent(parentHandle, Maths::C_QuatTrans(newParent->GetMatrix()));
		}
		//else
		//{
		//	LOGWARN(Actors::g_Log, DBSTR"Unsuitable parent");
		//	EnableSubscriber(GameMessaging::E_GameMsg_Update, false);
		//}
	}
}


// Detaches the component from a hierarchy into world space.
void C_MovementBase::ClearParent()
{
	const ActorMsg::C_ActorMovementRemoveChildMsg removeChildMsg(GetActorHandle());
	SendMessageToActor(m_ParentHandle, &removeChildMsg);

	m_ParentOffset = m_WorldTransform;	// World is now parent.
	m_ParentHandle = Actor::C_ActorHandle();

	if(m_MovementCount > 0)
		AddSubscriber(GameMessaging::E_GameMsg_Update);
}


#ifndef CFG_GOLD
void C_MovementBase::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!g_MovementDebugRender)
		return;

	debugRenderPtr->Draw3dMatrix(WorldTransform().Matrix());
	Btl::String str(GetActor()->GetActorName());
	if (IsAnimating())
		str += " (Active)";
	if(m_Children.Size())
		str += Btl::String().Printf(" %d kids",m_Children.Size());

	str += Btl::String().Printf(" MC=%d",m_MovementCount);

	if(IsSubscribedTo(GameMessaging::E_GameMsg_Update))
	{
		str += Btl::String("\nUpdating");
	}
	debugRenderPtr->Draw3dText(WorldTransform().Translation(), str.Chars());

	// Draw link to parent:
	Maths::HMat43 parentMatrix;
	if (GetActorWorldMatrix(m_ParentHandle, parentMatrix))
	{
		const Maths::HVec3 dir = Maths::HVec3(parentMatrix.Translation() - WorldTransform().Translation());
		const F32 len = dir.Length();
		if (len > 0.01f)
		{
			debugRenderPtr->Draw3dArrow(WorldTransform().Translation(), dir/len, len, 0.1f, Maths::RGBA(255,255,0));
		}
	}
}
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////
// Attacher.
//
bool C_MovementAttacher::CreateImpl()
{
	BIZ_ASSERT(GetCreationData()->GetComponentTypeID() == Components::E_MovementAttacher);
	const bool ok = C_MovementBase::CreateImpl();
	m_DopplerSpeed = GetCreationData()->m_DopplerSpeed;
	return ok;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// Translator.
//
bool C_MovementTranslator::CreateImpl()
{
	BIZ_ASSERT(GetCreationData()->GetComponentTypeID() == Components::E_MovementTranslator);

	return C_MovementBase::CreateImpl();
}


bool C_MovementTranslator::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_MovementMsg_Translate:
		{
			const ActorMsg::C_ActorMovementTranslateMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementTranslateMsg>(pMessage);
			m_NotificationHandle = msg->Notifier();
			Translate(*msg);
			break;
		}

	default:
		return C_MovementBase::OnMessage(pMessage);
	}

	return true;
}

void C_MovementTranslator::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_BaseTranslation;	// Translation from matrix.
	stream << m_TargetTranslation;
}

void C_MovementTranslator::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_BaseTranslation;	// Translation from matrix.
	stream >> m_TargetTranslation;
}

void C_MovementTranslator::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementTranslator::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementTranslator::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementTranslator::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Deserialise(stream);
}

void C_MovementTranslator::Translate(const ActorMsg::C_ActorMovementTranslateMsg& translateMsg)
{
	if (translateMsg.m_MoveLocal)
	{
		m_BaseTranslation = m_LocalTransform.Translation();
		m_TargetTranslation = Maths::C_Vertex(translateMsg.Translation());
	}
	else
	{
		m_TargetTranslation = m_BaseTranslation + translateMsg.Translation();
	}

	m_AnimateTime = 0;
	m_AnimateDuration = translateMsg.MovementTime();
	m_MotionFlags = ActorMsg::TMotionTypeFlags(translateMsg.m_MovementFlags);
	SetIsAnimating();
}


void C_MovementTranslator::Animate(const Maths::C_TimeS& timeStep)
{
	BIZ_ASSERT(IsAnimating());

	m_AnimateTime = Min(m_AnimateDuration.m_Scalar, m_AnimateTime.m_Scalar + timeStep.m_Scalar);
	if (m_AnimateTime < m_AnimateDuration)
	{
		// Lerp to next position:
		const F32 frac = CalcInterpolationFraction(m_AnimateTime.Get(), m_AnimateDuration.Get(), m_MotionFlags);
		const Maths::HVec3 pos = m_BaseTranslation.Lerp(m_TargetTranslation, frac);
		m_LocalTransform.Translation(pos);
	}
	else
	{
		m_BaseTranslation = m_TargetTranslation;
		m_LocalTransform.Translation(m_TargetTranslation);
		SetIsAnimating(false);
	}
}

#ifndef CFG_GOLD
void C_MovementTranslator::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	if (!g_MovementDebugRender)
		return;

	C_MovementBase::DebugRender(debugRenderPtr);

	debugRenderPtr->Draw3dArrow(m_BaseTranslation, m_TargetTranslation - m_BaseTranslation,1.0f,1.0f,Maths::RGBA(255,255,255,255) );
}
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////
// Rotator.
//
bool C_MovementRotator::CreateImpl()
{
	BIZ_ASSERT(GetCreationData()->GetComponentTypeID() == Components::E_MovementRotator);

	return C_MovementBase::CreateImpl();
}


bool C_MovementRotator::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_MovementMsg_Rotate:
		{
			const ActorMsg::C_ActorMovementRotateMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementRotateMsg>(pMessage);
			m_NotificationHandle = msg->Notifier();
			RotateAxis(*msg);
			break;
		}

	default:
		return C_MovementBase::OnMessage(pMessage);
	}

	return true;
}

void C_MovementRotator::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_Axis;
	stream << m_BaseRotation;
	stream << m_Rotation;
	stream << m_TargetRotation;
}

void C_MovementRotator::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_Axis;
	stream >> m_BaseRotation;
	stream >> m_Rotation;
	stream >> m_TargetRotation;
}

void C_MovementRotator::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementRotator::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementRotator::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementRotator::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Deserialise(stream);
}

void C_MovementRotator::RotateAxis(const ActorMsg::C_ActorMovementRotateMsg& msg)
{
	m_BaseRotation = m_Rotation;

	if (msg.m_MoveLocal)
		m_TargetRotation = -Maths::DegToRad(msg.m_Degrees);	// Absolute target.
	else
		m_TargetRotation = m_Rotation - Maths::DegToRad(msg.m_Degrees); // Relative target.

	//printf("RotateAxis: base:%f, tgt:%f, rot:%f\n", Maths::RadToDeg((F32)m_BaseRotation), Maths::RadToDeg((F32)m_TargetRotation), Maths::RadToDeg((F32)m_Rotation));

	m_Axis = msg.Axis();

	m_AnimateTime = 0;
	m_AnimateDuration = msg.MovementTime();
	m_MotionFlags = ActorMsg::TMotionTypeFlags(msg.m_MovementFlags);

	SetIsAnimating();
}


void C_MovementRotator::Animate(const Maths::C_TimeS& timeStep)
{
	BIZ_ASSERT(IsAnimating());

	m_AnimateTime = Maths::Min(m_AnimateDuration.m_Scalar, m_AnimateTime.m_Scalar + timeStep.m_Scalar);
	if (m_AnimateTime < m_AnimateDuration)
	{
		// Lerp to next position:
		const Maths::HF32 frac = CalcInterpolationFraction(m_AnimateTime.Get(), m_AnimateDuration.Get(), m_MotionFlags);
		m_Rotation = Lerp(m_BaseRotation, m_TargetRotation, frac);
	}
	else
	{
		m_Rotation = m_TargetRotation;
		SetIsAnimating(false);
	}

	m_LocalTransform.Quaternion(Maths::C_Quaternion(m_Axis, m_Rotation));

	//printf("Animate: base:%f, tgt:%f, rot:%f\n", Maths::RadToDeg((F32)m_BaseRotation), Maths::RadToDeg((F32)m_TargetRotation), Maths::RadToDeg((F32)m_Rotation));
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// RotTrans.
//
bool C_MovementRotTrans::CreateImpl()
{
	BIZ_ASSERT(GetCreationData()->GetComponentTypeID() == Components::E_MovementRotTrans);

	return C_MovementBase::CreateImpl();
}


bool C_MovementRotTrans::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_MovementMsg_MoveAndAlignTo:
		{
			const ActorMsg::C_ActorMovementMoveAndAlignToMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementMoveAndAlignToMsg>(pMessage);
			MoveAndAlignToNode(msg->Transform(), msg->MovementTime(), ActorMsg::TMotionTypeFlags(msg->m_MovementFlags));
			break;
		}

	case ActorMsg::E_MovementMsg_MoveAndAlignToActor:
		{
			const ActorMsg::C_ActorMovementMoveAndAlignToActorMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementMoveAndAlignToActorMsg>(pMessage);
			//const C_GameActor *pActor = (const C_GameActor *)C_GameWorld::GetInstance()->GetGameActorPtr(msg->m_ActorHandle);
			const Actor::C_Actor *pActor = (const Actor::C_Actor *)GetActorList()->GetActorPtr(msg->m_ActorHandle);
			m_NotificationHandle = msg->Notifier();

			if (pActor != NULL)
			{
				Maths::C_QuatTrans quatTrans(pActor->GetMatrix());
				MoveAndAlignToNode(quatTrans, msg->MovementTime(), ActorMsg::TMotionTypeFlags(msg->m_MovementFlags));
			}
			else
			{
				LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), DBSTR "actor '%s' got E_MovementMsg_MoveAndAlignToActor message with no target actor", GetActor()->GetActorName());
			}

			break;
		}
	case ActorMsg::E_MovementMsg_Translate:
		{
			const ActorMsg::C_ActorMovementTranslateMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementTranslateMsg>(pMessage);
			m_NotificationHandle = msg->Notifier();
			Translate(msg->Translation(), msg->MovementTime(), ActorMsg::TMotionTypeFlags(msg->m_MovementFlags));
			break;
		}
	case ActorMsg::E_MovementMsg_Rotate:
		{
			const ActorMsg::C_ActorMovementRotateMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementRotateMsg>(pMessage);
			m_NotificationHandle = msg->Notifier();
			Rotate(msg->Axis(), Maths::DegToRad(msg->m_Degrees), msg->MovementTime(), ActorMsg::TMotionTypeFlags(msg->m_MovementFlags));
			break;
		}

	default:
		return C_MovementBase::OnMessage(pMessage);
	}

	return true;
}

void C_MovementRotTrans::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_TargetTranslation;
	stream << m_qRot; 
}

void C_MovementRotTrans::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_TargetTranslation;
	stream >> m_qRot; 
}

void C_MovementRotTrans::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementRotTrans::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementRotTrans::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementRotTrans::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Deserialise(stream);
}

void C_MovementRotTrans::MoveAndAlignToNode(const Maths::C_QuatTrans& quatTrans, const Maths::C_TimeS PCREF time, ActorMsg::TMotionTypeFlags flags)
{

	{
		//CalcWorldTransform();
		const Maths::HMat43 mat(m_WorldTransform.Matrix());
		// normalise the transform to stop any error creep
		m_WorldTransform.m_Quaternion.Normalise();
		// Reset the parent offset:
		m_ParentOffset = m_WorldTransform; 
		// Get trans relative to the object:
		m_TargetTranslation  = quatTrans.Translation() * mat.InverseRotTrans();
	}

	m_qRot = !(m_ParentOffset.Quaternion()) * quatTrans.Quaternion();	

	m_AnimateTime = 0;
	m_AnimateDuration = time;
	m_MotionFlags = flags;

	m_LocalTransform = Maths::C_QuatTrans();
	SetIsAnimating();
}

void C_MovementRotTrans::Translate(const Maths::HVec3 PCREF translation, const Maths::C_TimeS PCREF time, ActorMsg::TMotionTypeFlags flags)
{

	m_ParentOffset = m_WorldTransform; 
	m_TargetTranslation = translation;
	m_qRot = Maths::C_Quaternion();
	m_AnimateTime = 0;
	m_AnimateDuration = time;
	m_MotionFlags = flags;

	m_LocalTransform = Maths::C_QuatTrans();
	SetIsAnimating();
}

void C_MovementRotTrans::Rotate(const Maths::C_Normal PCREF axis, const F32 angle, const Maths::C_TimeS PCREF time, ActorMsg::TMotionTypeFlags flags)
{

	m_WorldTransform.m_Quaternion.Normalise();
	// Reset the parent offset:
	m_ParentOffset = m_WorldTransform; 
	m_TargetTranslation = Maths::HVec3(0.0f,0.0f,0.0f);

	const Maths::C_Quaternion q(axis, angle);

	//m_qRot = !(m_ParentOffset.Quaternion()) * quatTrans.Quaternion();	
	m_qRot = q;

	m_AnimateTime = 0;
	m_AnimateDuration = time;
	m_MotionFlags = flags;

	m_LocalTransform = Maths::C_QuatTrans();
	SetIsAnimating();
}



void C_MovementRotTrans::Animate(const Maths::C_TimeS& timeStep)
{
	BIZ_ASSERT(IsAnimating());

	m_AnimateTime = Maths::Min(m_AnimateDuration.m_Scalar, (m_AnimateTime.m_Scalar + timeStep.m_Scalar));
	if (m_AnimateTime < m_AnimateDuration)
	{
		// Lerp to next position:
		const F32 frac = CalcInterpolationFraction(m_AnimateTime.Get(), m_AnimateDuration.Get(), m_MotionFlags);

		// use ShortestSlerp to take the shorter 4d arc
		const Maths::C_Quaternion ident;		
		const Maths::C_Quaternion deltaRot( ident.ShortestSlerp( m_qRot, frac) );
		m_LocalTransform.Quaternion(deltaRot);

		const Maths::HVec3 centre;
		const Maths::HVec3 pos = centre.Lerp(m_TargetTranslation, frac);
		m_LocalTransform.Translation(pos);
	}
	else
	{
		m_LocalTransform.Quaternion(m_qRot);
		m_LocalTransform.Translation(m_TargetTranslation);
		SetIsAnimating(false);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Spinner.
//
bool C_MovementSpinner::CreateImpl()
{
	BIZ_ASSERT(GetCreationData()->GetComponentTypeID() == Components::E_MovementSpinner);

	const F32 initialRPM = GetCreationData()->m_InitialRPM;
	if (initialRPM != 0.0f)
	{
		SpinAxisY(initialRPM, 0.0f);
	}

	return C_MovementBase::CreateImpl();
}


bool C_MovementSpinner::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_MovementMsg_SpinAxisY:
		{
			const ActorMsg::C_ActorMovementSpinAxisYMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementSpinAxisYMsg>(pMessage);
			m_NotificationHandle = msg->Notifier();
			SpinAxisY(msg->m_RevsPerMinute, msg->SpinUpTime());
			break;
		}

	case ActorMsg::E_MovementMsg_SpinStop:
		{
			const ActorMsg::C_ActorMovementSpinStopMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementSpinStopMsg>(pMessage);
			m_NotificationHandle = msg->Notifier();
			Stop(*msg);
			break;
		}

	default:
		return C_MovementBase::OnMessage(pMessage);
	}

	return true;
}

void C_MovementSpinner::Serialise(Sys::C_ContextualStream& stream) const
{
	stream.WriteAs<U32>(m_SpinState);
	stream << m_Angle;
	stream << m_RequiredSpinRadiansPerSec;
	stream << m_CurrentRPM;
}

void C_MovementSpinner::Deserialise(Sys::C_ContextualStream& stream)
{
	stream.ReadAs<U32>(m_SpinState);
	stream >> m_Angle;
	stream >> m_RequiredSpinRadiansPerSec;
	stream >> m_CurrentRPM;

}

void C_MovementSpinner::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementSpinner::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementSpinner::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementSpinner::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Deserialise(stream);
}

void C_MovementSpinner::SpinAxisY(F32 revsPerMinute, const Maths::C_TimeS& spinupTime)
{
	if (IsAnimating())
		return;	//?

	m_RequiredSpinRadiansPerSec = -Maths::DegToRad( (revsPerMinute * 360.0f) / 60.0f );
	m_SpinState = ESpinningUp;
	m_AnimateTime = 0;
	m_AnimateDuration =spinupTime;

	SetIsAnimating();
}


void C_MovementSpinner::Stop(const ActorMsg::C_ActorMovementSpinStopMsg& msg)
{
	if (!IsAnimating())
		return;

	m_AnimateTime = 0;
	m_AnimateDuration = msg.Time();
	m_SpinState = ESpinningDown;
}


void C_MovementSpinner::Animate(const Maths::C_TimeS& timeStep)
{
	BIZ_ASSERT(IsAnimating());
	BIZ_ASSERT(m_SpinState != EStopped);

	F32 frac;
	m_AnimateTime = Maths::Min(m_AnimateDuration.m_Scalar, m_AnimateTime.m_Scalar+timeStep.m_Scalar);
	if (m_AnimateTime < m_AnimateDuration)
	{
		// Spinning up or down:
		frac = CalcInterpolationFraction(m_AnimateTime.Get(), m_AnimateDuration.Get(), ActorMsg::E_MovementFlag_EaseIn);
		if (m_SpinState == ESpinningDown)
			frac = 1.0f-frac;
	}
	else
	{
		// Spin at full speed:
		frac = 1.0f;

		if (m_SpinState == ESpinningDown)
		{
			m_SpinState = EStopped;
			m_CurrentRPM = 0;
			SetIsAnimating(false);
			return;
		}
	}

	m_CurrentRPM = fabsf(frac * m_RequiredSpinRadiansPerSec * 60.0f)/Maths::Pi2;
	m_Angle += frac * m_RequiredSpinRadiansPerSec * timeStep.Get();
	m_Angle = WrapAngle( m_Angle );
	const Maths::HVec3 axis( Maths::Constants<Maths::HVec3>::UnitAxisPositiveY() );
	const Maths::HF32 angle( Maths::Load<Maths::HF32>( &m_Angle ) );
	const Maths::C_Quaternion q( axis, angle );
	m_LocalTransform.Quaternion(q);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
// Wheel.
//
bool C_MovementWheel::CreateImpl()
{
	BIZ_ASSERT(GetCreationData()->GetComponentTypeID() == Components::E_MovementWheel);

	m_Radius = GetCreationData()->m_Radius;
	//m_Angle = wheelCreate->m_WorldMatrix.GetSRT().Rotate().Pitch().m_AngleR;

	return C_MovementBase::CreateImpl();
}


// Notes:
// Assumes the actor's X axis is the 'axle' of the wheel.
// Assumes parent's Z axis as the 'direction of movement'.
// Assumes wheel X axis is perpendicular to parent's Z axis.
//void C_MovementWheel::Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, bool recalcTransform)
void C_MovementWheel::Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, const E_MovementTransform transform)
{
	const bool recalcTransform = (transform == E_MovementTransform_Update) || (transform == E_MovementTransform_Finalise);

	using namespace Maths;

	if (recalcTransform)
	{
		const HVec3 lastPos = WorldTransform().Translation();
		C_QuatTrans nextTransform = m_ParentOffset * parentWorldTransform;
		const HVec3 posDelta(nextTransform.Translation() - lastPos);
		const HVec3 directionOfMovement(parentWorldTransform.Matrix().AZ());
		const F32 dotScale = posDelta.Normalised().Normal() | directionOfMovement.Normalised().Normal();
		const F32 dist = posDelta.Length() * dotScale;
		const F32 circum = Pi * (m_Radius + m_Radius);
		m_Angle = -((dist / circum) * Pi2);
	}

	C_MovementBase::Update(timeStep, parentWorldTransform, transform);

	SetIsAnimating(recalcTransform);	// Do we want this?
}


void C_MovementWheel::Animate(const Maths::C_TimeS& timeStep)
{
	const Maths::HVec3 axis(Maths::Constants<Maths::HVec3>::UnitAxisPositiveX());
	const Maths::HF32 angle(Maths::Load<Maths::HF32>(&m_Angle));
	const Maths::C_Quaternion dq(axis, angle);
	const Maths::C_Quaternion newQuat = (m_LocalTransform.Quaternion() * dq).Normalised();
	m_LocalTransform.Quaternion(newQuat);
}

void C_MovementWheel::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_Radius;	// needed?
	stream << m_Angle;
}

void C_MovementWheel::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_Radius;	// needed?
	stream >> m_Angle;

}

void C_MovementWheel::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementWheel::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementWheel::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementWheel::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Serialise(stream);
}



//////////////////////////////////////////////////////////////////////////
// Movement constrainer
//
bool C_MovementConstrainer::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch (pMessage->Type())
	{
		case ActorMsg::E_MovementMsg_Constrain:
		{
			const ActorMsg::C_ActorMovementConstrainMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementConstrainMsg>(pMessage);

			if (msg->m_ConstrainXTrans)
			{
				m_ConstrainFlags |= ETransX;
				m_TransConstrain.X(WorldTransform().Translation().X());
			}
			else
			{
				m_ConstrainFlags &= (~ETransX);
				m_TransConstrain.X(0.0f);
			}

			if (msg->m_ConstrainYTrans)
			{
				m_ConstrainFlags |= ETransY;
				m_TransConstrain.Y(WorldTransform().Translation().Y());
			}
			else
			{
				m_ConstrainFlags &= (~ETransY);
				m_TransConstrain.Y(0.0f);
			}

			if (msg->m_ConstrainZTrans)
			{
				m_ConstrainFlags |= ETransZ;
				m_TransConstrain.Z(WorldTransform().Translation().Z());
			}
			else
			{
				m_ConstrainFlags &= (~ETransZ);
				m_TransConstrain.Z(0.0f);
			}

			if (msg->m_ConstrainHeading)
				m_ConstrainFlags |= EHdg;
			else
				m_ConstrainFlags &= (~EHdg);

			if (msg->m_ConstrainPitch)
				m_ConstrainFlags |= EPitch;
			else
				m_ConstrainFlags &= (~EPitch);

			if (msg->m_ConstrainRoll)
				m_ConstrainFlags |= ERoll;
			else
				m_ConstrainFlags &= (~ERoll);

		}	break;

		default:
			return C_MovementBase::OnMessage(pMessage);
	}

	return true;
}


static Maths::C_Quaternion CalcRotDiffQuat(const Maths::C_Quaternion& src, const Maths::C_Quaternion& dest, const Maths::HVec3& constrainAxis)
{
	const Maths::C_Quaternion q = !src * Nearest(src, dest);
	const Maths::HVec3 i = q.Imag()*2;
	const F32 d = DotProduct(i, constrainAxis);
	return Maths::C_Quaternion(constrainAxis, d);	// = quat difference to constrained axis.
}


void C_MovementConstrainer::CalcWorldTransform(const Maths::C_QuatTrans& parentWorldTransform)
{
	using namespace Maths;

	const C_QuatTrans prevTransform = m_WorldTransform;

	C_MovementBase::CalcWorldTransform(parentWorldTransform);

	if (m_ConstrainFlags == 0)
		return;

	if (m_ConstrainFlags & EHdg)
	{
		const C_Quaternion quatDiff = CalcRotDiffQuat(WorldTransform().Quaternion(), prevTransform.Quaternion(), prevTransform.Matrix().AY());
		m_LocalTransform.Quaternion(m_LocalTransform.Quaternion() * quatDiff);	// Takes out difference quat.
	}

	if (m_ConstrainFlags & EPitch)
	{
		const C_Quaternion quatDiff = CalcRotDiffQuat(WorldTransform().Quaternion(), prevTransform.Quaternion(), prevTransform.Matrix().AX());
		m_LocalTransform.Quaternion(m_LocalTransform.Quaternion() * quatDiff);	// Takes out difference quat.
	}

	if (m_ConstrainFlags & ERoll)
	{
		const C_Quaternion quatDiff = CalcRotDiffQuat(WorldTransform().Quaternion(), prevTransform.Quaternion(), prevTransform.Matrix().AZ());
		m_LocalTransform.Quaternion(m_LocalTransform.Quaternion() * quatDiff);	// Takes out difference quat.
	}

	C_MovementBase::CalcWorldTransform(parentWorldTransform);

	if (m_ConstrainFlags & ETransX)
		m_WorldTransform.m_Translation.X(m_TransConstrain.X());

	if (m_ConstrainFlags & ETransY)
		m_WorldTransform.m_Translation.Y(m_TransConstrain.Y());

	if (m_ConstrainFlags & ETransZ)
		m_WorldTransform.m_Translation.Z(m_TransConstrain.Z());
}


void C_MovementConstrainer::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_ConstrainFlags;
	stream << m_TransConstrain;
}


void C_MovementConstrainer::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_ConstrainFlags;
	stream >> m_TransConstrain;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Animation Slave Movement
//
C_MovementAnimationSlave::C_MovementAnimationSlave():m_AnimationObjectID(-1)
{

}

bool C_MovementAnimationSlave::CreateImpl()
{
	BIZ_ASSERT(GetCreationData()->GetComponentTypeID() == Components::E_MovementAnimationSlave);
	const bool ok = C_MovementBase::CreateImpl();
	//EnableSubscriber(GameMessaging::E_GameMsg_Update, false);
	return ok;
}

ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(C_MovementAnimationSlave)
	ACTOR_COMPONENT_MESSAGE_TYPES_SINGLE(ActorMsg::E_AnimationMsg_SetAnimationController)
ACTOR_COMPONENT_MESSAGE_TYPES_END(C_MovementAnimationSlave, C_MovementBase)

bool  C_MovementAnimationSlave::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_AnimationMsg_SetAnimationController:
		{
			SetIsAnimating(true);	// maybe we should send a start/stop animating message?
			m_AnimationObjectID = Messaging::MessageCast<ActorMsg::C_AnimationSetControllerMsg>(pMessage)->GetValue();
			m_AnimationController = Messaging::MessageCast<ActorMsg::C_AnimationSetControllerMsg>(pMessage)->GetActorHandle();
		}
		break;
	default:
		return C_MovementBase::OnMessage(pMessage);
	}
	return true;
}

void C_MovementAnimationSlave::CalcWorldTransform(const Maths::C_QuatTrans& parentWorldTransform)
{
	const Maths::C_QuatTrans prevTransform = m_WorldTransform;
	Maths::C_Vertex newTrans;
	Maths::C_Quaternion newRot;

	if(CalculateNewOrientation(newTrans,newRot))
	{
// 		const Actor::C_Actor *pParentActor = GetActorList()->GetActorPtr(m_ParentHandle);
// 
// 		if(pParentActor)
// 			m_WorldTransform = Maths::C_QuatTrans(newRot,newTrans) * pParentActor->GetMatrix();
// 		else
			m_WorldTransform = Maths::C_QuatTrans(newRot,newTrans) * parentWorldTransform;
	}
}

bool C_MovementAnimationSlave::CalculateNewOrientation(Maths::C_Vertex &newTrans,Maths::C_Quaternion &newRot)
{
	if(m_AnimationObjectID == -1)	// do not update if we don't belong to an animation object
		return false;

	// TODO: Query animation controller for current position & orientation for our curveset
	const Actor::C_Actor *pActor = GetActorList()->GetActorPtr(m_AnimationController);
	if(pActor)
	{
		const I_AnimationController *pAnimationControllerIF = (const I_AnimationController *)pActor->QueryInterface(E_IF_AnimationController);

		if(pAnimationControllerIF)
		{
			bool success = true;

			if(HasValidParent())
			{
				success = pAnimationControllerIF->GetObjectLocalTranslation(m_AnimationObjectID,newTrans);
				success |= pAnimationControllerIF->GetObjectLocalRotation(m_AnimationObjectID,newRot);
			}
			else
			{
				success = pAnimationControllerIF->GetObjectTranslation(m_AnimationObjectID,newTrans);
				success |= pAnimationControllerIF->GetObjectRotation(m_AnimationObjectID,newRot);
			}

			if(success)
				return true;
		}
	}

	return false;
}

// Serialisation functions
// -----------------------
void C_MovementAnimationSlave::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_AnimationController;	// needed?
	stream << m_AnimationObjectID;
}

void C_MovementAnimationSlave::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_AnimationController;	// needed?
	stream >> m_AnimationObjectID;
}

void C_MovementAnimationSlave::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementAnimationSlave::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementAnimationSlave::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementAnimationSlave::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Serialise(stream);
}

//
// C_MovementRelativeToBone
//

C_MovementRelativeToBone::C_MovementRelativeToBone():
	m_JointIndex(-1),
	m_AttachedToBone(false),
	m_IgnoreBoneRotation(false)
{
}

bool C_MovementRelativeToBone::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	//switch(pMessage->Type())
	//{
	//	default:
			return C_MovementBase::OnMessage(pMessage);
	//}
	//return true;
}

void C_MovementRelativeToBone::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}


void C_MovementRelativeToBone::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementRelativeToBone::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}


void C_MovementRelativeToBone::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Deserialise(stream);
}

void C_MovementRelativeToBone::CalcWorldTransform(const Maths::C_QuatTrans& parentWorldTransform)
{
	PERF_EVENT("SharedActors.MovementRelativeToBone.CalcWorldTransform");
	
	const Actor::C_Actor* pParent = GetActorList()->GetActorPtr(m_ParentHandle);
	
	if (pParent != NULL)
	{
		if (!m_AttachedToBone)
		{
			AttachToBone(pParent);
		}
		
		Animation::C_JointTransform jointModelTransform(Animation::C_JointTransform::IDENTITY);
		
		if (m_JointIndex >= 0)
		{
			const Components::I_AnimationStateComponent* pAnimState = NULL;
			if (pParent->QueryInterface(&pAnimState))
			{
				const Animation::C_Pose& pose = pAnimState->GetPose();
				jointModelTransform = pose.GetModelJointTransform(m_JointIndex, pAnimState->GetSkeleton());
			}

		#ifndef CFG_GOLD
			// Only need to do this every update if relying on live Sorcery updates.
			CalcOffsetTransform();
		#endif

			m_ParentOffset = Maths::C_QuatTrans(Maths::C_Quaternion(jointModelTransform.GetRotation()), Maths::HVec3(jointModelTransform.GetTranslation()));
			
			if (m_IgnoreBoneRotation)
			{
				m_ParentOffset.Quaternion(m_OriginalRotation);
			}
		}
	}
	
	m_WorldTransform = m_LocalTransform * m_ParentOffset * parentWorldTransform;
}

void C_MovementRelativeToBone::Animate(const Maths::C_TimeS& timeStep)
{
	// do nothing, as animation is sourced from elsewhere.
}

void C_MovementRelativeToBone::CalcOffsetTransform()
{
	Maths::HVec3 translation = Load(GetCreationData()->m_TranslationOffset);
	Maths::Vector3 eulerRotationAngles = Load(GetCreationData()->m_RotationOffset);
	Maths::Quaternion rotation = Maths::MakeUnitQuaternion(Maths::MakeMat33(Maths::MakeEulerXYZ(eulerRotationAngles * Maths::MakeVF32(Maths::Pi / 180.f))));
	
	m_LocalTransform = Maths::C_QuatTrans(Maths::C_Quaternion(rotation), translation);
}

void C_MovementRelativeToBone::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_OriginalRotation;
}

void C_MovementRelativeToBone::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_OriginalRotation;
}

bool C_MovementRelativeToBone::InitialiseImpl()
{
	bool ok = C_MovementBase::InitialiseImpl();

	m_OriginalRotation = m_WorldTransform.Quaternion();
		
	return ok;
}

bool C_MovementRelativeToBone::CreateImpl()
{
	BIZ_ASSERT(GetCreationData()->GetComponentTypeID() == Components::E_MovementRelativeToBone);
	const bool ok = C_MovementBase::CreateImpl();
	
	m_IgnoreBoneRotation = GetCreationData()->m_IgnoreBoneRotation;
	m_JointIndex = -1;
	m_AttachedToBone = false;
	
	CalcOffsetTransform();
	
	//EnableSubscriber(GameMessaging::E_GameMsg_Update, false);
	return ok;
}

void C_MovementRelativeToBone::ExternalDependenciesCreatedImpl()
{
	C_MovementBase::ExternalDependenciesCreatedImpl();

	const Actor::C_Actor* pParent = GetActorList()->GetActorPtr(m_ParentHandle);
	
	if (pParent)
	{
		AttachToBone(pParent);
	}
}

void C_MovementRelativeToBone::AttachToBone(const Actor::C_Actor* pParent)
{
	if (pParent)
	{
		const Components::I_AnimationStateComponent* pAnimState = NULL;
		if (pParent->QueryInterface(&pAnimState))
		{
			if ((pAnimState->GetLifetimeState() != ActorSys::E_LifetimeState_Killed) && pAnimState->GetPose().IsValid())
			{
				BIZ_ASSERT(pAnimState->GetLifetimeState() != ActorSys::E_LifetimeState_Failed);
				m_JointIndex = pAnimState->GetPose().FindJointTransformByName(GetCreationData()->m_BoneName.Text());
				m_AttachedToBone = true;
			}
		}
		
		if (!m_AttachedToBone)
		{
			m_JointIndex = -1;
			m_AttachedToBone = false;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Aimer.
//
bool C_MovementAimer::InitialiseImpl()
{
	BIZ_ASSERT(GetCreationData()->GetComponentTypeID() == Components::E_MovementAimer);

	m_IsAligned = false;
	m_LocalOffset = false;

	if (GetCreationData()->m_pTargetActor)
		m_TargetHandle = GetCreationData()->m_pTargetActor->GetActorHandle();

	if (m_TargetHandle)
	{
		m_TrackingSpeed = 1.f;
		m_UseRetrackingSpeed = false;
		//SetIsAnimating(true);
	}
	else
	{
		m_TrackingSpeed = Maths::Clamped(GetCreationData()->m_TrackingSpeed, 0.0f, 1.0f);
	}

	m_LimitAngle = GetCreationData()->m_LimitAngle;
	m_LimitAngleAmount = Maths::DegToRad(GetCreationData()->m_LimitAngleAmount);

	return C_MovementBase::InitialiseImpl();
}

bool C_MovementAimer::CreateImpl()
{
	if(!C_MovementBase::CreateImpl())
		return false;

	// moved from init
	if (m_TargetHandle)
		SetIsAnimating(true);

	return true;
}

bool C_MovementAimer::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	PERF_EVENT("C_MovementAimer.OnMessage");
	switch (pMessage->Type())
	{
		case ActorMsg::E_MovementMsg_AimAt:
		{
			const ActorMsg::C_ActorMovementAimAtMsg* msg = Messaging::MessageCast<ActorMsg::C_ActorMovementAimAtMsg>(pMessage);
			m_AimPos = msg->GetPosition();
			m_TrackingSpeed = 1.0f;
			m_LocalOffset = false;
			m_UseRetrackingSpeed = false;
			SetIsAnimating();
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
			SetIsAnimating();
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
				SetIsAnimating();
			}
			break;
		// does nothing but stop an error
		case ActorMsg::E_MovementNotificationMsg_AimerAligned:
		case ActorMsg::E_MovementNotificationMsg_AimerLostAlignment:
			break;

		default:
			return C_MovementBase::OnMessage(pMessage);
	}

	return true;
}

void C_MovementAimer::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_TargetHandle;
	stream << m_TrackingSpeed;
	stream << m_TargetQuatLs;
	stream << m_OffsetPos;
	stream << m_LocalOffset;
	stream << m_IsAligned;
	stream << m_FutureTime;
	stream << m_UseRetrackingSpeed;
	stream << m_RetrackingSpeed;
	stream << m_LimitAngle;
	stream << m_LimitAngleAmount;
	stream << m_InitialLock;
	stream << m_AimPos;
}

void C_MovementAimer::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_TargetHandle;
	stream >> m_TrackingSpeed;
	stream >> m_TargetQuatLs;	
	stream >> m_OffsetPos;
	stream >> m_LocalOffset;
	stream >> m_IsAligned;
	stream >> m_FutureTime;
	stream >> m_UseRetrackingSpeed;
	stream >> m_RetrackingSpeed;
	stream >> m_LimitAngle;
	stream >> m_LimitAngleAmount;
	stream >> m_InitialLock;
	stream >> m_AimPos;
}

void C_MovementAimer::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementAimer::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementAimer::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementAimer::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Deserialise(stream);
}


void C_MovementAimer::Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, const E_MovementTransform transform)
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
// 	else //[MarkC] removed code duplication
// 	{
// 		const Maths::AxisZ az = (m_AimPos - WorldTransform().Translation()).Normalised().Normal();
// 		const Maths::AxisY ay = Maths::C_Normal(0.0f,1.0f,0.0f);	// Can we have a constant for this please?
// 		const Maths::HMat33 aimRot(az, ay);
// 		Maths::C_Quaternion targetQuatWs(aimRot);
// 		// Calc target quat in local space:
// 		m_TargetQuatLs = !(m_ParentOffset.Quaternion() * parentWorldTransform.Quaternion()) * targetQuatWs;
// 	}

	const Maths::AxisZ az = (targetPos - WorldTransform().Translation()).Normalised().Normal();
	const Maths::AxisY ay = Maths::C_Normal(0.0f,1.0f,0.0f);	//[MarkC] Can we have a constant for this please?
	const Maths::HMat33 aimRot(az, ay);
	Maths::C_Quaternion targetQuatWs(aimRot);
	// Calc target quat in local space:
	m_TargetQuatLs = !(parentWorldTransform.Quaternion() * m_ParentOffset.Quaternion()) * targetQuatWs;

	//
	if (m_LimitAngle)
	{
		Maths::VF32 vrotAngle = Maths::RotationAngle(m_TargetQuatLs);
		F32 rotAngle (Maths::GetF32(vrotAngle));
		if (rotAngle > Maths::Pi)
			rotAngle = Maths::Pi2 - rotAngle;
		

		if (rotAngle > m_LimitAngleAmount)
		{
			Maths::C_Quaternion limited; 
			F32 amount = m_LimitAngleAmount / rotAngle;
			m_TargetQuatLs = limited.ShortestSlerp(m_TargetQuatLs, amount);
		}
	}
	

	C_MovementBase::Update(timeStep, parentWorldTransform, transform);
}


void C_MovementAimer::Animate(const Maths::C_TimeS& timeStep)
{
	const Actor::C_Actor* target = GetActorList()->GetActorPtr(m_TargetHandle);
	if (target)
	{
		//if (!m_IsAligned)	// Test stops wobble. but it puts a frame lag against moving objects
		//{
			F32 trackSpeed = m_TrackingSpeed;

			if (m_InitialLock==false&& m_UseRetrackingSpeed)
			{
				trackSpeed = m_RetrackingSpeed;
			}
			m_LocalTransform.Quaternion(m_LocalTransform.Quaternion().ShortestSlerp(m_TargetQuatLs, trackSpeed));
			m_LocalTransform.m_Quaternion.Normalise();
		//}

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
		//m_LocalTransform.Quaternion(m_LocalTransform.Quaternion().ShortestSlerp(m_TargetQuatLs, 1.0f));
		m_LocalTransform.Quaternion(m_TargetQuatLs);
		m_LocalTransform.m_Quaternion.Normalise();
		//SetIsAnimating(false);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Bobbing Boat
//
bool C_MovementBobbingBoat::CreateImpl()
{
	BIZ_ASSERT(GetCreationData()->GetComponentTypeID() == Components::E_MovementBobbingBoat);
	m_PitchWavelength = GetCreationData()->m_PitchTimerScale;
	m_RollWavelength = GetCreationData()->m_RollTimerScale;
	m_PitchTimer = GetCreationData()->m_PitchTimerOffset;
	m_RollTimer = GetCreationData()->m_RollTimerOffset;
	m_PitchWaveAmplitude = Maths::DegToRad(GetCreationData()->m_MaxPitchScale);
	m_RollWaveAmplitude = Maths::DegToRad(GetCreationData()->m_MaxRollScale);

	SetIsAnimating();
	return C_MovementBase::CreateImpl();
}



void C_MovementBobbingBoat::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_PitchTimer;
	stream << m_RollTimer;
	stream << m_PitchWavelength;
	stream << m_RollWavelength;
	stream << m_PitchWaveAmplitude;
	stream << m_RollWaveAmplitude;
}

void C_MovementBobbingBoat::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_PitchTimer;
	stream >> m_RollTimer;
	stream >> m_PitchWavelength;
	stream >> m_RollWavelength;
	stream >> m_PitchWaveAmplitude;
	stream >> m_RollWaveAmplitude;
}

void C_MovementBobbingBoat::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MovementBobbingBoat::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MovementBobbingBoat::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	C_MovementBase::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MovementBobbingBoat::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	C_MovementBase::DeserialiseForSync(stream);
	Deserialise(stream);
}



void C_MovementBobbingBoat::Animate(const Maths::C_TimeS& timeStep)
{
	BIZ_ASSERT(IsAnimating());
	
	m_PitchTimer += timeStep.Get() * m_PitchWavelength;
	m_RollTimer += timeStep.Get() * m_RollWavelength;
	F32 pitch = m_PitchWaveAmplitude * Maths::Sin(m_PitchTimer);
	F32 roll = m_RollWaveAmplitude * Maths::Cos(m_RollTimer);

	const Maths::HVec3 pitchAxis( Maths::Constants<Maths::HVec3>::UnitAxisPositiveX() );
	const Maths::HF32 pitchAngle( Maths::Load<Maths::HF32>( &pitch ) );
	const Maths::C_Quaternion Pitch( pitchAxis, pitchAngle );

	const Maths::HVec3 rollAxis( Maths::Constants<Maths::HVec3>::UnitAxisPositiveZ() );
	const Maths::HF32 rollAngle( Maths::Load<Maths::HF32>( &roll ) );
	const Maths::C_Quaternion Roll( rollAxis, rollAngle );
	m_LocalTransform.Quaternion(Pitch * Roll);
}

} // namespace Components

