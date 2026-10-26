#ifndef SHAREDACTORMESSAGES_H
#define SHAREDACTORMESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorMessageHandle.h>
#include <misc_shared/messaging/MessageValue.h>
#include <misc_shared/uid/FourCC.h>
#include <shared_actors/SharedActorMessages.h>


namespace ActorMsg
{

#ifndef CFG_GOLD
void RegisterSharedMessageDebugInfo();
#endif


enum E_SharedActorMessages
{
	E_GameActorMsg_HiBits = FOURCC_HI20('MSAM'),	// Mercury Shared Actor Messages

	E_GameActorMsg_Enable,			///< Enable the functionality of the actor
	E_GameActorMsg_Disable,			///< Disable the functionality of the actor
	E_GameActorMsg_Reset,			///< Reset the actor to it's original state

	E_GameActorMsg_SetUsable,
	E_GameActorMsg_SetNotUsable,
	E_GameActorMsg_RequestUse,	
	E_GameActorMsg_Use,			
	E_GameActorMsg_Activate,	
	E_GameActorMsg_Deactivate,
	E_GameActorMsg_Open,
	E_GameActorMsg_Close,
	E_GameActorMsg_Increment,
	E_GameActorMsg_Decrement,
	E_GameActorMsg_SetIntegerValue,
	E_GameActorMsg_FlipFlop,

	E_GameActorMsg_ActionBegin,				///< @see C_ActionBeginMsg.
	E_GameActorMsg_ActionCompleted,			///< @see C_ActionCompletedMsg.
	E_GameActorMsg_ActionAborted,			///< @see C_ActionAbortedMsg.
	E_GameActorMsg_ControlStarted,			///< Actor has started controlling recipient
	E_GameActorMsg_ControlEnded,			///< Actor has finished controlling recipient

	E_GameActorMsg_Update2ndPass,			// Update the actor during the 2nd pass			
	E_GameActorMsg_SetTransform,
	E_GameActorMsg_AnimateTransform,

	E_GameActorMsg_PhysicsConstraintDestroyed,

	E_GameActorMsg_SetProxyActor,

	// update management messages
	E_GameActorMsg_UpdateSeedPosition,		// seed sends its position
	E_GameActorMsg_SeedRequestPosition,
	E_GameActorMsg_CountedActorPosition,	// counted actor sends position to seed
	E_GameActorMsg_CountedActorOrder,		// counted actor gets its update order

	E_GameActorMsg_DisableSlowMotion,		// Disable Slow Motion effects on update time

	E_GameActorMsg_End, 
	E_GameActorMsg_Begin = E_GameActorMsg_HiBits + 1, 
	E_GameActorMsg_Count = E_GameActorMsg_End - E_GameActorMsg_Begin
};

// Message Structures
// In another file?

// Util functions

// Stores QT into the given storage.
inline void StoreQuatTrans(const Maths::C_QuatTrans& qt, MathsStorage::Float4& quat, MathsStorage::Float3& trans)
{
	Maths::Store((Maths::HVec4)qt.m_Quaternion, &quat);
	Maths::Store(qt.m_Translation, &trans);
}

// Extracts QT from the given storage.
inline Maths::C_QuatTrans LoadQuatTrans(const MathsStorage::Float4& quat, const MathsStorage::Float3& trans)
{
	Maths::C_QuatTrans qt;
	qt.m_Quaternion = Maths::C_Quaternion(Maths::Load<Maths::HVec4>(&quat));
	qt.m_Translation = Maths::Load<Maths::HVec3>(&trans);
	return qt;
}

/**
@brief Message class for E_GameActorMsg_SetIntegerValue.
Generic message sent to an actor passing an integer value that is used to set its primary property
Use GetValue() for integer value.
*/
typedef Messaging::C_MessageValue<S32, E_GameActorMsg_SetIntegerValue> C_IntegerValueMsg;

/**
@brief Message class for E_GameActorMsg_ActionBegin.
Generic message sent to an actor asking it to perform an action and notify 
the actor specified with E_GameActorMsg_ActionCompleted when the action is 
complete.
Use GetValue() for actor to receive the completion message.
@see C_ActionCompletedMsg.
*/
typedef Actor::C_MessageActorHandle<E_GameActorMsg_ActionBegin> C_ActionBeginMsg;


/**
@brief Message class for E_GameActorMsg_ActionCompleted.
Generic message sent when an actor has completed an action. This is normally 
in response to E_GameActorMsg_ActionBegin but could be triggered by other
causes.
Use GetValue() for the actor that completed the action.
@see C_MessageValue.
*/
typedef Actor::C_MessageActorHandle<E_GameActorMsg_ActionCompleted> C_ActionCompletedMsg;

/**
@brief Message class for E_GameActorMsg_ActionAborted.
Generic message sent when an actor has aborted an action. This is normally 
in response to E_GameActorMsg_ActionBegin but could be triggered by other
causes.
Use GetValue() for the actor that aborted the action.
TODO:	do we want to return an abort code?
@see C_MessageValue.
*/
typedef Actor::C_MessageActorHandle<E_GameActorMsg_ActionAborted> C_ActionAbortedMsg;

/**
@brief Message class for E_GameActorMsg_ControlStarted.
Generic message sent to an actor that tells it that it is being controlled by a particular actor
Use GetValue() for the actor that is controlling.
@see C_ControlEndedMsg.
*/
typedef Actor::C_MessageActorHandle<E_GameActorMsg_ControlStarted> C_ControlStartedMsg;


/**
@brief Message class for E_GameActorMsg_ControlEnded.
Generic message sent when an actor has finished controlling the recipient actor
Use GetValue() for the actor that was controlling.
@see C_MessageValue.
*/
typedef Actor::C_MessageActorHandle<E_GameActorMsg_ControlEnded> C_ControlEndedMsg;

// 2nd Pass Update Message
typedef Messaging::C_MessageValue<F32, E_GameActorMsg_Update2ndPass> C_Update2ndPassMsg;

// Set the actor that the proxy actor is linked to
typedef Actor::C_MessageActorHandle<E_GameActorMsg_SetProxyActor> C_SetProxyActorMsg;

// Set absolute transform.
class C_ActorSetTransformMsg : public Messaging::C_MessageDerived<C_ActorSetTransformMsg, E_GameActorMsg_SetTransform>
{
public:
	explicit C_ActorSetTransformMsg(const Maths::C_QuatTrans& transform)
	{
		StoreQuatTrans(transform, m_Quat, m_Trans);
	}

	Maths::C_QuatTrans Transform() const {return LoadQuatTrans(m_Quat, m_Trans);}
private:
	MathsStorage::Float4 m_Quat;
	MathsStorage::Float3 m_Trans;
};

// This version implies animation between transforms (if special processing is required).
class C_ActorAnimateTransformMsg : public Messaging::C_MessageDerived<C_ActorAnimateTransformMsg, E_GameActorMsg_AnimateTransform>
{
public:
	explicit C_ActorAnimateTransformMsg(const Maths::C_QuatTrans& transform, F32 timeStep)
	{
		m_TimeStep = timeStep;
		StoreQuatTrans(transform, m_Quat, m_Trans);
	}

	Maths::C_QuatTrans Transform() const {return LoadQuatTrans(m_Quat, m_Trans);}

	F32 m_TimeStep;

private:
	MathsStorage::Float4 m_Quat;
	MathsStorage::Float3 m_Trans;
};

typedef Actor::C_MessageActorHandle<ActorMsg::E_GameActorMsg_RequestUse> C_RequestUseMsg;

typedef Messaging::C_MessageValue<bool, E_GameActorMsg_DisableSlowMotion> C_DisableSlowMotionMsg;


}//namespace ActorMsg


#endif
